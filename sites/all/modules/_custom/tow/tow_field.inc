<?php
// $Id: tow_field.inc,v 1.0.0.0 2011/02/28 23:42:15 tow Exp $
/**
 * @file
 * 
 */

/**
 * @return confirmation form for field deletion
 */
function tow_field_delete_confirm($form_state, $table_nid, $type, $delta, $index) {

  global $user;
  global $table;
  global $dataset;
  
  $table = node_load($table_nid);
  if ($user->uid != $node->uid && !user_access('edit any table content')) {
    drupal_access_denied();
    die();
  }
  if (!$table->{'field_title_' . $type}[$delta]['value']) {
    drupal_not_found();
    die();
  }
  $dataset = node_load($table->field_dataset[0]['value']);
     
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $type
  );
  $form['delta'] = array(
    '#type' => 'value',
    '#value' => $delta
  );
  $form['field_index'] = array(
    '#type' => 'value',
    '#value' => $index
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete field %field?',
      array('%field' => $table->{'field_title_' . $type}[$delta]['value'])
    ),
    'node/'. $table_nid,
    t('Deletion of field may cause loss of data'),
    t('Confirm'),
    t('Cancel')
  ); 

}

/**
 * Submission callback
 */
function tow_field_delete_confirm_submit($form, &$form_state) {
  
  global $table;

  $type = $form_state['values']['type'];
  $delta = $form_state['values']['delta'];
  $field_index = $form_state['values']['field_index'];
  
  db_query("
    DELETE FROM {content_field_%s} WHERE nid IN
    (
      SELECT nid FROM content_type_record WHERE field_table_value = %d
    )
  AND delta = %d
 ", $type, $table->nid, $delta);
  
  unset($table->{'field_title_' . $type}[$delta]);
  if ($type = 'enum')
    unset($table->field_enum_options[$delta]);

  $weights = unserialize($table->field_weights[0]['value']);
  $current_weight = $weights[$field_index];  
  unset($weights[$field_index]);
  foreach ($weights as $i => $weight)
    if($weight > $current_weight)
      $weights[$i] --;
  $table->field_weights[0]['value'] = serialize(array_values($weights));
  
  // modify indexes array
  $indexes = unserialize($table->field_indexes[0]['value']);
  unset($indexes[$field_index]);
  $table->field_indexes[0]['value'] = serialize($indexes);
  
  //modify unit values
  unset($table->field_units[$field_index]);
    
  node_save($table);
    
  drupal_set_message('The field has been deleted');
  $form_state['redirect'] = 'node/' . $table->nid;
}

/**
 * Interface for field editing
 */
function tow_field_edit_form($form_state, $table_nid, $type, $delta, $index) {
  
  global $user;
  global $table;
  global $dataset;
  
  $table = node_load($table_nid);
  if ($user->uid != $node->uid && !user_access('edit any table content')) {
    drupal_access_denied();
    die();
  }
  if (!$table->{'field_title_' . $type}[$delta]['value']) {
    drupal_not_found();
    die();
  }
  $dataset = node_load($table->field_dataset[0]['value']);

  $form['current_type'] = array(
    '#type' => 'value',
    '#value' => $type
  );  
  $form['delta'] = array(
    '#type' => 'value',
    '#value' => $delta
  );  
  $form['field_index'] = array(
    '#type' => 'value',
    '#value' => $index
  );
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Field title'),
    '#default_value' => $table->{'field_title_' . $type}[$delta]['value'],
  );
  
  $possible_fields = tow_possible_fields();
  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => $possible_fields,
    '#default_value' => array_search($type, $possible_fields)
  );
  
   $form['unit'] = array(
    '#type' => 'textfield',
    '#title' => t('Unit'),
    '#default_value' => _tow_remove_fake($table->field_units[$index]['value']),
  );
  
  // Read field description from database  
  $description_res = db_query('SELECT `description` FROM {tow_field_info} WHERE `nid` = %d AND `index` = %d', $table_nid, $index);
  $description = '';
  while ($res = db_fetch_array($description_res))
     $description = $res['description'];
  $form['current_description'] = array(
    '#type' => 'value',
    '#value' => $description
  );
  
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $description
  );
  
  $index_array = unserialize($table->field_indexes[0]['value']);
  $form['index'] = array(
    '#type' => 'checkbox',
    '#title' => t('Index'),
    '#default_value' => $index_array[$index]
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  
  $form['#redirect'] = 'node/' . $table_nid;
  
  return $form;
}

