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
  $index_array = unserialize($table->field_indexes[0]['value']);
  $index_array[$form_state['values']['field_index']] = $form_state['values']['index'];
  $table->field_indexes[0]['value'] = serialize($index_array);
  node_save($table);
}