<?php
// $Id: tow_table_filter.inc,v 1.0.0.0  2011/02/28 15:32:21 tow Exp $

/**
 * @file
 * User interface for table filtering
 */

/**
 * Implementation of hook_form_ID_alter().
 * Include array of fields to select list.
 */
function tow_form_tow_table_filter_alter(&$form, &$form_state) {
  $form['table_filter']['field']['#options'] = tow_table_filter_fields();
}

/**
 * Build form for table filtering.
 */
function tow_table_filter() {
  $form = array();
  
  $form['table_filter'] = array(
    '#type' => 'fieldset',
    '#title' => 'Filter table rows',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,  
  );
  
  $form['table_filter']['field'] = array(
    '#type' => 'select',
    '#options' => array(),
    '#description' => t('Select field to filter.'),
    '#required' => TRUE,
  );
  
  $form['table_filter']['operator'] = array(
    '#type' => 'select',
    '#options' => tow_table_filter_operators(), 
    '#description' => t('Select operator to apply.'),
    '#required' => TRUE,
  );
  
  $form['table_filter']['value'] = array(
    '#type' => 'textfield',
    '#size' => 15,
    '#maxlength' => 20,
    '#description' => t('Enter here value for filtering.'),
    '#required' => TRUE,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Apply'),
  );
  
  return $form;
}

/**
 * return array of fields to filter
 */
function tow_table_filter_fields() {
  global $table;
  $index = 0;
  $weights_array = unserialize($table->field_weights[0]['value']);
  if (!$weights_array)
    return array();
  foreach (tow_possible_fields() as $field_type)
    if (is_array($table->{'field_title_' . $field_type}))
      foreach($table->{'field_title_' . $field_type} as $title_value)
        if ($title_value['value']) {
          $aux_array[$weights_array[$index]] = $title_value['value'];
          $index++;
        }
  if (!empty($aux_array))
    ksort($aux_array);
  return $aux_array;
}

/**
 * Return array of filter operators
 */
function tow_table_filter_operators() {
  return array('Is equal to', 'Is less than', 'Is greater than');
}

/**
 * Return corresponding sql operator. See function above.
 */
function tow_table_filter_translate_operator($op_key) {
  $sql_operators = array('=', '<', '>');
  return $sql_operators[$op_key];
}

/**
 * Submit callback
 */
function tow_table_filter_submit($form, &$form_state) {
  global $table;
  if (is_numeric($_GET['order']) && $_GET['order'] >= 0 && ($_GET['sort'] == 'asc' || $_GET['sort'] == 'desc'))
    $query = 'order=' . $_GET['order'] . '&sort=' . $_GET['sort'] . '&'; 
  $query .= 'filter=' . $form_state['values']['field'] . '&op=' . $form_state['values']['operator'] . '&value=' . $form_state['values']['value'];
  drupal_goto('table/' . $table->nid, $query);
}