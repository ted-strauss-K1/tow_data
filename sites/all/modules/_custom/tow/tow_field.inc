<?php
// $Id: tow_field.inc,v 1.0.0.0 2011/02/28 23:42:15 tow Exp $
/**
 * @file
 * 
 */

/**
 * @return confirmation form for field deletion
 */
function tow_field_delete_confirm($form_state, $table_nid, $type, $delta) {

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
    
  $form['table_nid'] = array(
    '#type' => 'value',
    '#value' => $table_nid
  );
  
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $type
  );
  
  $form['delta'] = array(
    '#type' => 'value',
    '#value' => $delta
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
  
  $nid = $form_state['values']['table_nid'];
  $type = $form_state['values']['type'];
  $delta = $form_state['values']['delta'];
  
  db_query("
    DELETE FROM {content_field_%s} WHERE nid IN
    (
      SELECT nid FROM content_type_record WHERE field_table_nid = %d
    )
  AND delta = %d
 ", $type, $nid, $delta);
  
  $node = node_load($nid);
  unset($node->{'field_title_' . $type}[$delta]);
  if ($type = 'enum')
    unset($node->field_enum_options[$delta]);
  node_save($node);
    
  drupal_set_message('The field has been deleted');
  $form_state['redirect'] = 'node/' . $form_state['values']['table_nid'];
}

/**
 * Interface for field editing
 */
function tow_field_edit_form($form_state, $table_nid, $type, $delta) {
  
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
    
  $form['table_nid'] = array(
    '#type' => 'value',
    '#value' => $table_nid
  );
  
  $form['current_type'] = array(
    '#type' => 'value',
    '#value' => $type
  );
  
  $form['delta'] = array(
    '#type' => 'value',
    '#value' => $delta
  );
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#default_value' => $table->{'field_title_' . $type}[$delta]['value'],
  );
  
  $possible_fields = tow_possible_fields();
  $form['type'] = array(
    '#type' => 'select',
    '#options' => $possible_fields,
    '#default_value' => array_search($type, $possible_fields)
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  
  return $form;
}

/**
 * Submission callback
 */
function tow_field_edit_form_submit($form, $form_state) {
  
}