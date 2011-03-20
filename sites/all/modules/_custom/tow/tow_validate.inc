<?php
// $Id: tow.validate.inc,v 1.0 .0.0 2011/02/28 15:32:21 tow Exp $

/**
 * @file
 * Validate callbacks for different forms.
 */

/**
 * Performs form validation during table creation or editing.
 *
 * @param $form, &$form_state
 * 
 */
function tow_table_validate_common(&$form, &$form_state) {

  $error = FALSE;  
  $err_field_names = array();
  $err_msgs = array();
  
  $field_types = tow_possible_fields();

   ## Unique table title
  $table_titles_res = db_query("SELECT title FROM {node} WHERE type = 'table' AND nid IN 
  	( SELECT nid FROM {content_type_table} WHERE field_dataset_nid = %d) AND nid <> %d", 
    $form_state['values']['field_dataset'][0]['nid'], $form_state['values']['nid']);

  $table_titles = array();
  while ($rs = db_fetch_array($table_titles_res)) {
    $table_titles[] = $rs['title'];
  }

  if (in_array($form_state['values']['title'], $table_titles)) {
    $error = TRUE;
    $err_field_names[] = "title";
    $err_msgs[] = t('The table with this title already exists.');
  } 
  
 foreach($form_state['values']['fields']['data'] as $key => $value) {
    // Trim field values 
    $form_state['values']['fields']['data'][$key]['title'] = trim($value['title']); 
## Empty fields check
    if ($form_state['values']['fields']['data'][$key]['title'] == '') {
      $error = TRUE;  
      $err_field_names[] = "fields][data][$key][title";
      $err_msgs[] = t('Column titles must neither be empty nor consist of nothing but whitespaces.');
    }
  ## enum options quantity check
    if (($field_types[$form_state['values']['fields']['data'][$key]['type']] == 'enum') &&
        trim($form_state['values']['fields']['data'][$key]['enum_options']) == '') {
      $error = TRUE;  
      $err_field_names[] = "fields][data][$key][enum_options";
      $err_msgs[] = t('You must specify at least one option for enum field.');
    }
    $column_names[] = $value['title'];  
  }  
    
## Unique column names check
  if (count($column_names) != count(array_unique($column_names))){
    $error = TRUE;
    // Count the frequency of column names. At least one of them be will greater than 1.
    $column_names_qty = array_count_values($column_names);
    // Remove not repeated elements from $column_names_qty array 
    $column_names_not_repeated = array_keys($column_names_qty, 1);

    foreach ($column_names_not_repeated as $column_name) {
      unset($column_names_qty[$column_name]);
    }
    foreach ($column_names_qty as $column_name => $qty) {
      $repeated_field_no = array_keys($column_names, $column_name);
      foreach ($repeated_field_no as $no) {
        $err_field_names[] = "fields][data][$no][title";
        $err_msgs[] = t('Column titles must have unique values.');
      }
    } 
  }
  
## Setting error messages check  
  if ($error) {
    foreach ($err_field_names as $err_no => $err_field_name) {
      form_set_error($err_field_name, $err_msgs[$err_no]);
    }
    // Exclude repeating messages
    $_SESSION['messages']['error'] = array_unique($_SESSION['messages']['error']);
  }
}
/**
 * Performs extra form validation during table editing connected with the possibility of 
 * adding columns.
 *
 * @param $form, &$form_state
 * 
 */
