<?php

/**
 * @file
 * 
 * Truncate nodes testing.
 */



/****************************************************************************************************
 * Requirements
 ****************************************************************************************************/

module_load_include('inc', 'tow', 'includes/tests/tow.tests.truncate_cck');
module_load_include('inc', 'tow', 'includes/tests/tow.tests.truncate_linked_records');



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Page callback.
 */
function tow_tests_truncate_nodes() {
  return drupal_get_form('tow_tests_truncate_nodes_form');  
}

/**
 * Form.
 */
function tow_tests_truncate_nodes_form() {
  
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

/**
 * Submission callback.
 */
function tow_tests_truncate_nodes_form_submit($form, &$form_state) {
  
  // Tables.
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
  
  // Dataset tags are not deleted.
  foreach ($tables as $table) {
    db_query('TRUNCATE TABLE {%s}', $table);
    $output .= "<p> TRUNCATE TABLE " . $table . "</p>"; 
  }
  
  drupal_set_message(filter_xss($output));
  
  // Delete terms.
  $vid_res = db_query("

    SELECT 
      v.vid 

      FROM 
        {vocabulary} v 

      WHERE 
        v.name = '%s'

  ", 'Dataset tags');
  $vid_tag = db_fetch_array($vid_res);
  $vid_tag = $vid_tag['vid']; 
  db_query("DELETE FROM {term_data} WHERE vid = %d", $vid_tag);
  
  drupal_set_message('Terms from Dataset tags vocabulary are deleted.');
  
  // Truncate CCK tables and linked records.
  $array = array();
  tow_tests_truncate_linked_records_form_submit(array(), $array);
  tow_tests_truncate_cck_form_submit(array(), $array);
}