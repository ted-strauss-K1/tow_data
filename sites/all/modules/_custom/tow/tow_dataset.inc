<?php
// $Id: tow_dataset.inc,v 1.0.0.0  2011/02/28 15:32:21 tow Exp $

/**
 * @file
 */

/**
 * Prepare global variable with dataset statistics
 */
function tow_dataset_extract_statistics($node) {  
  global $dataset_statistics;
  $dataset_statistics = db_fetch_array(db_query("SELECT count, timestamp FROM {tow_dataset_counter} WHERE nid = %d LIMIT 1", $node->nid));
}

/**
 * If current user is not an owner of dataset node, store data about page view in tow_dataset_counter table
 */
function tow_dataset_update_statistics($node) {
  // dataset owner should not affect statistics of his fatasets
  global $user;
  if ($user->uid == $node->uid) {
    return;
  }
    
  $time = time();
  db_query("
    INSERT INTO {tow_dataset_counter} (nid, count, timestamp)
    VALUES (%d, 1, %d)
    ON DUPLICATE KEY UPDATE count = count+1, timestamp = %d
    ", $node->nid, $time, $time);
}

/**
 * Create table block
 */
function tow_dataset_create_table_block() {
  global $dataset;
  if (!$dataset)
    return;
  
  global $user;
  if ($user->uid === $dataset->uid) {
    $args = arg();
    if ($args[0] == 'dataset' && !isset($args[2]))
      $block['content'] = drupal_get_form('tow_dataset_create_table_form');
  }
  return $block;
}

/**
 * Create table form
 */
function tow_dataset_create_table_form() {

  $form['create_table'] = array(
    '#type' => 'fieldset',
    '#title' => 'Create a new table',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['create_table']['table_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Name',
  );

  $form['create_table']['count_fields'] = array(
    '#type' => 'textfield',
    '#title' => 'Number of fields',
    '#size' => 2
  );

  $form['create_table']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go')
  );

  return $form;
}

/**
 * Submit callback.
 * Store values in $_SESSION
 * Redirect to node/table/add
 */
function tow_dataset_create_table_form_submit($form, &$form_state) {
  $args = arg();
  $_SESSION['table_definition'] = array(
    'dataset' => $args[1],
    'table_name' => $form_state['values']['table_name'],
    'count_fields' => $form_state['values']['count_fields']
  );
  drupal_goto('node/add/table', 'destination=dataset/' . $args[1]);
}

/**
 * Extracts and prepares info for dataset summary page
 * @param $node dataset node
 */
function tow_dataset_summary(&$node) {

  global $user;
  // for administer view only
  if ($user->uid == $node->uid) {
    $node->summary['edit_description'] = l('edit description', 'node/' . $node->nid . '/edit');
    $node->summary['edit_categories'] = l('edit categories', 'node/' . $node->nid . '/edit');
    $node->summary['admin_tasks'] = _tow_dataset_admin_tasks($node);
  }
  
  // Extract and show names of tables from the dataset
  $table_titles_res = db_query("SELECT nid, title FROM {node} WHERE nid IN
      (SELECT nid FROM {content_type_table} WHERE field_dataset_nid = %d)", $node->nid);

  $tables = array();

  while ($rs = db_fetch_array($table_titles_res)) {
    $tables[] = array ('title' => $rs['title'], 'nid' => $rs['nid'], 'link' => "table/" . $rs['nid']);
    $tables_list[] = l($rs['title'], 'table/' . $rs['nid']);
  }
  if (!empty($tables_list))
    $node->summary['tables'] = implode(', ', $tables_list);

  // get nids and titles of two random tables that contain records
  $res = db_query("
    SELECT n.nid, title, count FROM {node} n 
    LEFT JOIN
    (
	  SELECT COUNT(*) AS count, field_table_nid FROM {content_type_record}
	  GROUP BY field_table_nid
     ) t
    ON n.nid = t.field_table_nid
    WHERE n.nid IN
    (
	  SELECT nid FROM {content_type_table}
	  WHERE field_dataset_nid = %d
    )
    AND field_table_nid > 0
    ORDER BY RAND()
    LIMIT 0, 2
  ", $node->nid);

  while ($table = db_fetch_object($res)) {
    $first_record_id = rand(0, $table->count - 3);
    $display_options['sort'] = array ('sort_index' => 'nid', 'sort_order' => 'ASC');
    $display_options['limits'] = array ('first_elem_no' => $first_record_id, 'total_elements' => 3);
    
    $node->summary['view_table'][] = array(
      'title' => l($table->title, 'table/' . $table->nid) . t(' table'),
      'table' => tow_table(node_load($table->nid), $display_options)
    );
  }

  tow_dataset_prepare_badge_block($node, count($tables));
  
}

/**
 * @return array of administrator links
 */
function _tow_dataset_admin_tasks($node) {
  
  $count = _tow_dataset_count_all($node->uid);
  
  if ($node->status)
    $maintenance = t('Put this data set into ') . l('maintenance mode', 'dataset/' . $node->nid . '/maintenance');
  else
    $maintenance = t('Now this data set is in maintenance mode. ' . l('Publish it', 'dataset/' . $node->nid . '/maintenance')); 
    
  $admin_tasks = array(
    'create' => l('Create', 'node/add/table', array('query' => 'dataset=' . $node->nid)) . t(' or ') . l('import', 'import') . t(' a new table'),
    'manage' => l('Manage tables',  'dataset/' . $node->nid),
    'current_status' => t('Currently a normal user (free), with ') . $count['tables'] . t(' tables, ') . $count['records'] . t(' rows.'),
    'maintenence' => $maintenance,
    'delete' => l('Delete this dataset forever', 'node/' . $node->nid . '/delete'),
  );
  
  return theme_item_list($admin_tasks);
}

/**
 * Count amount of tables and records the user owns.
 */
function _tow_dataset_count_all($uid) {
  $res = db_query("
      SELECT COUNT(*) AS c FROM {node} WHERE uid = %d AND type = 'table'
    UNION ALL
      SELECT COUNT(*)AS c FROM {node} WHERE uid = %d AND type = 'record'
    ", $uid, $uid);
  
  $data[] = db_fetch_array($res);
  $data[] = db_fetch_array($res);  
  
  $count = array(
    'tables' => $data[0]['c'],
    'records' => $data[1]['c'],
  );
  
  return $count;    
}

/**
 * Prepare two global variables for dataset badge block
 */
function tow_dataset_prepare_badge_block($node, $count_tables) {

  global $dataset_count_tables;
  $dataset_count_tables = $count_tables;

  global $dataset_count_records;
  $dataset_count_records = db_result(db_query("
    SELECT SUM(t.count) FROM {node} n 
    LEFT JOIN
    (
	  SELECT COUNT(*) AS count, field_table_nid FROM {content_type_record}
	  GROUP BY field_table_nid
     ) t
    ON n.nid = t.field_table_nid
    WHERE n.nid IN
    (
	  SELECT nid FROM {content_type_table}
	  WHERE field_dataset_nid = %d
    )
  ", $node->nid));

}

/**
 * @return "Dataset badge" block
 */
function tow_dataset_badge_block() {
  
  global $dataset;
  if (!dataset) {
    return;
  }
  
  global $dataset_count_tables;
  if (!isset($dataset_count_tables)) {
    return;
  }
    
  global $dataset_count_records;
  if (!isset($dataset_count_records))
    $dataset_count_records = 0;
  
  
  $posted_by = t('Posted by ') . l($dataset->name, 'user/' . $dataset->uid);
  $count = $dataset_count_tables . t(' tables') . ' ' . $dataset_count_records . t(' rows');

  $added = t('added ') . ago($dataset->created);
  $updated = t('updated ') . ago($dataset->changed);

  $block['content'] = theme('tow_theme_dataset_badge', $posted_by, $count, NULL, $added, $updated, NULL);

  $account = new stdClass;
  $account->uid = $dataset->uid;
  $account->name = $dataset->name;
  $account->picture = $dataset->picture;

  $block['content'] .= theme('user_picture', $account);

  return $block;
}

/**
 * @return "Dataset stats" block
 */
function tow_dataset_stats_block() {
  global $dataset;
  if (!$dataset) {
    return;
  }
  
  global $dataset_count_tables;
  if (!isset($dataset_count_tables)) {
    return;
  }
  
  $block['title'] = t('Stats');
  
  global $dataset_statistics;
  if (!$dataset_statistics) {
    $dataset_statistics = array(
      'count' => 0,
      'timestamp' => t('never')
    );
  }else{
    $dataset_statistics['timestamp'] = ago($dataset_statistics['timestamp']);
  }   
  
  $block['content'] = theme('table', array(), array(
    array(
      t('Views'), $dataset_statistics['count']), 
    array(
      t('Last view'), $dataset_statistics['timestamp'])
  ));

  return $block;
}

/**
 * @return date in "intervals ago" format. (intervals are minutes, hours, days etc.)
 * @param $timestamp int unix time
 */
function ago($timestamp){
   $difference = time() - $timestamp;
   $periods = array("second", "minute", "hour", "day", "week", "month", "year", "decade");
   $lengths = array("60","60","24","7","4.35","12","10");
   for($j = 0; $difference >= $lengths[$j]; $j++)
     $difference /= $lengths[$j];
   $difference = round($difference);
   if($difference != 1) $periods[$j].= "s";
     $text = "$difference $periods[$j] ago";
   return $text;
}

/**
 * @return maintenance confirmation form
 */
function tow_dataset_maintenance_confirm(&$form_state, $nid) {
  global $dataset;
  $dataset = node_load($nid);
  
  if ($dataset->type != 'dataset') {
    drupal_not_found();
    die();
  }
  
  $form['dataset'] = array(
    '#type' => 'value',
    '#value' => $dataset,
  );
  
  if ($dataset->status)
    $message = 'Are you sure you want to put %title to maintenance mode?';
  else
    $message = 'Now %title is in maintenance mode. Do you want to publish it?';
  
  return confirm_form($form,
    t($message, array('%title' => $dataset->title)),
    'node/'. $dataset->nid,
    '',
    t('Yes'),
    t('Cancel')
  );
}

/**
 * Submission callback for maintenance confirmation form
 * Published dataset becomes unpublished and vice-versa
 */
function tow_dataset_maintenance_confirm_submit($form, &$form_state) {
  $dataset = $form_state['values']['dataset']; 
  if ($dataset->status)
    $dataset->status = 0;
  else
    $dataset->status = 1;
  node_save($dataset);
  $form_state['redirect'] = 'node/' . $dataset->nid;  
}