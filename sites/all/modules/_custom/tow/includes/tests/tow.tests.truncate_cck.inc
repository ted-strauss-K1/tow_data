<?php

/**
 * @file
 * 
 * Truncate CCK tables testing.
 */



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Page callback.
 */
function tow_tests_truncate_cck() {
  return drupal_get_form('tow_tests_truncate_cck_form');  
}

/**
 * Form.
 */
function tow_tests_truncate_cck_form() {
  
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
function tow_tests_truncate_cck_form_submit($form, &$form_state) {
  
  $tables = array();
  
  // Get tables for testing.
  foreach (tow_possible_fields() as $field_type) {
    $tables[] = 'content_field_' . $field_type;
    $tables[] = 'content_field_title_' . $field_type;
  }
  $tables[] = 'content_field_enum_options';
  //$tables[] = 'content_field_displayed_tables';
  //$tables[] = 'content_field_scanned_tables';
  //$tables[] = 'content_field_units';
 
  // Truncate each table.
  foreach ($tables as $table) {
    db_query('TRUNCATE TABLE {%s}', $table);
    $output .= "<p> TRUNCATE TABLE " . $table . "</p>"; 
  }
  
  // NULLate weights.
  db_query('UPDATE {content_type_table} SET field_weights_value = NULL');
  $output .= "<p>UPDATE content_type_table SET field_weights_value = NULL</p>";
  
  // Clear caches.
  drupal_flush_all_caches();
  
  // Messaging.
  drupal_set_message(filter_xss($output));
  drupal_set_message('Caches cleared.');
  
  // Redirect to front page.
  drupal_goto('<front>');
}