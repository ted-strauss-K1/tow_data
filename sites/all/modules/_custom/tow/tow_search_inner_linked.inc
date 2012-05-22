<?php 

/**
 * If necessary changes linked records upon table change
 */
function tow_search_inner_relink_table($table_nid) {mydpm('tow_search_inner_relink_table ' . $table_nid);
  // find all records for table being changed 
  $deleted_nids = db_query("SELECT `nid` FROM {content_field_tables}
                    WHERE `field_tables_value`='%d'", $table_nid);

  // build tablesets containing current table
  $tableset = tow_search_inner_build_table_tablesets($table_nid);
  if (is_null($tableset))
    return;
  mydpm($tableset);
  // create joint record nodes
  $results = tow_search_inner_link_records($tableset);
  $tables = tow_search_inner_retrieve_tableset_tables($tableset);
  while ($nids = db_fetch_array($results)) {
    $deleted_nid = db_result($deleted_nids);
    mydpm($deleted_nid . '->' . implode(' ', $nids));
    tow_search_inner_create_linked_record($nids, $tables, $deleted_nid);
  }
  
  while ($nid = db_result($deleted_nids)) {
    mydpm($nid);
    node_delete($nid);
  };  
}

/**
 * Deletes all linked records with current table
 */
function tow_search_inner_unlink_table($table_nid) {mydpm('tow_search_inner_unlink_table ' . $table_nid);
  // find all records for table being deleted
  $deleted_nids = db_query("SELECT `nid` FROM {content_field_tables}
                    WHERE `field_tables_value`='%d'", $table_nid);
  // delete them
  while ($nid = db_result($deleted_nids)) {
    node_delete($nid);
  };    
}

/**
 * Create new linked records upon creation of new record
 */
function tow_search_inner_link_record($item){mydpm('tow_search_inner_link_record ' . $item['record_nid']);
  $table_nid = $item['table_nid'];

  // build tablesets containing current table
  $tableset = tow_search_inner_build_table_tablesets($table_nid);
  if (is_null($tableset))
    return;

  // create joint record nodes
  $record_info = array('record' => $item['record_nid'], 'table' => $table_nid);
  $results = tow_search_inner_link_records($tableset, $record_info);
  $tables = tow_search_inner_retrieve_tableset_tables($tableset);
  while ($nids = db_fetch_array($results)) {
    tow_search_inner_create_linked_record($nids, $tables);
  }
}

/**
 * Retrieves tables of records being linked as serialized array
 */
function tow_search_inner_retrieve_tableset_tables($tableset) {
  $tables = array();
  foreach ($tableset as $key => $table) {
    $tables[] = $key;
  }
//  sort($tables);
  return $tables;
}
/**
 * Creates new node of 'linked_record' type filled with nids of nodes being linked
 */
function tow_search_inner_create_linked_record($nids, $tables, $deleted_nid = FALSE) {//mydpm($tables);
  $node = new stdClass();
  if ($deleted_nid) {
    $node->nid = $deleted_nid;
    $node->vid = $deleted_nid;
  }
  $node->name = "Links " . implode(', ', $nids);
  $node->title = $node->name;
  $node->body = '';
  $node->type = 'linked_record'; //This can be any node type
//  $node->field_tables[] = array('value' => $tables);
  $node->promote = 0; // Display on front page ? 1 : 0
  $node->sticky = 0;  // Display top of page ? 1 : 0
  $node->format = 0;  // 1:Filtered HTML, 2: Full HTML
  $node->status = 1;   // Published ? 1 : 0
  foreach ($nids as $key => $nid) {
    $node->field_record_nid[] = array('value' => _tow_insert_fake($nid)); //
    $node->field_tables[] = array('value' => array_shift($tables)); //
  }
  if ($node = node_submit($node)) {
    node_save($node);
    mydpm($node);
  } else {
    drupal_set_message(t("Linked record linking records " . implode(', ', $nids) . " added incorrectly"), "error");
  }
}

function tow_search_inner_set_empty(&$node) {
  foreach($node->field_record_nid as &$nid) {
    $nid['value'] = _tow_remove_fake($nid['value']);
  }
}

/**
 * Deletes linked records containing the record
 */
function tow_search_inner_unlink_record($item, $do_not_delete = FALSE){mydpm('tow_search_inner_unlink_record ' . $item['record_nid']);
  // find all joint records containing record being changed
  $nids = db_query("SELECT `nid` FROM {content_field_record_nid} WHERE `field_record_nid_value`='%d'", $item['record_nid']);
  // delete them
  $deleted_nids = array();
  while ($nid = db_result($nids)) {
    // if there are the other records in the linked record, put null instead of current record
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
      // delete linked record otherwise
      if ($do_not_delete) 
        $deleted_nids[] = $nid;
      else
        node_delete($nid);
    } else {
      node_save($linked_record);
    }
  }
  if ($do_not_delete)
    return $deleted_nids;
}

/**
 * Changes linked records containing the record
 */
function tow_search_inner_relink_record($item){mydpm('tow_search_inner_relink_record ' . $item['record_nid']);
  $table_nid = $item['table_nid'];
  
  $deleted_nids = tow_search_inner_unlink_record($item, TRUE);
  // build tablesets containing current table
  $tableset = tow_search_inner_build_table_tablesets($table_nid);
  if (is_null($tableset))
    return;

  // create joint record nodes
  $record_info = array('record' => $item['record_nid'], 'table' => $table_nid);
  $results = tow_search_inner_link_records($tableset, $record_info);
  $tables = tow_search_inner_retrieve_tableset_tables($tableset);
  while ($nids = db_fetch_array($results)) {
    tow_search_inner_create_linked_record($nids, $tables, array_shift($deleted_nids));
  }

  while ($nid = array_shift($deleted_nids)) {
    node_delete($nid);
  }
}

/**
 * Builds linked tablesets containing given table
 * @param: $base_table nid of base table
 */    
function tow_search_inner_build_table_tablesets($base_table) {

  // to allow cron work
  // TODO: modify _tow_search_inner_build_linking_query()
  //       for the case of many tables
  //       In the present state is takes too long timecs 
  

  $base_table_node = node_load($base_table, NULL, TRUE);
  $dataset_nid = $base_table_node->field_dataset[0]['value'];

  //retrieving tables nids
  $tables = array(); // tables of the same dataset as $base_table
  $results = db_query("SELECT nid FROM {content_type_table} WHERE field_dataset_value='%d' AND nid<>'%d'", $dataset_nid, $base_table);
  while ($result = db_fetch_array($results)) {
    $tables[] = $result['nid'];
  };
  if (empty($tables))
    return NULL;
    
  sort($tables);
    
  $tableset = array(
    $base_table => array(
      'select' => array(),
      'join' => NULL,
    )
  );
  $tablesets = array();
  _tow_search_inner_add_table($tablesets, $tables, $tableset, array($base_table)); 
  
  if (empty($tablesets))
    return NULL;
  else
    return $tablesets[0];
  
}
    
/**
 * Builds all linked tablesets for given dataset
 */    
function tow_search_inner_build_tablesets($dataset_nid) {
  //retrieving tables nids
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
    _tow_search_inner_add_table($tablesets, $shifted_tables, $tableset, array($table)); 
  }  
  return array($tables, $tablesets);
}