/**
 * Submission callback
 */
function tow_field_edit_form_submit($form, &$form_state) {
  global $table;
  
  $table_nid = $table->nid;
  $field_type = $form_state['values']['current_type'];
  $delta = $form_state['values']['delta'];

  // Update index array of the table
  $index_array = unserialize($table->field_indexes[0]['value']);
  $index_array[$form_state['values']['field_index']] = $form_state['values']['index'];
  $new_indexes = serialize($index_array);
  if ($new_indexes != $table->field_indexes[0]['value']) {
    $node_save = TRUE;
    $table->field_indexes[0]['value'] = $new_indexes;
  }
  
  // Update unit values of the table
  $new_units_array = $table->field_units;
  $new_units_array[$form_state['values']['field_index']]['value'] = _tow_insert_fake($form_state['values']['unit']);
  if ($new_units_array != $table->field_units) {
    $node_save = TRUE;
    $table->field_units = $new_units_array;
  } 
  
  if ($table->{'field_title_' . $field_type}[$delta]['value'] != $form_state['values']['title']) {
    $table->{'field_title_' . $field_type}[$delta]['value'] = $form_state['values']['title'];
    $node_save = TRUE;
  }
  
  if ($node_save)
    node_save($table);
  
  $is_new = ($form_state['values']['current_description'] === '');
  if ($form_state['values']['current_description'] !== trim($form_state['values']['description']))
    _tow_field_description_save($table_nid, 
                                $form_state['values']['field_index'], 
                                trim($form_state['values']['description']), 
                                $is_new);
}

/**
 *  Shows field description
 */
function tow_field_description_callback($field_index, $table_nid) {
  
// CHANGED
  $dataset_nid_res = db_query("SELECT `field_dataset_value` FROM {content_type_table} WHERE `nid` = %d", $table_nid);
  $dataset_nid = db_fetch_array($dataset_nid_res);
  $dataset_nid = $dataset_nid['field_dataset_value'];
  
  $dataset = node_load($dataset_nid);
// END OF CHANGED  
  $description_res = db_query('SELECT `description` FROM {tow_field_info} WHERE `nid` = %d AND `index` = %d', $table_nid, $field_index);
  
  $description = '';
  while ($res = db_fetch_array($description_res)){
     $description = $res['description'];
  } 
  //$output = "<div>" . $table_nid . $field_index ."</div>";
  
  if ($description !== '') {
    $output = "<div>" . $description . "</div>";
  }
  else {
    $output = "<div>" . t("The owner hasn't provided any description for this field yet.") . "</div>";
  }
  
  return $output;
}
/**
 *  Submits field descriptions into the database after table node was created
 */
function tow_field_descriptions_submit($table_nid) {
  global $descriptions;
  if (is_array($descriptions))
    foreach ($descriptions as $index => $description)
      _tow_field_description_save($table_nid, $index, $description);
  unset($descriptions);
}
/**
 *  Saves field description in the database
 */

function _tow_field_description_save($table_nid, $index, $description, $is_new = TRUE) {
  if (!$is_new) {
    if ($description !== '') {
      db_query("UPDATE {tow_field_info} 
      			SET `description` = '%s' 
      			WHERE `nid` = '%d' 
      			AND `index` = '%d';", $description, $table_nid, $index);
    }
    else {
      db_query("DELETE FROM {tow_field_info}
      			WHERE `nid` =  '%d'  
      			AND `index` = '%d';", $table_nid, $index);
    }
  }
  else {
    if ($description !== '')
      db_query("INSERT INTO {tow_field_info} (`nid`, `index`, `description`) VALUES ('%d', '%d', '%s');", 
        $table_nid, $index, $description);
  } 
}
/**
 *	Checks whether unit string is empty and chages it by '-' if so
 */
function _tow_insert_fake($unit) {
	return ($unit=='')?'-':$unit;
}

/**
 *	Checks whether unit string is '-' and chages it by empty string if so
 */
function _tow_remove_fake($unit) {
	return ($unit=='-')?'':$unit;
}
