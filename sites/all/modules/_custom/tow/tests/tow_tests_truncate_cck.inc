<?php

function tow_tests_truncate_cck() {
  return drupal_get_form('tow_tests_truncate_cck_form');  
}

function tow_tests_truncate_cck_form(){
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

function tow_tests_truncate_cck_form_submit($form, &$form_state){
  
  $tables = array();
  foreach(tow_possible_fields() as $field_type) {
    $tables[] = 'content_field_' . $field_type;
    $tables[] = 'content_field_title_' . $field_type;
  }
  $tables[] = 'content_field_enum_options';
  foreach ($tables as $table) {
    db_query('TRUNCATE TABLE {%s}', $table);
    $output .= "<p> TRUNCATE TABLE " . $table . "</p>"; 
  }
  
  db_query('UPDATE {content_type_table} SET field_weights_value = NULL');
  $output .= "<p>UPDATE content_type_table SET field_weights_value = NULL</p>";
  
  drupal_flush_all_caches();
  
  drupal_set_message($output);
  drupal_set_message('Cache cleared.');
  
  drupal_goto('<front>');
}