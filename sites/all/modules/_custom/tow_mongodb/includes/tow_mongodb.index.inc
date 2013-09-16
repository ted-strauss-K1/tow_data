<?php

/**
 * @file
 *
 * ToW MongoDB indexing operations.
 */

/**
 * Truncate and rebuild the tow_mongodb_index table, reset the tow_mongodb_index_last variable.
 * This is the most complete way to force reindexing, or to build the indexing table for the first time.
 */
function tow_mongodb_index_rebuild_index_table() {

    db_query("DELETE FROM {tow_mongodb_index}");

    // Populate table.
    if (module_exists('comment')) {

        // If comment module is enabled, use last_comment_timestamp as well.
        db_query("INSERT INTO {tow_mongodb_index} (nid, status, changed)
              SELECT n.nid, n.status, GREATEST(n.created, n.changed, COALESCE(c.last_comment_timestamp, 0)) AS changed
              FROM {node} n
              LEFT JOIN {node_comment_statistics} c ON n.nid = c.nid");
    } else {
        db_query("INSERT INTO {tow_mongodb_index} (nid, status, changed)
              SELECT n.nid, n.status, GREATEST(n.created, n.changed) AS changed
              FROM {node} n");
    }

    // Make sure no nodes end up with a timestamp that's in the future.
    $time = time();
    db_query("UPDATE {tow_mongodb_index} SET changed = %d WHERE changed > %d", $time, $time);
    tow_mongodb_index_clear_last_index();
}

/**
 * Returns last changed and last nid.
 * 
 * @return array MongoDB last index.
 */
function tow_mongodb_index_get_last_index() {
    return variable_get('tow_mongodb_index_last', array());
}

/**
 * Clear last changed and nid.
 */
function tow_mongodb_index_clear_last_index() {
    variable_del('tow_mongodb_index_last');
}

/**
 * Mark all nodes of one type as needing re-indexing.
 */
function tow_mongodb_index_mark_node_type($type_name) {
    switch ($GLOBALS['db_type']) {
        case 'mysql':
        case 'mysqli':
            db_query("UPDATE {tow_mongodb_index} asn INNER JOIN {node} n ON asn.nid = n.nid SET asn.changed = %d WHERE n.type = '%s'", time(), $type_name);
            break;
        default:
            db_query("UPDATE {tow_mongodb_index} SET changed = %d WHERE nid IN (SELECT nid FROM {node} WHERE type = '%s')", time(), $type_name);
            break;
    }
}

/**
 * Mark one node as needing re-indexing.
 */
function tow_mongodb_index_mark_node($nid) {
    db_query("UPDATE {tow_mongodb_index} SET changed = %d WHERE nid = %d", time(), $nid);
}

/**
 * Helper function to keep track of when the index has been updated.
 */
function tow_mongodb_index_updated($updated = NULL) {
    if (isset($updated)) {
        if ($updated) {
            variable_set('tow_mongodb_index_updated', (int) $updated);
        } else {
            variable_del('tow_mongodb_index_updated');
        }
    }
    return variable_get('tow_mongodb_index_updated', 0);
}

function tow_mongodb_index_delete_node_from_index($node) {
    try {
        $mongo = tow_mongodb_get_mongo();
        if ($node->type == 'dataset') {
            $criteria = array('_id' => $node->nid);
        }

        // Table.
        if ($node->type == 'table') {

            $criteria = array('_id' => $node->field_dataset[0]['value']);
            $value = array(
                '$pull' => array(
                    'tables' => array(
                        '_id' => $node->nid
                    ),
                ),
            );
        }

        // Record.
        if ($node->type == 'record') {
            $table = node_load($node->field_table[0]['value']);
            $criteria = array('_id' => $table->field_dataset[0]['value'], 'tables._id' => $table->nid);
            $value = array(
                '$pull' => array(
                    'tables.$.rows' => array(
                        '_id' => $node->nid,
                    ),
                ),
            );
        }

        // Linked record.
        if ($node->type == 'linked_record') {
            $tables = array();
            foreach ($node->field_tables as $f) {
                $tables[] = $f['value'];
            }
            sort($tables);
            $table_nid = implode('+', $tables);
            $dataset_nid = node_load($tables[0])->field_dataset[0]['value'];
            $criteria = array('_id' => $dataset_nid, 'tables._id' => $table_nid);
            $value = array(
                '$pull' => array(
                    'tables.$.rows' => array(
                        '_id' => $node->nid,
                    ),
                ),
            );
        }

        if ($node->type == 'dataset') {
            $mongo->datasets->remove($criteria);
        } else {
            $mongo->datasets->update($criteria, $value);
        }

        tow_mongodb_index_updated(time());
        return TRUE;
    } catch (Exception $e) {
        watchdog('ToW MongoDB', nl2br(check_plain($e->getMessage())), NULL, WATCHDOG_ERROR);

        // Don't keep trying queries if they are failing.
        return FALSE;
    }
}

/**
 * Function to handle the indexing of nodes.
 *
 * The calling function must supply a name space or track/store
 * the timestamp and nid returned.
 * Returns FALSE if no nodes were indexed (none found or error).
 */
function tow_mongodb_index_nodes($rows) {
    if (!$rows) {

        // Nothing to do.
        return FALSE;
    }

    try {
        $mongo = tow_mongodb_get_mongo();
    } catch (Exception $e) {
        watchdog('ToW MongoDB', nl2br(check_plain($e->getMessage())), NULL, WATCHDOG_ERROR);
        return FALSE;
    }
    $documents = array();
    $old_position = tow_mongodb_index_get_last_index();
    $position = $old_position;

    // Always build the content for the index as an anonynmous user.
    global $user;
    session_save_session(FALSE);
    $saved_user = $user;
    $user = drupal_anonymous_user();

    foreach ($rows as $row) {
        try {
            tow_mongodb_index_add_node_document($documents, $row->nid);

            // Variables to track the last item changed.
            $position['last_change'] = $row->changed;
            $position['last_nid'] = $row->nid;
        } catch (Exception $e) {

            // Something bad happened - log the error.
            watchdog('ToW MongoDB', 'Error constructing documents to index: <br /> !message', array('!message' => "Node ID: {$row->nid}<br />" . nl2br(strip_tags($e->getMessage()))), WATCHDOG_ERROR);
        }
    }
    // Restore the user.
    $user = $saved_user;
    session_save_session(TRUE);

    if (count($documents)) {
        try {
            watchdog('ToW MongoDB', 'Adding @count documents.', array('@count' => count($documents)));
            // Chunk the adds by 20s
            $docs_chunk = array_chunk($documents, 20);
            foreach ($docs_chunk as $docs) {
                $mongo = tow_mongodb_get_mongo();
                foreach ($docs as $doc) {
                    $criteria = $doc['criteria'];
                    $values = $doc['values'];
                    $upsert = TRUE;
                    foreach ($values as $value) {
                        $mongo->datasets->update($criteria, $value, array('upsert' => $upsert));
                    }
                }
            }

            // Set the timestamp to indicate an index update.
            tow_mongodb_index_updated(time());
        } catch (Exception $e) {
            $nids = array();
            if (!empty($docs)) {
                foreach ($docs as $doc) {
                    $nids[] = $doc->nid;
                }
            }
            watchdog('ToW MongoDB', 'Indexing failed on one of the following nodes: @nids <br /> !message', array(
                '@nids' => implode(', ', $nids),
                '!message' => nl2br(strip_tags($e->getMessage())),
                    ), WATCHDOG_ERROR);
            return FALSE;
        }
    }

    // Save the new position in case it changed.
    variable_set('tow_mongodb_index_last', $position);

    return $position;
}

/**
 * Returns an array of rows from a query based on an indexing namespace.
 */
function tow_mongodb_index_get_nodes_to_index($limit) {
    extract(tow_mongodb_index_get_last_index());
    $args = array($last_change, $last_change, $last_nid);
    $rows = array();
    $result = db_query_range("SELECT tmi.nid, tmi.changed FROM {tow_mongodb_index} tmi INNER JOIN {node} n ON n.nid = tmi.nid WHERE (tmi.changed > %d OR (tmi.changed = %d AND tmi.nid > %d)) AND tmi.status = 1 ORDER BY tmi.changed ASC, tmi.nid ASC", $args, 0, $limit);
    while ($row = db_fetch_object($result)) {
        $rows[] = $row;
    }
    return $rows;
}

/**
 * Utility function to delete the index and reset all index counters.
 */
function tow_mongodb_index_delete_index() {

    // Instantiate a new Solr object.
    $mongo = tow_mongodb_get_mongo();
    $mongo->datasets->drop();
    $mongo->createCollection('datasets');

    // Rebuild our node-tracking table.
    tow_mongodb_index_rebuild_index_table();
    tow_mongodb_index_updated(time());
}

/**
 * A helper to try to make {tow_mongodb_index} consistent with {node}.
 */
function tow_mongodb_index_check_node_table() {

    // Check for unpublished content that wasn't deleted from the index.
    $result = db_query("SELECT n.nid, n.status FROM {tow_mongodb_index} asn INNER JOIN {node} n ON n.nid = asn.nid WHERE asn.status <> n.status");
    $node_lists = array();
    $nodes = array();

    // Update or delete at most this many in each Solr query.
    $limit = variable_get('tow_mongodb_cron_mass_limit', 500);
    while ($node = db_fetch_object($result)) {
        $nodes[$node->nid] = $node;
        if (count($nodes) == $limit) {
            $node_lists[] = $nodes;
            $nodes = array();
        }
    }

    // Any remaning ones if the limit is not reached.
    if (count($nodes)) {
        $node_lists[] = $nodes;
    }
    foreach ($node_lists as $nodes) {
        if (!tow_mongodb_index_nodeapi_mass_update($nodes)) {

            // Solr query failed - so stop trying.
            break;
        }
    }

    // Check for deleted content that wasn't deleted from the index.
    $result = db_query("SELECT asn.nid FROM {tow_mongodb_index} asn LEFT JOIN {node} n ON n.nid = asn.nid WHERE n.nid IS NULL");
    $node_lists = array();
    $nodes = array();
    while ($node = db_fetch_object($result)) {
        $nodes[$node->nid] = $node;
        if (count($nodes) == $limit) {
            $node_lists[] = $nodes;
            $nodes = array();
        }
    }

    // Any remaning ones if the limit is not reached.
    if (count($nodes)) {
        $node_lists[] = $nodes;
    }
    foreach ($node_lists as $nodes) {
        if (!tow_mongodb_index_nodeapi_mass_delete($nodes)) {

            // Solr query failed - so stop trying.
            break;
        }
    }
}

function tow_mongodb_index_nodeapi_mass_update($nodes) {
    if (empty($nodes)) {
        return TRUE;
    }

    $published_ids = array();
    $unpublished_ids = array();
    
    $errors = 0;
    
    foreach ($nodes as $node) {
        if ($node->status) {
            $published_ids[$node->nid] = $node->nid;
        } else {
            $unpublished_ids[$node->nid] = $node->nid;
            if (!tow_mongodb_index_delete_node_from_index($node)) {
                $errors++;
            }
        }
    }
    
    if ($errors == 0) {
        // There was no exception, so update the table.
        if ($published_ids) {
            db_query('UPDATE {tow_mongodb_index} SET changed = %d, status = 1 WHERE nid IN (' . db_placeholders($published_ids) . ')', array_merge((array) $time, array_keys($published_ids)));
        }
        if ($unpublished_ids) {
            db_query('UPDATE {tow_mongodb_index} SET changed = %d, status = 0 WHERE nid IN (' . db_placeholders($unpublished_ids) . ')', array_merge((array) $time, array_keys($unpublished_ids)));
        }
        return TRUE;
    } else {
        return FALSE;
    }

}

function tow_mongodb_index_nodeapi_mass_delete($nodes) {
    if (empty($nodes)) {
        return TRUE;
    }

    $ids = array();
    $nids = array();

    $errors = 0;
    
    foreach ($nodes as $node) {
        $ids[] = $node->nid;
        $nids[] = $node->nid;
        if (!tow_mongodb_index_delete_node_from_index($node)) {
            $errors++;
        }
    }
    if ($errors == 0) {
        // There was no exception, so update the table.
        db_query("DELETE FROM {tow_mongodb_index} WHERE nid IN (" . db_placeholders($nids) . ")", $nids);
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * Add a document to the $documents array based on a node ID.
 */
function tow_mongodb_index_add_node_document(&$documents, $nid) {
    if ($document = tow_mongodb_index_node_to_document($nid)) {
        $documents[] = $document;
    }
}

/**
 * Given a node ID, return a document representing that node.
 */
function tow_mongodb_index_node_to_document($nid) {

    // Set reset = TRUE to avoid static caching of all nodes that get indexed.
    $node = node_load($nid, NULL, TRUE);
    if (empty($node)) {
        return FALSE;
    }

    $mongo = tow_mongodb_get_mongo();

    // Dataset.
    if ($node->type == 'dataset') {
        $document['criteria'] = array('_id' => $node->nid);
        $document['values'][] = array(
            '$set' => array(
                'title' => $node->title,
            ),
        );
        list($tables, ) = tow_search_inner_linked_build_tablesets($node->nid);
        sort($tables);
        $table_nid = implode('+', $tables);
        $dataset_nid = $node->nid;
        $cursor = array_shift(iterator_to_array($mongo->datasets->find(array('tables._id' => $table_nid), array('tables._id' => TRUE, 'tables.rows' => TRUE))));
        $rows = array();
        if ($cursor['tables']) {
            foreach ($cursor['tables'] as $t) {
                $rows = ($t['_id'] == $table_nid) ? $t['rows'] : $rows;
            }
        }
        $document['values'][] = array(
            '$pull' => array(
                'tables' => array(
                    '_id' => $table_nid
                ),
            ),
        );
        $document['values'][] = array(
            '$push' => array(
                'tables' => array(
                    '_id' => $table_nid,
                    'header' => tow_apachesolr_table_fields($table_nid),
                    'rows' => $rows
                ),
            ),
        );
    }

    // Table.
    if ($node->type == 'table') {
        $cursor = array_shift(iterator_to_array($mongo->datasets->find(array('tables._id' => $node->nid), array('tables._id' => TRUE, 'tables.rows' => TRUE))));
        $rows = array();
        if ($cursor['tables']) {
            foreach ($cursor['tables'] as $t) {
                $rows = ($t['_id'] == $node->nid) ? $t['rows'] : $rows;
            }
        }
        $document['criteria'] = array('_id' => $node->field_dataset[0]['value']);
        $document['values'][] = array(
            '$pull' => array(
                'tables' => array(
                    '_id' => $node->nid
                ),
            ),
        );
        $document['values'][] = array(
            '$push' => array(
                'tables' => array(
                    '_id' => $node->nid,
                    'header' => tow_apachesolr_table_fields($node->nid),
                    'rows' => $rows
                ),
            ),
        );
    }

    // Record.
    if ($node->type == 'record') {
        $table = node_load($node->field_table[0]['value']);
        $document['criteria'] = array('_id' => $table->field_dataset[0]['value'], 'tables._id' => $table->nid);
        $document['values'][] = array(
            '$pull' => array(
                'tables.$.rows' => array(
                    '_id' => $node->nid,
                ),
            ),
        );
        $values = array();
        $header = tow_apachesolr_table_fields($table->nid);
        foreach ($header as $h) {
            $values[$h['name']] = $node->{'field_' . $h['type']}[$h['delta']]['value'];
        }
        $values['_id'] = $node->nid;
        $document['values'][] = array(
            '$push' => array(
                'tables.$.rows' => $values,
            ),
        );
    }

    // Linked record.
    if ($node->type == 'linked_record') {
        $tables = array();
        foreach ($node->field_tables as $f) {
            $tables[] = $f['value'];
        }
        sort($tables);
        $table_nid = implode('+', $tables);
        $dataset_nid = node_load($tables[0])->field_dataset[0]['value'];
        $document['criteria'] = array('_id' => $dataset_nid, 'tables._id' => $table_nid);
        $document['values'][] = array(
            '$pull' => array(
                'tables.$.rows' => array(
                    '_id' => $node->nid,
                ),
            ),
        );
        $tables = array();
        foreach ($node->field_tables as $table) {
            $tables[] = $table['value'];
        }
        $sorted_tables = $tables;
        sort($sorted_tables);

        // Put records.
        $records = array();
        foreach ($sorted_tables as $key => $table_nid) {
            $record_nid = $node->field_record_nid[array_search($table_nid, $tables)]['value'];
            $record = node_load($record_nid);
            $records[$table_nid] = $record;
        }
        foreach (tow_apachesolr_table_fields($tables) as $field) {
            $values[$field['name']] = NULL;
            foreach ($field['table'] as $table) {
                $record = $records[$table['nid']];
                if (!empty($record)) {
                    $values[$field['name']] = $record->{'field_' . $field['type']}[$table['delta']]['value'];
                    break;
                }
            }
        }
        $values['_id'] = $node->nid;
        $document['values'][] = array(
            '$push' => array(
                'tables.$.rows' => $values,
            ),
        );
    }

    return $document;
}

/**
 * Helper function for modules implmenting hook_search's 'status' op.
 */
function tow_mongodb_index_status() {
    extract(tow_mongodb_index_get_last_index());
    $args = array($last_change, $last_change, $last_nid);

    $total = db_result(db_query("SELECT COUNT(tmi.nid) FROM {tow_mongodb_index} tmi WHERE tmi.status = 1"));
    $remaining = db_result(db_query("SELECT COUNT(tmi.nid) FROM {tow_mongodb_index} tmi INNER JOIN {node} n ON n.nid = tmi.nid WHERE (tmi.changed > %d OR (tmi.changed = %d AND tmi.nid > %d)) AND tmi.status = 1", $args));
    $indexed = $total - $remaining;

    return array('indexed' => $indexed, 'remaining' => $remaining, 'total' => $total);
}