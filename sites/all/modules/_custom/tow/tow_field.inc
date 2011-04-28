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
  $dataset = node_load($table->field_dataset[0]['nid']);
     
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
      SELECT nid FROM content_type_record WHERE field_table_nid = %d
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
  
  $indexes = unserialize($table->field_indexes[0]['value']);
  unset($indexes[$field_index]);
  $table->field_indexes[0]['value'] = serialize($indexes);
  
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
  $dataset = node_load($table->field_dataset[0]['nid']);

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
  
  // Update index array of the table
  $index_array = unserialize($table->field_indexes[0]['value']);
  $index_array[$form_state['values']['field_index']] = $form_state['values']['index'];
  $table->field_indexes[0]['value'] = serialize($index_array);
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
  $dataset_nid_res = db_query("SELECT `field_dataset_nid` FROM {content_type_table} WHERE `nid` = %d", $table_nid);
  $dataset_nid = db_fetch_array($dataset_nid_res);
  $dataset_nid = $dataset_nid['field_dataset_nid'];
  
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