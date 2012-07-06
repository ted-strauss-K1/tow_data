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
    //$res1=db_query("CALL dataset_hash_update(%d)", $node->nid);//watchdog('search',($res1==FALSE?'Failed':'Success'));
    tow_search_dataset_hash_update($node->nid);//watchdog('search',($res1==FALSE?'Failed':'Success'));
    $res1=db_query("DELETE FROM {cache_content} WHERE cid = 'content:%s:%s'", $node->nid, $node->nid);//watchdog('search',($res1==FALSE?'Failed':'Success'));
  }
  //watchdog('search','Datasets\' hash is updated');
   
}

function tow_search_dataset_hash_update($nid) {
  
  $hash = '';
  $params = array(
    array('table' => 'node_revisions', 'field' => 'body', 'alias' => 'table_descriptions'),
    array('table' => 'tow_field_info', 'field' => 'description', 'alias' => 'field_descriptions'),
    array('table' => 'node', 'field' => 'title', 'alias' => 'table_names'),
    array('table' => 'content_field_title_int', 'field' => 'field_title_int_value', 'alias' => 'int_fields'),
    array('table' => 'content_field_title_char', 'field' => 'field_title_char_value', 'alias' => 'char_fields'),
    array('table' => 'content_field_title_float', 'field' => 'field_title_float_value', 'alias' => 'float_fields'),
    array('table' => 'content_field_title_bool', 'field' => 'field_title_bool_value', 'alias' => 'bool_fields'),
    array('table' => 'content_field_title_text', 'field' => 'field_title_text_value', 'alias' => 'text_fields'),
    array('table' => 'content_field_title_enum', 'field' => 'field_title_enum_value', 'alias' => 'enum_fields'),
    array('table' => 'content_field_title_date', 'field' => 'field_title_date_value', 'alias' => 'date_fields'),
    array('table' => 'content_field_title_datetime', 'field' => 'field_title_datetime_value', 'alias' => 'datetime_fields'),
    array('table' => 'content_field_title_time', 'field' => 'field_title_time_value', 'alias' => 'time_fields'),
    array('table' => 'content_field_title_timestamp', 'field' => 'field_title_timestamp_value', 'alias' => 'timestamp_fields'),
    array('table' => 'content_field_title_code', 'field' => 'field_title_code_value', 'alias' => 'code_fields'),
  );
  
  foreach ($params as $p) {
    $query = "
      SELECT 
        t1.%s AS %s

        FROM 
            {%s} t1
          INNER JOIN 
            {content_type_table} t2 
            ON 
              t1.nid = t2.nid 

        WHERE 
          t2.field_dataset_value = %d
    ";
    $query = db_query($query, $p['field'], $p['alias'], $p['table'], $nid);
    while ($result = db_fetch_object($query)) {
      $hash .= $result->{$p['alias']} . ' ';
    }
  }
  $hash = trim($hash);

  $query = "

    UPDATE {content_type_dataset} 
  
      SET 
        field_hash_value = '%s' 

      WHERE 
        nid = %d
  
  ";
  db_query($query, $hash, $nid);

}