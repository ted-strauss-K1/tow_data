<?php 
// $Id: tow_table_truncate.inc,v 1.0.0.0  2011/02/28 15:32:21 tow Exp $

/**
 * @file
 */

/**
 * Build confirmation form for table truncating.
 * @param unknown_type $form_state
 * @param $nid of table to be truncated
 */
function tow_table_truncate_confirm(&$form_state, $nid) {
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $nid,
  );
  
  $node = node_load($nid);
  
  $form['#dataset'] = $node->field_dataset[0]['nid'];
  
  return confirm_form($form,
    t('Are you sure you want to truncate %title?', array('%title' => $node->title)),
    'table/'. $node->nid,
    t('This action cannot be undone.'),
    t('Truncate'),
    t('Cancel')
  );
  
}

/**
 * Submit callback. Delete all records of table. 
 */
function tow_table_truncate_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $res = db_query("SELECT nid FROM {content_type_record} WHERE field_table_nid = %d", $form['nid']['#value']);
      while($node_to_delete = db_fetch_object($res))
        node_delete($node_to_delete->nid);
  }
  $form_state['redirect'] = 'node/' . $form['#dataset'];
}