function tow_table_validate_edit(&$form, &$form_state) {
  
  $error = FALSE;  
  $err_field_names = array();
  $err_msgs = array();
  
  $field_types = tow_possible_fields();
 
  foreach($form_state['values']['fields']['data'] as $key => $value) {
    $column_names[] = $value['title'];  
  } 
  
  ## Unique new column name check
  $no = array_search($form_state['values']['field_add']['title'], $column_names);
  if ($no) {
    $error = TRUE;
    $err_field_names[] = "field_add][title";
    $err_msgs[] = t('New column must have unique title.'); 
    $err_field_names[] = "fields][data][$no][title";
    $err_msgs[] = t('New column must have unique title.');
  } 

  ## Correct number of enum options for new field check
  $field_types = tow_possible_fields();
  
  if (($field_types[$form_state['values']['field_add']['type']] == 'enum') &&
  trim($form_state['values']['field_add']['enum_options']) == '') {
    $error = TRUE;
    $err_field_names[] = "field_add][enum_options";
    $err_msgs[] = t('You must specify at least one option for enum field.');
  }
  
## Setting error messages check  
  if ($error) {
    foreach ($err_field_names as $err_no => $err_field_name) {
      form_set_error($err_field_name, $err_msgs[$err_no]);
    }
    // Exclude repeating messages
    $_SESSION['messages']['error'] = array_unique($_SESSION['messages']['error']);
  }
}
/**
 * Performs form validation during record editing/creation.
 *
 * @param $form, &$form_state
 * 
 */
function tow_record_validate_edit(&$form, &$form_state) {

  $error = FALSE;  
  $err_field_names = array();
  $err_msgs = array();
   
 foreach($form_state['values']['fields']['data'] as $key => $value_array) {
    $type = $form['fields']['data'][$key]['type']['#value'];
    
    switch ($type) {
      case 'int': 
        $value = trim($value_array['value']);
        $form_state['values']['fields']['data'][$key]['value'] = $value;
        if (($value != '') && (!is_numeric($value) || !((int)$value == $value))) { 
           $error = TRUE;  
           $err_field_names[] = "fields][data][$key][value";
           $err_msgs[] = t('Specified value is not integer.');
        }       
        break;
      case 'float':
        $value = trim($value_array['value']);
        $form_state['values']['fields']['data'][$key]['value'] = $value; 
        if (($value != '') && (!is_numeric($value))) { 
           $error = TRUE;  
           $err_field_names[] = "fields][data][$key][value";
           $err_msgs[] = t('Specified value is not float.');
        }       
        break;
      case 'char':
        break;
      case 'text':
        break;
      case 'enum':
        break;
      case 'bool':
        break;
    }
  }

## Setting error messages check  
  if ($error)
  {
    foreach ($err_field_names as $err_no => $err_field_name) {
      form_set_error($err_field_name, $err_msgs[$err_no]);
    }
    // Exclude repeating messages
    $_SESSION['messages']['error'] = array_unique($_SESSION['messages']['error']);
      
  }
}
/**
 * Performs form validation during table adding on the dataset screen.
 *
 * @param $form, &$form_state
 * 
 */
function tow_dataset_create_table_form_validate($form, &$form_state) {
  $error = FALSE;  
  $err_field_names = array();
  $err_msgs = array();
  global $node;

  ## Unique table title
  $table_titles_res = db_query("SELECT title FROM {node} WHERE type = 'table' AND nid IN 
  	(SELECT nid FROM {content_type_table} WHERE field_dataset_nid = %d)", $node->nid);
  $table_titles = array();
  while ($rs = db_fetch_array($table_titles_res)) {
    $table_titles[] = $rs['title'];
  }  
  
  if (in_array($form_state['values']['table_name'], $table_titles)) {
     $error = TRUE;
     $err_field_names[] = "table_name";
     $err_msgs[] = t('The table with this title already exists.');
  }  

## Max and min fields check
  if (($form_state['values']['count_fields'] > MAX_FIELDS_IN_TABLE)||
      ($form_state['values']['count_fields'] < MIN_FIELDS_IN_TABLE)) {
     $error = TRUE;
     $err_field_names[] = "count_fields";
     $err_msgs[] = t("The number of fields must be greater than $MIN_FIELDS_IN_TABLE and less than $MAX_FIELDS_IN_TABLE.");
  }  
  
  ## Setting error messages check  
  if ($error) {
    foreach ($err_field_names as $err_no => $err_field_name) {
      form_set_error($err_field_name, $err_msgs[$err_no]);
    }
    // Exclude repeating messages
    $_SESSION['messages']['error'] = array_unique($_SESSION['messages']['error']);
  }
}