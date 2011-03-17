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
  
  $form['create_table']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go')
  );
  
  return $form;
}

/**
 * Submit callback
 */
function tow_dataset_create_table_form_submit($form, &$form_state) {
  $node = new stdClass;
  if ($form_state['values']['table_name']) {
    $node->type = 'table';
    global $user;
    $node->uid = $user->uid;
    $node->status = 1;
    $node->title = $form_state['values']['table_name'];
    $dataset_nid = arg(1);
    $node->field_dataset = array(array('nid' => $dataset_nid));
    node_save($node);
  }
}