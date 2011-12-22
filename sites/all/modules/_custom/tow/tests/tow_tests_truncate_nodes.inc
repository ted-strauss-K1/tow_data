<?php

include_once('./' . drupal_get_path('module', 'tow') . '/tests/tow_tests_truncate_cck.inc');

function tow_tests_truncate_nodes() {
  return drupal_get_form('tow_tests_truncate_nodes_form');  
}

function tow_tests_truncate_nodes_form(){
  $form['desc'] = array(
    '#type' => 'item',
    '#value' => 'Confirm?'
  );
  $form['truncate'] = array(
    '#type' => 'submit',
    '#value' => 'Yes',
  );
  return $form;
}

function tow_tests_truncate_nodes_form_submit($form, &$form_state){
  
  $tables = array(
    'content_type_table',
    'content_type_record',
    'content_type_dataset',
    'node',
    'node_comment_statistics',
    'node_counter',
    'node_revisions',
    'term_node',
    'tow_dataset_counter',
    'tow_access_requests',
    'tow_field_info',
	'tow_invites',
	'tow_table_counter',
	'content_type_saved_search',
  );
//dataset tags are not  deleted
  foreach ($tables as $table) {
    db_query('TRUNCATE TABLE {%s}', $table);
    $output .= "<p> TRUNCATE TABLE " . $table . "</p>"; 
  }
  
  drupal_set_message($output);
  
  $array = array();
  tow_tests_truncate_cck_form_submit(array(), $array);
  
}