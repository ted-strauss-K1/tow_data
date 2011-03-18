<?php
// $Id: tow_record.inc,v 1.0.0.0  2011/02/28 15:32:21 tow Exp $

/**
 * @file
 */

/**
 * @param unknown_type $form
 * @param unknown_type $form_state
 */
function tow_record_form_alter(&$form, &$form_state) {
  // remove title, menu options, revision information, comment settings, publishing options
  unset($form['title']);
  unset($form['menu']);
  unset($form['revision_information']);
  unset($form['comment_settings']);
  //unset($form['author']); //TODO: hide this field but place user id like field_table?
  unset($form['options']);
    
  // hide native cck fields
  foreach (tow_possible_fields() as $field_type)
    $form['field_' . $field_type]['#access'] = FALSE;
    
  // get table node
  if ($_GET['table'])
    $table = node_load($_GET['table']);
  else
    $table = node_load($form['#node']->field_table[0]['nid']);
  if (!$table)
    return;
  
  $form['table_nid'] = array(
    '#type' => 'value',
    '#value' => $table->nid
  );
  
  $form['field_table']['#access'] = FALSE;
    
  // add tabular form containing cck fields ordered by weight
  $form['fields'] = array(
    '#theme' => 'tabular_form',
    '#title' => 'Fields',
    '#tree' => true,
  );
  
  $form['fields']['header'] = array(
    '#type' => 'value',
    '#value' => array('Title', 'Type', 'Value'),
  );
  
  $form['delta'] = array(
    '#tree' => TRUE
  );
  
  $weights_array = unserialize($table->field_weights[0]['value']);
  if ($weights_array) {   
    $index = 0;
    foreach (tow_possible_fields() as $field_type) {
      $delta = 0;
      if (!is_array($table->{'field_title_' . $field_type}))
        continue;
      foreach($table->{'field_title_' . $field_type} as $title_value) {
        if ($title_value['value']) {
          $aux_array[$weights_array[$index]]['title'] = $title_value['value'];
          $aux_array[$weights_array[$index]]['type'] = $field_type;
          $aux_array[$weights_array[$index]]['value'] = $form['field_' . $field_type][$delta]['#default_value']['value'];
          $aux_array[$weights_array[$index]]['delta'] = $delta;
          $index++;
          $delta++;
        }
      }
    }
    ksort($aux_array);
  
    foreach($aux_array as $key => $value) {
      $form['fields']['data'][$key]['title'] = array(
        '#type' => 'item',
        '#value' => $value['title'],
      );
      $form['fields']['data'][$key]['type'] = array(
        '#type' => 'item',
        '#value' => $value['type'],
      );
      
      switch ($value['type']) {//TODO: control size of inputs depending on type?
        case 'text':
          $input_type = 'textarea';
          break;
        case 'bool':
          $value['value'] = _tow_format_bool($value['value']);
          $input_type = 'textfield';
          break;
        case 'enum':
          $input_type = 'select';
          break;
        default :
          $input_type = 'textfield';
      }
           
      $form['fields']['data'][$key]['value'] = array(
        '#type' => $input_type,
        '#default_value' => $value['value'],
      );
      
      if ($value['type'] == 'enum') {
        $form['fields']['data'][$key]['value']['#options'] = array('-1' => '-----'); 
        $enum_options = unserialize($table->field_enum_options[$value['delta']]['value']); 
        if (is_array($enum_options))
          foreach($enum_options as $option)
            $form['fields']['data'][$key]['value']['#options'][] = $option;
      }
            
      $form['delta'][$key] = array(
        '#type' => 'value',
        '#value' => $value['delta']
      );
      
      if (!$form['field_' . $value['type']][$value['delta']]) {
        $form['field_' . $value['type']][$value['delta']] = $form['field_' . $value['type']][$value['delta'] -1];
        $form['field_' . $value['type']][$value['delta']]['_weight']['#delta'] = $value['delta'];
        $form['field_' . $value['type']][$value['delta']]['#delta'] = $value['delta'];
        $form['field_' . $value['type']][$value['delta']]['#weight'] = $value['delta'];
      }
      
    }
  }
  
  $form['#redirect'] = 'table/' . $table->nid; 
  
  // register submit callbacks
  $form['#submit'][] = 'tow_record_submit_table';
  $form['#submit'][] = 'tow_record_submit_values';
}

/**
 * Submit callback.
 * Set nid of table.
 */
function tow_record_submit_table($form, &$form_state) {
  $form_state['values']['field_table'][0]['nid'] = $form_state['values']['table_nid'];
}

/**
 * Submit callback.
 * Set corresponding cck fields (e.g. $form_state['values'][]field_int) field values to be equal to values entered by user 
 */
function tow_record_submit_values($form, &$form_state) {
  if (!is_array($form_state['values']['fields']['data']))
    return;
  foreach($form_state['values']['fields']['data'] as $key => $value) {
    $type = $form['fields']['data'][$key]['type']['#value'];
    if ($type == 'bool')
      $value['value'] = _tow_bool_to_int($value['value']);
    $delta = $form_state['values']['delta'][$key];
    $form_state['values']['field_' . $type][$delta]['_weight'] = $delta;
    $form_state['values']['field_' . $type][$delta]['value'] = $value['value'];
  }
}