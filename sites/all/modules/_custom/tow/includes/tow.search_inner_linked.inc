<?php

/**
 * @file
 *
 * Linking tables and records.
 */
/* * **************************************************************************************************
 * Hook Handlers
 * ************************************************************************************************** */

/**
 * Part of hook_cron() implementation. Defines actions for table and records linking while running cron.
 */
function tow_search_inner_linked_cron() {
    $time = time();

    // Linking data.
    $query = db_query("

    SELECT
      *

      FROM
        {tow_link_tables} t

  ");
    $records = array();

    // Apply actions to the linked data according to options specified.
    while ($result = db_fetch_object($query)) {
        $old[] = $result;
        $tn = $result->table_nid;
        if (!isset($records[$tn])) {
            $records[$tn] = $result;
        }

        // Options.
        else {
            switch ($result->op) {

                // Delete.
                case 'delete':
                    $records[$tn] = ($records[$tn]->record_nid < $result->record_nid) ? $records[$tn] : $result;
                    break;

                // Update.
                case 'update':
                    if ($records[$tn]->op != 'delete') {
                        $records[$tn] = ($records[$tn]->record_nid < $result->record_nid) ? $records[$tn] : $result;
                    }
                    break;

                // Insert.
                case 'insert':
                    if (($records[$tn]->op != 'delete') && ($records[$tn]->op != 'update')) {
                        $records[$tn] = ($records[$tn]->record_nid < $result->record_nid) ? $records[$tn] : $result;
                    }
                    break;
            }
        }
    }

    // Delete unnecessary items.
    db_query("

    DELETE
      FROM
        {tow_link_tables} t

      WHERE
        t.timestamp < %d

  ", $time);

    foreach ($records as $r) {
        drupal_write_record('tow_link_tables', $r);
    }

    // Remaining records and tables.
    $results = db_query("

    SELECT
      *

      FROM
        {tow_link_tables} t

      WHERE
        t.timestamp < %d

  ", $time);
    $datasets = array();

    while ($item = db_fetch_array($results)) {
        if ($item['record_nid']) {
            if (!in_array($item['dataset_nid'], $datasets)) {
                tow_search_inner_linked_update_dataset_indexes($item['dataset_nid']);
                $datasets[] = $item['dataset_nid'];
            }
            tow_search_inner_linked_relink_table($item['table_nid']);
        } else {

            // Table.
            switch ($item['op']) {

                case 'delete':
                    tow_search_inner_linked_unlink_table($item['table_nid']);
                    tow_search_inner_linked_update_dataset_indexes($item['dataset_nid'], $item['table_nid']);
                    break;

                case 'insert':
                case 'update':
                    if (!in_array($item['dataset_nid'], $datasets)) {
                        tow_search_inner_linked_update_dataset_indexes($item['dataset_nid']);
                        $datasets[] = $item['dataset_nid'];
                    }
                    tow_search_inner_linked_relink_table($item['table_nid']);
                    break;
            }
        }
    }

    // Delete processed items.
    db_query("

    DELETE
      FROM
        {tow_link_tables} t

      WHERE
        t.timestamp < %d

  ", $time);
}

/**
 * 'Inner search: list of separate and linked tables of a dataset' block.
 */
function tow_search_inner_linked_list_of_tables_block() {

    // Checks if we are on the dataset search page.
    $nid = tow_search_inner_get_nid();
    if (!is_numeric($nid)) {
        $is_dataset = FALSE;
    } else {
        $is_dataset = db_result(db_query("SELECT COUNT(*) FROM {content_type_dataset} ctd WHERE ctd.nid = '%s'", $nid));
    }

    // Get dataset NID.
    if (!$is_dataset) {
        $table_nid = array_shift(explode('+', $nid));
        $dataset_nid = db_result(db_query("SELECT ctt.field_dataset_value AS field_dataset_value FROM {content_type_table} ctt WHERE ctt.nid = '%s'", $table_nid));
        unset($table_nid);

        if (!$dataset_nid) {
            return;
        }
    } else {
        $dataset_nid = $nid;
    }
    unset($nid);

    // Get tables and tablestes.
    list($tables, $tablesets) = tow_search_inner_linked_build_tablesets($dataset_nid);

    // Single table titles.
    $titles = array();
    $table_nodes = array();
    foreach ($tables as $table_nid) {
        $table_node = node_load($table_nid);
        $table_nodes[$table_node->nid] = $table_node;
        $titles[$table_nid] = $table_node->title;
    }

    // Linked table titles.
    $linked_titles = array();
    foreach ($tablesets as $tableset) {
        $linked_title = array();
        foreach ($tableset as $nid => $table) {
            $links = array();
            if (!empty($table['join'])) {
                foreach ($table['join'] as $link_nid => $link_fields) {
                    $fields = array();
                    foreach ($link_fields['fields'] as $field_info) {
                        $fields[] = $field_info['title'];
                    }
                    $links[] = 'linked to ' . $titles[$link_nid] . ' by ' . implode(', ', $fields);
                }
            }
            $linked_title[] = $titles[$nid] . (empty($links) ? '' : ' (' . implode('; ', $links) . ')');
        }
        $linked_titles[] = implode(', ', $linked_title);
    }

    // Full titles set.
    $titles = array_merge($titles, $linked_titles);

    return array('title' => 'List of tables', 'content' => theme('item_list', $titles));
}

/**
 * Part of hook_nodeapi() implementation. Record linking.
 */
function tow_search_inner_linked_queue_record_linking($node, $op) {

    // Tell the system of a linked record has been inserted, updated or deleted.
    $table = node_load($node->field_table[0]['value']);
    switch ($op) {
        case 'insert':
        case 'update':
        case 'delete':
            db_query("

        INSERT
          INTO
            {tow_link_tables}
              (record_nid, table_nid, dataset_nid, op, timestamp)

          VALUES
            (%d, %d, %d, '%s', %d)

      ", $node->nid, $node->field_table[0]['value'], $table->field_dataset[0]['value'], $op, time());
            break;
        default:
            break;
    }
}

/**
 * Part of hook_nodeapi() implementation. Table linking.
 */
function tow_search_inner_linked_queue_table_linking($node, $op) {
    $needed = FALSE;

    switch ($op) {

        // Check table indexes on presave.
        case 'presave':
            if (!empty($node->nid))
                tow_search_inner_linked_check_table_indexes($node);
            break;

        // Check if indexes had been changed on update. Recording is neede if so.
        case 'update':
            if (isset($_SESSION['table_indexes_changed'][$node->nid])) {
                $needed = TRUE;
                unset($_SESSION['table_indexes_changed'][$node->nid]);
            }
            break;

        // On insertion or deletion recording is always needed.
        case 'insert':
        case 'delete':
            $needed = TRUE;
            break;
        default:
            break;
    }

    // Write records.
    if ($needed) {
        db_query("

      INSERT
        INTO
          {tow_link_tables}
            (record_nid, table_nid, dataset_nid, op, timestamp)

        VALUES
          (%d, %d, %d, '%s', %d)

    ", 0, $node->nid, $node->field_dataset[0]['value'], $op, time());
    }
}

/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Builds all linked tablesets for given dataset.
 */
function tow_search_inner_linked_build_tablesets($dataset_nid) {

    // Retrieving tables nids.
    $tables = array();
    $results = db_query("SELECT nid FROM {content_type_table} WHERE field_dataset_value='%d'", $dataset_nid);
    while ($result = db_fetch_array($results)) {
        $tables[] = $result['nid'];
    };
    sort($tables);
    $shifted_tables = $tables;

    $tablesets = array();

    while (!empty($shifted_tables)) {
        $table = array_shift($shifted_tables);
        $tableset = array(
            $table => array(
                'select' => array(),
                'join' => NULL,
            )
        );
        _tow_search_inner_linked_add_table($tablesets, $shifted_tables, $tableset, array($table));
    }
    return array($tables, $tablesets);
}

/**
 * Builds linked tablesets containing given table.
 */
function tow_search_inner_linked_build_table_tablesets($base_table) {

    $base_table_node = node_load($base_table, NULL, TRUE);
    $dataset_nid = $base_table_node->field_dataset[0]['value'];

    // Retrieving tables nids.
    $tables = array(); // Tables of the same dataset as $base_table.
    $results = db_query("SELECT nid FROM {content_type_table} WHERE field_dataset_value='%d' AND nid<>'%d'", $dataset_nid, $base_table);
    while ($result = db_fetch_array($results)) {
        $tables[] = $result['nid'];
    };
    if (empty($tables)) {
        return NULL;
    }

    sort($tables);

    $tableset = array(
        $base_table => array(
            'select' => array(),
            'join' => NULL,
        )
    );
    $tablesets = array();
    _tow_search_inner_linked_add_table($tablesets, $tables, $tableset, array($base_table));

    if (empty($tablesets)) {
        return NULL;
    } else {
        return $tablesets[0];
    }
}

/**
 * Checks if the fields had been changed.
 */
function tow_search_inner_linked_check_table_indexes($changed_node) {
    $nid = $changed_node->nid;
    $orig_node = node_load($nid, NULL, TRUE);
    $flag = FALSE;

    // Check if fields were changed.
    foreach (tow_possible_fields() as $field) {
        $orig_fields = $orig_node->{'field_title_' . $field};
        $changed_fields = $changed_node->{'field_title_' . $field};
        if (count($orig_fields) == count($changed_fields)) {
            foreach ($orig_fields as $key => $orig_field) {
                if ($orig_field['value'] != $changed_fields[$key]['value']) {
                    $flag = TRUE;
                    break;
                }
            }
            if ($flag) {
                break;
            }
        } else {
            $flag = TRUE;
            break;
        }
    }

    // Changed.
    if ($flag) {

        // Save to session NIDs of tables with changed indexes.
        if (!isset($_SESSION['table_indexes_changed'])) {
            $_SESSION['table_indexes_changed'] = array();
        }
        $_SESSION['table_indexes_changed'][$nid] = 1;
    }

    // Not changed.
    else {

        // Unset info for the table of changed indexs.
        if (isset($_SESSION['table_indexes_changed'])) {
            unset($_SESSION['table_indexes_changed'][$nid]);
        }
    }
}

/**
 * Creates new node of 'linked_record' type filled with nids of nodes being linked.
 */
function tow_search_inner_linked_create_linked_record($nids, $tables, $deleted_nid = FALSE) {

    // Prepare node object.
    $node = new stdClass();
    if ($deleted_nid) {
        $node->nid = $deleted_nid;
        $node->vid = $deleted_nid;
    }
    $aux_nids = array();
    foreach ($nids as $nid) {
        $aux_nids[] = (is_null($nid)) ? '[NULL]' : $nid;
    }
    $node->name = "Links " . implode(', ', $aux_nids);
    $node->title = $node->name;
    $node->body = '';
    $node->type = 'linked_record';
    $node->promote = 0;
    $node->sticky = 0;
    $node->format = 0;
    $node->status = 1;

    // Save the info on records and tables.
    foreach ($nids as $key => $nid) {
        $node->field_record_nid[] = array('value' => _tow_field_insert_fake($nid)); //
        $node->field_tables[] = array('value' => array_shift($tables)); //
    }

    // Save node if all the values are apropriate ones.
    if ($node = node_submit($node)) {
        try {
            node_save($node);
        } catch (Exception $e) {
            watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
        }
    }
    // Show an error message otherwise.
    else {
        drupal_set_message(t("Linked record linking records " . implode(', ', $nids) . " added incorrectly"), "error");
    }
}

/**
 * Create new linked records upon creation of a new record.
 */
function tow_search_inner_linked_link_record($item) {
    $table_nid = $item['table_nid'];

    // Build tablesets containing current table.
    $tableset = tow_search_inner_linked_build_table_tablesets($table_nid);
    if (is_null($tableset)) {
        return;
    }

    // Create joined record nodes.
    $record_info = array('record' => $item['record_nid'], 'table' => $table_nid);
    $results = tow_search_inner_linked_link_records($tableset, $record_info);
    $tables = _tow_search_inner_linked_retrieve_tableset_tables($tableset);
    while ($nids = db_fetch_array($results)) {
        tow_search_inner_linked_create_linked_record($nids, $tables);
    }
}

/**
 * Builds query to retrieve nids of records being linked and returns SQL response with these nids.
 *
 * @param
 *   $tableset
 * @param
 *   $record_info
 *     NULL or array('record' => $record_nid, 'table' => $table_nid),
 *     if not empty, contains nid of record and nid of table
 *     for the case when single record is linked to other tables.
 */
function tow_search_inner_linked_link_records($tableset, $record_info = NULL) {

    $select = array();
    $queries = array();
    $ons = array();
    foreach ($tableset as $key => $table) {

        // {content_type_record} alias: ctr.
        $ctr_alias = "ctr";

        // Arrays for field tables, fields, conditions and deltas.
        $content_field_tables = array(); // {content_field_int} AS  cfi1,
        $content_fields = array(); // "cfi1.field_int_value
        $conditions = array(); // cfi1.nid = ctr1.nid
        $deltas = array(); // cfi1.delta = 0";
        foreach ($table['select'] as $cfkey => $field) {

            // Table alias: cf . $field['type'] . $cfkey.
            $table_alias = 'cf' . $cfkey . '';

            // Table: e.g. {content_field_int} AS cfi1.
            $content_field_tables[] = '{content_field_' . $field['type'] . '} AS ' . $table_alias;

            // Field: e.g. cfi1.field_int_value.
            $content_fields[] = $table_alias . '.field_' . $field['type'] . '_value AS ' . $field['type'] . $field['delta'] . '';

            // Delta: e.g. cfi1.delta = 0.
            $deltas[] = $table_alias . '.delta = ' . $field['delta'];
            if (empty($record_info) || $record_info['table'] != $key) {

                // Condition: cfi1.nid = ctr1.nid.
                $conditions[] = $table_alias . '.nid = ' . $ctr_alias . '.nid';
            } else {
                $conditions[] = $table_alias . '.nid = ' . $record_info['record'];
            }
        }

        // Implode arrays for field tables, fields, conditions and deltas.
        $content_field_tables = implode(', ', $content_field_tables);
        $content_fields = implode(', ', $content_fields);
        $conditions = implode(' AND ', $conditions);
        $deltas = implode(' AND ', $deltas);

        // Construct query.
        if (empty($record_info) || $record_info['table'] != $key) {
            $query =
                    "SELECT " . $ctr_alias . ".nid,  " . $content_fields .
                    " FROM {content_type_record} AS  " . $ctr_alias .
                    ", " . $content_field_tables .
                    " WHERE " . $ctr_alias . ".field_table_value = " . $key .
                    " AND " . $conditions .
                    " AND " . $deltas;
        } else {
            $query =
                    "SELECT " . $record_info['record'] . " AS nid,  " . $content_fields .
                    " FROM " . $content_field_tables .
                    " WHERE " . $conditions .
                    " AND " . $deltas;
        }

        // On-conditions.
        $on = array();
        if (!empty($table['join'])) {
            foreach ($table['join'] as $table_key => $to_table) {
                foreach ($to_table['fields'] as $field_key => $field) {
                    $to_field = $to_table['to_fields'][$field_key];
                    $on[] = 't' . $key . '.' . $field['type'] . $field['delta'] . ' = ' . 't' . $table_key . '.' . $to_field['type'] . $to_field['delta'] . '';
                }
            }
        }

        // Define select, queries and ons for the $key.
        $select[$key] = 't' . $key . '.nid AS t' . $key . 'nid';
        $queries[$key] = $query;
        $ons[$key] = implode(' AND ', $on);
    }

    // Build query.
    $select = implode(', ', $select);
    if (empty($record_info)) {
        $query = _tow_search_inner_linked_build_linking_query($select, $queries, $ons);
    } else {
        $query = _tow_search_inner_linked_build_linking_query($select, $queries, $ons, $record_info['table']);
    }

    return db_query($query);
}

/**
 * Changes linked records containing the record.
 */
function tow_search_inner_linked_relink_record($item) {
    $table_nid = $item['table_nid'];

    $deleted_nids = tow_search_inner_linked_unlink_record($item, TRUE);

    // Build tablesets containing current table.
    $tableset = tow_search_inner_linked_build_table_tablesets($table_nid);
    if (is_null($tableset))
        return;

    // Create joined record nodes.
    $record_info = array('record' => $item['record_nid'], 'table' => $table_nid);
    $results = tow_search_inner_linked_link_records($tableset, $record_info);
    $tables = _tow_search_inner_linked_retrieve_tableset_tables($tableset);
    while ($nids = db_fetch_array($results)) {
        tow_search_inner_linked_create_linked_record($nids, $tables, array_shift($deleted_nids));
    }

    while ($nid = array_shift($deleted_nids)) {
        node_delete($nid);
    }
}

/**
 * If necessary changes linked records upon table change.
 */
function tow_search_inner_linked_relink_table($table_nid) {

    // Find all records for table being changed.
    $deleted_nids = db_query("

    SELECT
      t.nid AS nid

      FROM
        {content_field_tables} t

      WHERE
        field_tables_value = '%s'

  ", $table_nid);

    // Build tablesets containing current table.
    $tableset = tow_search_inner_linked_build_table_tablesets($table_nid);
    if (is_null($tableset)) {
        return;
    }

    // Create joined record nodes.
    $results = tow_search_inner_linked_link_records($tableset);
    $tables = _tow_search_inner_linked_retrieve_tableset_tables($tableset);
    $counter = 0;
    while ($nids = db_fetch_array($results)) {
        $counter++;
        $deleted_nid = db_result($deleted_nids);
        tow_search_inner_linked_create_linked_record($nids, $tables, $deleted_nid);
    }

    while ($nid = db_result($deleted_nids)) {
        node_delete($nid);
    }
}

/**
 * Deletes linked records containing the record.
 */
function tow_search_inner_linked_unlink_record($item, $do_not_delete = FALSE) {

    // Find all joined records containing record being changed.
    $nids = db_query("SELECT nid FROM {content_field_record_nid} WHERE field_record_nid_value='%d'", $item['record_nid']);

    // Delete them.
    $deleted_nids = array();
    while ($nid = db_result($nids)) {

        // If there are the other records in the linked record, put null instead of current record.
        $linked_record = node_load($nid);
        $is_empty = TRUE;
        foreach ($linked_record['field_record_nid'] as &$record_nid) {
            if ($record_nid['value'] == $nid) {
                $record_nid['value'] = NULL;
            } elseif (!is_null($record_nid['value'])) {
                $is_empty = FALSE;
            }
        }

        if ($is_empty) {

            // Delete linked record otherwise.
            if ($do_not_delete) {
                $deleted_nids[] = $nid;
            } else {
                node_delete($nid);
            }
        } else {
            try {
                node_save($linked_record);
            } catch (Exception $e) {
                watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
            }
        }
    }
    if ($do_not_delete) {
        return $deleted_nids;
    }
}

/**
 * Deletes all linked records with current table.
 */
function tow_search_inner_linked_unlink_table($table_nid) {

    // Find all records for table being deleted.
    $deleted_nids = db_query("

    SELECT
      t.nid AS nid

      FROM
        {content_field_tables} t

      WHERE
        field_tables_value = '%s'

  ", $table_nid);

    // Delete them.
    while ($nid = db_result($deleted_nids)) {
        node_delete($nid);
    }
}

/**
 * Update dataset indexes.
 */
function tow_search_inner_linked_update_dataset_indexes($dataset_nid, $table_to_delete = NULL) {

    // Load tables.
    $results = db_query("SELECT nid FROM {content_type_table} WHERE field_dataset_value = %d", $dataset_nid);
    while ($nid = db_result($results)) {
        if ($nid != $table_to_delete) {
            $tables[] = node_load($nid, NULL, TRUE);
        }
    };
    if (empty($tables)) {
        return;
    }

    // Loop tables.
    foreach ($tables as $base_table) {

        // Get weights and indexes.
        $indexes = unserialize($base_table->field_indexes[0]['value']);
        $weights = unserialize($base_table->field_weights[0]['value']);
        $ii = 0;

        // Define the field that had been changed.
        foreach (tow_possible_fields() as $type) {
            foreach ($base_table->{'field_title_' . $type} as &$base_field) {
                if ($base_field['value'] != NULL) {
                    $flag = FALSE;
                    foreach ($tables as $table) {
                        if ($table->nid != $base_table->nid) {
                            foreach ($table->{'field_title_' . $type} as $field) {
                                if ($base_field['value'] == $field['value']) {
                                    $flag = TRUE;
                                    break;
                                }
                            }
                        }
                        if ($flag)
                            break;
                    }
                    $indexes[$weights[$ii]] = $flag ? 1 : 0;
                    $ii++;
                }
            }
        }

        db_query("UPDATE {content_type_table} SET field_indexes_value = '%s' WHERE nid = %d", serialize($indexes), $base_table->nid);
        db_query("DELETE FROM {cache_content} WHERE cid = 'content:%d:%d'", $base_table->nid, $base_table->nid);
    }

    // Load the node.
    $node = array_shift($tables);
    if ($node) {
        node_load($node->nid, NULL, TRUE); // Reset static cache.
    }
}

/* * **************************************************************************************************
 * Helper functions
 * ************************************************************************************************** */

/**
 * Recursively adds linked tables.
 */
function _tow_search_inner_linked_add_table(&$tablesets, &$tables, $tableset, $tables_in) {
    $tableset_is_updated = FALSE;
    foreach ($tables as $key => $new_table) {
        $new_table_is_linked = FALSE;
        $linked_to = array();
        $select = array();
        foreach ($tables_in as $table_in) {
            $linked_to_instance = _tow_search_inner_linked_to($table_in, $new_table);
            if (!empty($linked_to_instance)) {
                $linked_to[$table_in] = $linked_to_instance;
                foreach ($linked_to_instance['fields'] as $new_field) {
                    if (!in_array($new_field, $select)) {
                        $select[] = $new_field;
                    }
                }
                foreach ($linked_to_instance['to_fields'] as $new_field) {
                    if (!in_array($new_field, $tableset[$table_in]['select'])) {
                        $tableset[$table_in]['select'][] = $new_field;
                    }
                }
            }
        }
        if (!empty($linked_to)) {
            $tableset[$new_table] = array(
                'select' => $select,
                'join' => $linked_to,
            );
            $tableset_is_updated = TRUE;
            unset($tables[$key]);
            _tow_search_inner_linked_add_table($tablesets, $tables, $tableset, array_merge($tables_in, array($new_table)));
            break;
        }
    }
    if (!$tableset_is_updated && count($tableset) > 1) {
        $tablesets[] = $tableset;
    }
}

/**
 * Builds FULL JOINed query ,
 */
function _tow_search_inner_linked_build_linking_query($select, $queries, $ons, $relinked_record_table = NULL) {
    $n = count($ons) - 1;
    $query = array();

    // Prepare query.
    for ($i = 0; $i < $n; $i++) {
        $tables_query = array();
        if ($relinked_record_table) {
            $nulls = array('t' . $relinked_record_table . '.nid IS NOT NULL');
        } else {
            $nulls = array();
        }
        foreach ($queries as $table_nid => $table_query) {
            $on = $ons[$table_nid];
            $table = '(' . $table_query . ') AS t' . $table_nid . '';
            if (empty($on)) {

                // First table -> no join.
                $tables_query[] = $table;
            } else {
                $tables_query[] = 'FULL JOIN ' . $table . ' ON ' . $on;
            }
        }

        $tables_query = implode(' ', $tables_query);
        if (empty($nulls)) {
            $where = '';
        } else {
            $where = ' WHERE ' . implode(' AND ', $nulls);
        }
    }

    // Build query.
    $query = 'SELECT ' . $select . ' FROM ( ' . $tables_query . ' )' . $where;

    return $query;
}

/**
 * Reduces a field.
 */
function _tow_search_inner_linked_reduce_field($field) {
    return array(
        'type' => $field['type'],
        'title' => $field['title'],
        'delta' => $field['delta'],
    );
}

/**
 * Retrieves tables of records being linked as serialized array.
 */
function _tow_search_inner_linked_retrieve_tableset_tables($tableset) {
    $tables = array();

    // Retrieve tables.
    foreach ($tableset as $key => $table) {
        $tables[] = $key;
    }

    return $tables;
}

/**
 * Replaces auxiliary fake values with empty ones.
 */
function _tow_search_inner_linked_set_empty(&$node) {
    foreach ($node->field_record_nid as &$nid) {
        $nid['value'] = _tow_field_remove_fake($nid['value']);
    }
}

/**
 * Table indexes.
 */
function _tow_search_inner_linked_table_indexes($nid) {
    static $indexes_array = array();
    if (!isset($indexes_array[$nid])) {
        $indexes = array();
        $fields = tow_apachesolr_table_fields($nid);
        foreach ($fields as $field) {
            if ($field['index']) {
                $indexes[] = _tow_search_inner_linked_reduce_field($field);
            }
        }
        $indexes_array[$nid] = $indexes;
    }
    return $indexes_array[$nid];
}

/**
 * Defines the fields a table is linked to another one by.
 */
function _tow_search_inner_linked_to($to_nid, $nid) {
    $fields = array();
    $to_fields = array();
    foreach (_tow_search_inner_linked_table_indexes($nid) as $field) {
        foreach (_tow_search_inner_linked_table_indexes($to_nid) as $to_field) {
            if ($field['title'] == $to_field['title'] && $field['type'] == $to_field['type']) {
                $fields[] = $field;
                $to_fields[] = $to_field;
            }
        }
    }
    return empty($fields) ? NULL : array('fields' => $fields, 'to_fields' => $to_fields);
}

/**
 * Defines a linking fields within a tableset.
 */
function _tow_search_inner_linking_fields($tableset_nids) {
    $return = array();
    foreach ($tableset_nids as $to_nid_key => $to_nid) {
        foreach ($tableset_nids as $nid_key => $nid) {
            if ($nid == $to_nid) {
                continue;
            }
            $fields = _tow_search_inner_linked_to($to_nid, $nid);
            $fields = isset($fields['fields']) ? $fields['fields'] : array();
            foreach ($fields as $field) {
                $return[$field['title'] . '_' . $field['type']] = $field;
            }
        }
        unset($tableset_nids[$to_nid_key]);
    }

    return $return;
}