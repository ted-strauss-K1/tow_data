<?php

define('TOW_CRON_HASH_SLEEP', 0);

/**
 * Constructs a form for search-within-dataset on search_dataset/% page.
 * Called by drupal_get_form function, assigned to the specified link (see hook_menu).
*/
function tow_search_form(&$form_state, $dataset_nid) {
//  x('tow_search_form called');
  
  $form = array();
  // Add form elements
  tow_search_within_dataset_altering_form($form, $form_state, $dataset_nid);
  return $form;
}

/** 
 * Alters apachesolr search form on search/apachesorl_search page. Called from hook_form_alter.
 * Two varians possible depending on URL parameters: one for search-for-datasets, second for
 * search-within-datasets (after submitting simple search).
 */
function tow_search_apachesolr_search_form(&$form, &$form_state) {
  // x($form); 
  $filters = $_GET['filters'];
 
  // Calls search_within_dataset_altering_form function in case if redirecton to the apachesolr search page 
  // was made by simple search_within_dataset 
  if (strpos($filters, 'type:record') !== FALSE) {
  
    $matches_qty = preg_match('/(?<=dataset_nid:)\d{1,}$/', $filters, $dataset_nids);
    if ($matches_qty) {
      $dataset_nid = $dataset_nids[0];
      tow_search_within_dataset_altering_form($form, $form_state, $dataset_nid);
      return $form;
    }
    else {
      // TODO: GOTO 404 page 
    }
  }
  else {
    //tow_search_datasets_altering_form($form);
  }
  return $form;
}

function tow_search_cron() {
  
   $res = db_query("SELECT nid FROM {node} WHERE type='dataset'");
  while ($node = db_fetch_object($res)) {//watchdog('search',$node->nid);
    sleep(TOW_CRON_HASH_SLEEP);
    $res1=db_query("CALL dataset_hash_update(%d)", $node->nid);//watchdog('search',($res1==FALSE?'Failed':'Success'));
    $res1=db_query("DELETE FROM {cache_content} WHERE cid = 'content:%s:%s'", $node->nid, $node->nid);//watchdog('search',($res1==FALSE?'Failed':'Success'));
  }
  //watchdog('search','Datasets\' hash is updated');
   
}