<?php

/**
 * @file
 * 
 * Truncate linked records testing.
 */



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Page callback.
 */
function tow_tests_truncate_linked_records() {
  return drupal_get_form('tow_tests_truncate_linked_records_form');  
}

/**
 * Form.
 */
function tow_tests_truncate_linked_records_form() {

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
function tow_tests_truncate_linked_records_form_submit($form, &$form_state) {
  
  db_query('DELETE FROM {node} WHERE nid IN (SELECT nid FROM {content_type_linked_record})');
  drupal_set_message('Nodes of linked records are deleted.');

  // Tables.
  $tables = array(
    'content_type_linked_record',
    'content_field_record_nid',
    'content_field_tables',
  );
  
  // Dataset tags are not deleted.
  foreach ($tables as $table) {
    db_query('TRUNCATE TABLE {%s}', $table);
    $output .= "<p> TRUNCATE TABLE " . $table . "</p>"; 
  }
  
  drupal_set_message(filter_xss($output));
}