/**
 *
 */
function _tow_search_inner_add_table(&$tablesets, &$tables, $tableset, $tables_in) {
  $tableset_is_updated = FALSE;
  foreach ($tables as $key => $new_table) {
    $new_table_is_linked = FALSE;
    $linked_to = array();
    $select = array();
    foreach ($tables_in as $table_in) {
      $linked_to_instance = _tow_search_inner_linked_to($table_in, $new_table);
      if (!empty($linked_to_instance)) {
        $linked_to[$table_in] = $linked_to_instance;
        foreach($linked_to_instance['fields'] as $new_field) {
          if (!in_array($new_field, $select)) {
            $select[] = $new_field;
          }
        }
        foreach($linked_to_instance['to_fields'] as $new_field) {
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
      _tow_search_inner_add_table($tablesets, $tables, $tableset, array_merge($tables_in, array($new_table)));
      break;
    }    
  }
  if (!$tableset_is_updated && count($tableset) > 1) {
    $tablesets[] = $tableset;
  }
}

/**
 * Builds query to retrieve nids of records being linked and returns SQL response with these nids
 * @param
 *  $tableset
 * @param
 *  $record_info = NULL or array('record' => $record_nid, 'table' => $table_nid)if not empty, contains nid of record and nid of table 
 *        for the case when single record is linked to other tables
 */
function tow_search_inner_link_records($tableset, $record_info = NULL) {mydpm('tow_search_inner_link_records');
  $select = array();
  $queries = array();
  $ons = array();
  foreach ($tableset as $key => $table) {watchdog('linking', 'table ' . $key);
    $ctr_alias = "`ctr`"; //"`ctr" . $key . "`";
    $content_field_tables = array(); //  {content_field_int} AS  `cfi1`,
    $content_fields = array(); //"`cfi1`.`field_int_value`
    $conditions = array(); //`cfi1`.`nid` =  `ctr1`.`nid`
    $deltas = array(); //`cfi1`.`delta` =0";
    foreach ($table['select'] as $cfkey => $field) {
      $table_alias = '`cf' . $cfkey . '`';//'`cf' . $field['type'] . $cfkey . '`';
      $content_field_tables[] = '{content_field_' . $field['type'] . '} AS ' . $table_alias;  //  {content_field_int} AS  `cfi1`,
      $content_fields[] = $table_alias . '.`field_' . $field['type'] . '_value` AS `' . $field['type'] . $field['delta'] . '`'; //"`cfi1`.`field_int_value`
      $deltas[] = $table_alias . '.`delta` = ' . $field['delta']; //`cfi1`.`delta` =0";
      if (empty($record_info) || $record_info['table'] != $key)
        $conditions[] = $table_alias . '.`nid` = ' . $ctr_alias . '.`nid`'; //`cfi1`.`nid` =  `ctr1`.`nid`
      else
        $conditions[] = $table_alias . '.`nid` = ' . $record_info['record']; 
    }
    $content_field_tables = implode(', ', $content_field_tables);
    $content_fields = implode(', ', $content_fields);
    $conditions = implode(' AND ', $conditions);
    $deltas = implode(' AND ', $deltas);
    if (empty($record_info) || $record_info['table'] != $key) {
      $query = "SELECT " . $ctr_alias . ".`nid` ,  " . $content_fields . //"`cfi1`.`field_int_value`
            " FROM {content_type_record} AS  " . $ctr_alias . ", " 
            . $content_field_tables . //  {content_field_int} AS  `cfi1`,
            " WHERE " . $ctr_alias . ".`field_table_value` = " . $key . 
            " AND " . $conditions . //`cfi1`.`nid` =  `ctr1`.`nid`
            " AND " . $deltas; //`cfi1`.`delta` =0";
    } else {
      $query = "SELECT " . $record_info['record'] . " AS `nid`,  " . $content_fields . //"`cfi1`.`field_int_value`
            " FROM " . $content_field_tables . //  {content_field_int} AS  `cfi1`,
            " WHERE " . $conditions . //`cfi1`.`nid` =  $record
            " AND " . $deltas; //`cfi1`.`delta` =0";
    }
    $on = array();
    if (!empty($table['join'])) {
      foreach ($table['join'] as $table_key => $to_table) {
        foreach ($to_table['fields'] as $field_key => $field) {
          $to_field = $to_table['to_fields'][$field_key];
          $on[] = '`t' . $key . '`.`' . $field['type'] . $field['delta'] . '` = ' . '`t' . $table_key . '`.`' . $to_field['type'] . $to_field['delta'] . '`';
        }
      }
    }
    $select[$key] = '`t' . $key . '`.`nid` AS `t' . $key . 'nid`';
    $queries[$key] = $query;
    $ons[$key] = implode(' AND ', $on);
  }
  $select = implode(', ', $select);

  if (empty($record_info))
    $query = _tow_search_inner_build_linking_query($select, $queries, $ons);
  else
    $query = _tow_search_inner_build_linking_query($select, $queries, $ons, $record_info['table']);
  
  mydpm($tableset);
  mydpm(array($query));
  return db_query($query);

}

function _tow_search_inner_build_linking_query($select, $queries, $ons, $relinked_record_table = NULL){mydpm('_tow_search_inner_build_linking_query');
  $n = count($ons)-1;
  $query = array();
  for ($ii = 0; $ii < pow(2,$n); $ii++) {mydpm($ii);
    $jj = $ii;
    $tables_query = array();
    if ($relinked_record_table)
      $nulls = array('`t' . $relinked_record_table . '`.`nid` IS NOT NULL');
    else 
      $nulls = array();
    foreach ($queries as $table_nid => $table_query) {
      $on = $ons[$table_nid];
      $table = '(' . $table_query . ') AS `t' . $table_nid . '`';
      if (empty($on)) {
        // first table -> no join
        $tables_query[] = $table;
      } else {
        if ($jj & 1) {
          // LEFT JOIN
          $tables_query[] = 'LEFT JOIN ' . $table . ' ON ' . $on;
          $nulls[] = '`t' . $table_nid . '`.`nid` IS NULL';
        } else {
          // RIGHT JOIN
          $tables_query[] = 'RIGHT JOIN ' . $table . ' ON ' . $on;
        }
        $jj = $jj >> 1;
      }
    }
    $tables_query = implode(' ', $tables_query);
    if(empty($nulls)) {
      $where = '';
    } else {
      $where = ' WHERE ' . implode(' AND ', $nulls);
    }
    
    $query[] = 'SELECT ' . $select . ' FROM ( ' . $tables_query . ' )' . $where;
  }
  return implode(' UNION ALL ', $query);
}

function _tow_search_inner_linked_to($to_nid, $nid) {
  $fields = array();
  $to_fields = array();
  foreach (_tow_search_inner_table_indexes($nid) as $field) {
    foreach(_tow_search_inner_table_indexes($to_nid) as $to_field) {
      if ($field['title'] == $to_field['title'] && $field['type'] == $to_field['type']) {
        $fields[] = $field;
        $to_fields[] = $to_field;
      }
    }
  }
  return empty($fields) ? NULL : array('fields' => $fields, 'to_fields' => $to_fields);
}

function _tow_search_inner_reduce_field($field) {
  return array(
    'type' => $field['type'], 
    'title' => $field['title'], 
    'delta' => $field['delta'], 
  );
}
function _tow_search_inner_table_indexes($nid) {xdebug_break();
  static $indexes_array = array();
  if (!isset($indexes_array[$nid])) {
    $indexes = array();
    $fields = tow_apachesolr_table_fields($nid);
    foreach ($fields as $field){
      if ($field['index'])
        $indexes[] = _tow_search_inner_reduce_field($field);
    }
    $indexes_array[$nid] = $indexes;
  }
  return $indexes_array[$nid];
}

function tow_search_inner_queue_table_linking($node, $op){
  $flag = FALSE;
  switch ($op) {
    case 'presave':
      if (!empty($node->nid))
        tow_search_inner_check_table_indexes($node);
      break;
    case 'update':
      if (isset($_SESSION['table_indexes_changed'][$node->nid])) {
        $flag = TRUE;
        unset($_SESSION['table_indexes_changed'][$node->nid]);
      }
      break;
    case 'insert':
    case 'delete';
        $flag = TRUE;
      break;
    default:
      break;
  }
  if ($flag) {
    db_query("INSERT IGNORE INTO {tow_link_tables} (record_nid, table_nid, dataset_nid, op, timestamp) 
              VALUES ('0', '%d', '%d', '%s', '%d')", $node->nid, $node->field_dataset[0]['value'], $op, time());
  }
}
function tow_search_inner_queue_record_linking($node, $op){mydpm('tow_search_inner_queue_record_linking');
  $table = node_load($node->field_table[0]['value']);
  switch ($op) {
    case 'insert':
    case 'update':
    case 'delete':
      db_query("INSERT IGNORE INTO {tow_link_tables} (record_nid, table_nid, dataset_nid, op, timestamp) 
          VALUES ('%d','%d', '%d', '%s', '%d')", $node->nid, $node->field_table[0]['value'], $table->field_dataset[0]['value'], $op, time());
      break;
    default:
      break;
  }
}

function tow_search_inner_check_table_indexes($changed_node) {mydpm('tow_search_inner_check_table_indexes ' . $changed_node->nid);
  $nid = $changed_node->nid;
  $orig_node = node_load($nid, NULL, TRUE);
  $flag= FALSE;
  foreach (tow_possible_fields() as $field) {
    $orig_fields = $orig_node->{'field_title_' . $field};
    $changed_fields = $changed_node->{'field_title_' . $field};
    if (count($orig_fields) == count($changed_fields)) {
      foreach($orig_fields as $key => $orig_field) {
        if ($orig_field['value'] != $changed_fields[$key]['value']) {
          $flag = TRUE;
          break;
        }
      }
      if ($flag)
        break;
    } else {
      $flag = TRUE;
      break;
    }
  }
  if ($flag) {
    if (!isset($_SESSION['table_indexes_changed'])) {
      $_SESSION['table_indexes_changed'] = array();
    }
    $_SESSION['table_indexes_changed'][$nid] = 1;
  } else {
    mydpm('Table ' . $nid . ' is not changed');
    if (isset($_SESSION['table_indexes_changed'])) {
      unset($_SESSION['table_indexes_changed'][$nid]);
    }
  }
}

function tow_search_inner_update_dataset_indexes($dataset_nid, $table_to_delete = NULL) {mydpm('tow_search_inner_update_dataset_indexes ' . $dataset_nid);
//  mydpm($node);
  $results = db_query("SELECT nid FROM {content_type_table} WHERE field_dataset_value='%d'", $dataset_nid);
  while ($nid = db_result($results)) {
    if ($nid != $table_to_delete)
      $tables[] = node_load($nid, NULL, TRUE);
  };
  
  if (empty($tables))
    return;
    
  foreach ($tables as $base_table) {
    mydpm('table '. $base_table->nid);
    $indexes = unserialize($base_table->field_indexes[0]['value']);
    $weights = unserialize($base_table->field_weights[0]['value']);
    $ii = 0;
    foreach (tow_possible_fields() as $type) {
      foreach ($base_table->{'field_title_' . $type} as &$base_field) {
        if ($base_field['value'] != NULL) {
          $flag = FALSE;
          foreach ($tables as $table) {
            if ($table->nid != $base_table->nid) {
              foreach ($table->{'field_title_' . $type} as $field) {
                if ($base_field['value'] == $field['value']) {
                  $flag = TRUE;
                  mydpm($base_field['value'] . ' is index in tables ' . $base_table->nid . ' and ' . $table->nid);
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
    db_query("UPDATE {content_type_table} SET `field_indexes_value` = '%s' 
              WHERE `nid`='%d'", serialize($indexes), $base_table->nid);
              
    db_query("DELETE FROM {cache_content} 
              WHERE `cid` =  'content:%d:%d'", $base_table->nid, $base_table->nid);
  }
  $node = array_shift($tables);
  if ($node) {
    mydpm('reset static cache');
    node_load($node->nid, NULL, TRUE); //reset static cache
  }
}

function mydpm($x){
//  dpm($x);
//  dd($x);
  watchdog('link_on_import', '<pre>' . print_r($x, TRUE) . '</pre>');
}

function tow_search_inner_linked_cron() {
  $time = time();
  // delete duplicates
  db_query("DELETE `t1` FROM {tow_link_tables} AS `t1`, {tow_link_tables} AS `t2` 
            WHERE `t1`.`record_nid`>`t2`.`record_nid` AND
             `t1`.`table_nid`=`t2`.`table_nid` AND
             `t1`.`timestamp`<'%d' AND `t2`.`timestamp`<'%d'", $time, $time);
  // delete tables with insert and update ops if there is the same table with delete op
  db_query("DELETE `t1` FROM {tow_link_tables} AS `t1` WHERE `t1`.`table_nid` IN 
            (SELECT * FROM
            (SELECT `t2`.`table_nid` FROM {tow_link_tables} AS `t2`
            WHERE `t2`.`record_nid`='0' AND `t2`.`op`='delete')
            AS `t3`)
            AND `t1`.`record_nid`='0'  AND `t1`.`op`<>'delete'
            AND `t1`.`timestamp` < '%d'", $time);
  // delete tables with insert op if there is the same table with update op
  db_query("DELETE `t1` FROM {tow_link_tables} AS `t1` WHERE `t1`.`table_nid` IN 
            (SELECT * FROM
            (SELECT `t2`.`table_nid` FROM {tow_link_tables} AS `t2`
            WHERE `t2`.`record_nid`='0' AND `t2`.`op`='update')
            AS `t3`)
            AND `t1`.`record_nid`='0'  AND `t1`.`op`='insert'
            AND `t1`.`timestamp` < '%d'", $time);
  // remaining records and tables
  $results = db_query("SELECT `record_nid`, `table_nid`, `dataset_nid`, `op`
            FROM {tow_link_tables} WHERE `timestamp` < '%d'", $time);
  $datasets = array();
  //TODO: nodes -> nids in handling functions
  while ($item = db_fetch_array($results)) {
    mydpm($item);
    if ($item['record_nid']) {
      if (!in_array($item['dataset_nid'], $datasets)) {
        tow_search_inner_update_dataset_indexes($item['dataset_nid']);
        $datasets[] = $item['dataset_nid'];
      }
      tow_search_inner_relink_table($item['table_nid']);      
    } else {
      // table
      switch ($item['op']) {
        case 'delete':
          tow_search_inner_unlink_table($item['table_nid']);
          tow_search_inner_update_dataset_indexes($item['dataset_nid'], $item['table_nid']);
          break;
        case 'insert':
        case 'update':
          if (!in_array($item['dataset_nid'], $datasets)) {
            tow_search_inner_update_dataset_indexes($item['dataset_nid']);
            $datasets[] = $item['dataset_nid'];
          }
          tow_search_inner_relink_table($item['table_nid']);
          break;
      }
    }
  }
  // delete processed items
  db_query("DELETE FROM {tow_link_tables} WHERE `timestamp` < '%d'", $time);
}

function tow_search_inner_list_of_tables_block() {

  $nid = tow_search_inner_get_nid();
  $is_dataset = db_result(db_query("SELECT COUNT(*) FROM {content_type_dataset} WHERE `nid`='%s'", $nid));
  if (!$is_dataset) {
    $table_nid = array_shift(explode('+', $nid));
    $dataset_nid = db_result(db_query("SELECT `field_dataset_value` 
                  FROM {content_type_table} WHERE `nid`='%s'", $table_nid));
    unset($table_nid);              
    if (!$dataset_nid) {
    //  drupal_not_found();
      return;
    }
  } else {
    $dataset_nid = $nid;
  }
  unset($nid);
  
  list($tables, $tablesets) = tow_search_inner_build_tablesets($dataset_nid);

  $titles = array();
  foreach ($tables as $table_nid) {
    $table_node = node_load($table_nid);
    $titles[$table_nid] = $table_node->title;
  }
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
      $linked_title[] = $titles[$nid] . (empty($links) ? '' : ' (' . implode('; ', $links). ')');
    }  
    $linked_titles[] = implode(', ', $linked_title);
  }
  $titles = array_merge($titles, $linked_titles);
  return array('title' => 'List of tables', 'content' => theme('item_list', $titles));
}