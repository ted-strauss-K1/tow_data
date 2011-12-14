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
 */
function tow_table_validate_common($form, &$form_state) {
  
  if ($form_state['clicked_button']['#value'] == 'Cancel')
    return;
  
  $error = FALSE;  
  $err_field_names = array();
  $err_msgs = array();
  
  $field_types = tow_possible_fields();

   ## Unique table title
  $table_titles_res = db_query("SELECT title FROM {node} WHERE nid IN 
  	( SELECT nid FROM {content_type_table} WHERE field_dataset_value = %d) AND nid <> %d", 
    $form_state['values']['field_dataset'][0]['value'], $form_state['values']['nid']);

  $table_titles = array();
  while ($rs = db_fetch_array($table_titles_res)) {
    $table_titles[] = strtolower($rs['title']);
  }
  $form_state['values']['title'] = trim($form_state['values']['title']);
  if (in_array(strtolower($form_state['values']['title']), $table_titles)) {
    $error = TRUE;
    $err_field_names[] = "title";
    $err_msgs[] = t('The table with this title already exists.');
  } 
  
 foreach($form_state['values']['fields']['data'] as $key => $value) {
    // Trim field values 
    $form_state['values']['fields']['data'][$key]['title'] = trim($value['title']); 
    $column_names[] = strtolower($form_state['values']['fields']['data'][$key]['title']);
    ## Empty fields check
    if ($form_state['values']['fields']['data'][$key]['title'] == '') {
      $error = TRUE;  
      $err_field_names[] = "fields][data][$key][title";
      $err_msgs[] = t('Field titles must neither be empty nor consist of nothing but whitespaces.');
    }
  ## enum options quantity check
    if (($field_types[$form_state['values']['fields']['data'][$key]['type']] == 'enum') &&
        trim($form_state['values']['fields']['data'][$key]['enum_options']) == '') {
      $error = TRUE;  
      $err_field_names[] = "fields][data][$key][enum_options";
      $err_msgs[] = t('You must specify at least one option for enum field.');
    }
  }  
    
## Unique field names check
  if (count($column_names) != count(array_unique($column_names))){
    $error = TRUE;
    // Count the frequency of field names. At least one of them be will greater than 1.
    $column_names_qty = array_count_values($column_names);
    // Remove not repeated elements from $column_names_qty array 
    $column_names_not_repeated = array_keys($column_names_qty, 1);
    foreach ($column_names_not_repeated as $column_name) {
      unset($column_names_qty[$column_name]);
    }
  
    foreach ($column_names_qty as $column_name => $qty) {
      // extract numbers of fields with repeating titles
      $repeated_field_no = array_keys($column_names, $column_name);
      // set error for them
      foreach ($repeated_field_no as $no) {
        $err_field_names[] = "fields][data][$no][title";
        $err_msgs[] = t('Field titles must have unique values.');
      }
    } 
  }
  ## Showing error messages 
  _tow_show_validation_error($error, $err_field_names, $err_msgs);
}
/**
 * Performs extra form validation during table editing from node/table_id/edit/ page connected with the possibility of 
 * adding fields.
 *
 * @param $form, &$form_state
 */
function tow_table_validate_edit(&$form, &$form_state) {
  
  $error = FALSE;  
  $err_field_names = array();
  $err_msgs = array();
  
  $field_types = tow_possible_fields();
 
  foreach($form_state['values']['fields']['data'] as $key => $value) {
    $form_state['values']['fields']['data'][$key]['title'] = trim($value['title']); 
    $column_names[] = strtolower($form_state['values']['fields']['data'][$key]['title']);
  } 
  
  ## Unique new column name check
  $no = array_search(strtolower($form_state['values']['field_add']['title']), $column_names);
  
  if (!($no === FALSE) && ($column_names[$no] != '')) {
    $error = TRUE;
    $err_field_names[] = "field_add][title";
    $err_msgs[] = t('New field must have unique title.'); 
    $err_field_names[] = "fields][data][$no][title";
    $err_msgs[] = t('New field must have unique title.');
  } 
  
  ## Correct number of enum options for new field check
  $field_types = tow_possible_fields();
  
  if (($field_types[$form_state['values']['field_add']['type']] == 'enum') &&
  trim($form_state['values']['field_add']['enum_options']) == '') {
    $error = TRUE;
    $err_field_names[] = "field_add][enum_options";
    $err_msgs[] = t('You must specify at least one option for enum field.');
  }
  ## Showing error messages 
  _tow_show_validation_error($error, $err_field_names, $err_msgs);
}

/**
 * Performs form validation during adding field from node/table_id/ page.
 *
 * @param $form, &$form_state
 */
