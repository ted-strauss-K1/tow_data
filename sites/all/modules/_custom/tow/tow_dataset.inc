<?php
// $Id: tow_dataset.inc,v 1.0.0.0  2011/02/28 15:32:21 tow Exp $

/**
 * @file
 */

/**
 * Create table block
 */
function tow_dataset_create_table_block() {
  global $node;
  if ($node->type !== 'dataset')
    return;
  $block['content'] = drupal_get_form('tow_dataset_create_table_form');
  return $block;
}

/**
 * Create table form
 */
function tow_dataset_create_table_form() {
  
  $form['create_table'] = array(
    '#type' => 'fieldset',
    '#title' => 'Create a new table',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  
  $form['create_table']['table_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Name',
  );
  
  $form['create_table']['count_fields'] = array(
    '#type' => 'textfield',
    '#title' => 'Number of fields',
    '#size' => 2
  );
  
  $form['create_table']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go')
  );
  
  return $form;
}

/**
 * Submit callback.
 * Store values in $_SESSION
 * Redirect to node/table/add 
 */
function tow_dataset_create_table_form_submit($form, &$form_state) {
  $args = arg();
  $_SESSION['table_definition'] = array(
    'dataset' => $args[1],
    'table_name' => $form_state['values']['table_name'],
    'count_fields' => $form_state['values']['count_fields']
  );
  drupal_goto('node/add/table');
}