function tow_table_field_add_form_validate(&$form, &$form_state) {
  
  $error = FALSE;  
  $err_field_names = array();
  $err_msgs = array();
  
  $field_types = tow_possible_fields();
  
## Empty fields check
  if ($form_state['values']['field_add']['title'] == '') {
    $error = TRUE;  
    $err_field_names[] = "field_add][title";
    $err_msgs[] = t('New field title must neither be empty nor consist of nothing but whitespaces.');
  }
  else {
  
  ## Unique new column name check
    $column_names = $form_state['values']['existing_fields'];
    $no = array_search(strtolower($form_state['values']['field_add']['title']), $column_names);
  
    if (!($no === FALSE)) {
      $error = TRUE;
      $err_field_names[] = "field_add][title";
      $err_msgs[] = t('New field must have unique title.'); 
    } 
  }
  ## Correct number of enum options for new field check
  $field_types = tow_possible_fields();
  
  if (($field_types[$form_state['values']['field_add']['type']] == 'enum') &&
  trim($form_state['values']['field_add']['enum_options']) == '') {
    $error = TRUE;
    $err_field_names[] = "field_add][enum_options";
    $err_msgs[] = t('You must specify at least one option for enum field.');
  }
  ## Showing error messages 
  _tow_show_validation_error($error, $err_field_names, $err_msgs);
}

/**
 * Performs form validation during record editing/creation.
 *
 * @param $form, &$form_state
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
   	  case 'time':
        $value = trim($value_array['value']);
        if (!preg_match("/^-?(([0-7]?[0-9]?[0-9])|([8][0-2][0-9])|([8][3][8])):([0-5]?[0-9])(:([0-5]?[0-9]))?$/", $value)) {
          $error = TRUE;  
          $err_field_names[] = "fields][data][$key][value";
          $err_msgs[] = t('Time must be entered in "HHH:MM:SS" or "HHH:MM" format.');
        }
        break;
      case 'code':
        $value = $value_array['value'];
        if (strlen($value) > 3) { 
          $error = TRUE;  
          $err_field_names[] = "fields][data][$key][value";
          $err_msgs[] = t('Code field must not contain more than 3 symbols.');
        } 
        break;
    }
  }
  ## Showing error messages 
  _tow_show_validation_error($error, $err_field_names, $err_msgs);
}

/**
 * Performs form validation during table adding on the dataset screen.
 *
 * @param $form, &$form_state
 */
function tow_dataset_create_table_form_validate($form, &$form_state) {
  $error = FALSE;  
  $err_field_names = array();
  $err_msgs = array();
  global $dataset;
  $form_state['values']['table_name'] = trim($form_state['values']['table_name']);

  ## Empty table title
  if ($form_state['values']['table_name'] == '') {
     $error = TRUE;
     $err_field_names[] = "table_name";
     $err_msgs[] = t('Table title must neither be empty nor consist of nothing but whitespaces.');
  }
  
  ## Unique table title
   $table_titles_res = db_query("SELECT title FROM {node} WHERE nid IN 
  	(SELECT nid FROM {content_type_table} WHERE field_dataset_value = %d)", $dataset->nid);
  $table_titles = array();
  while ($rs = db_fetch_array($table_titles_res)) {
    $table_titles[] = strtolower($rs['title']);
  }  
  
  if (in_array(strtolower($form_state['values']['table_name']), $table_titles)) {
     $error = TRUE;
     $err_field_names[] = "table_name";
     $err_msgs[] = t('The table with this title already exists.');
  }  

## Max and min fields check
  if (($form_state['values']['count_fields'] > MAX_FIELDS_IN_TABLE)||
      ($form_state['values']['count_fields'] < MIN_FIELDS_IN_TABLE)) {
     $error = TRUE;
     $err_field_names[] = "count_fields";
     $err_msgs[] = t("The number of fields must be greater than " . MIN_FIELDS_IN_TABLE . " and less than " . MAX_FIELDS_IN_TABLE. ".");
  }  
  ## Showing error messages 
  _tow_show_validation_error($error, $err_field_names, $err_msgs);
}

/**
 * Sets validation error messages if any.
 *
 * @param $error, $err_field_names, $err_msgs
 */
function _tow_show_validation_error($error, $err_field_names, $err_msgs) {
  if ($error) {
    foreach ($err_field_names as $err_no => $err_field_name) {
      form_set_error($err_field_name, $err_msgs[$err_no]);
    }
    // Exclude repeating messages
    $_SESSION['messages']['error'] = array_unique($_SESSION['messages']['error']);
  }
}

/**
 * Validate user registration form
 */
function tow_user_register_validate($form, &$form_state) {

  if ($form_state['clicked_button']['#value'] != t('Create new account'))
    return;
  
  if ($form_state['values']['email_verify'] != $form_state['values']['mail']) {
    $error = TRUE;
    $err_field_names[] = 'email_verify';
    $err_msgs[] = t('E-mail address and E-mail verify should be same');
  }
  
  if (!$form_state['values']['terms_of_service']) {
    $error = TRUE;
    $err_field_names[] = 'terms_of_service';
    $err_msgs[] = t('You should agree on terms of service and privacy terms');
  }
  _tow_show_validation_error($error, $err_field_names, $err_msgs);
}
/**
 * Validate form submit when performing advanced search 
 */
function tow_search_advanced_search_validate($form, &$form_state) {
  
}
