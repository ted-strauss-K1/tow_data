<?php

function tow_search_outer_filter_block() {
  
  $block['title'] = NULL;
  $block['content'] = drupal_get_form('tow_search_outer_filter_form');
  
  return $block;
}

function tow_search_outer_filter_taxonomy() {
  
  $terms = array(t('Any'));
  
  $res = db_query("SELECT tid, name FROM {term_data} WHERE vid = 2 ORDER BY name ASC");
  while ($term = db_fetch_array($res))
    $terms[$term['tid']] = $term['name'];
  return $terms;

}

/** 
 * Alters apachesolr search form. 
 * Two varians possible depending on URL parameters: one for search-for-datasets, second for
 * search-within-datasets (after submitting simple search).
 */
function tow_search_apachesolr_search_form(&$form, &$form_state) {
  // x($form); 
  $filters = $_GET['filters'];
 // x($form);
 // x($form_state);
//	x($filters);
	
	//x($form);
	
  // Calls search_within_dataset_altering_form function in case if redirecton to the apachesolr search page 
  // was made by simple search_within_dataset 
  if (strpos($filters, 'type:record') !== FALSE) {
	
	$matches_qty = preg_match('/(?<=dataset_nid:)\d{1,}$/', $filters, $dataset_nids);
	if ($matches_qty) {
		$dataset_nid = $dataset_nids[0];
		tow_search_within_dataset_altering_form(&$form, &$form_state, $dataset_nid);
		return $form;
	}
	
	else {
	// TODO: GOTO 404 page 
	}
  }
  else {
 
	  $form['subjects'] = array(
		'#type' => 'select',
		'#options' => tow_search_outer_filter_taxonomy(),
		'#title' => t('Subjects')
	  );
	  $form['subjects']['#default_value'] = _tow_search_outer_default_subject(array_keys($form['subjects']['#options']), $filters); 

	  $form['size'] = array(
		'#type' => 'select',
		'#options' => array(
		  t('Any'),
		  t('More than 100 rows'),
		  t('More than 500 rows'),
		  t('More than 1000 rows'),
		  t('More than 5000 rows')
		),
		'#title' => t('Size of data set')
	  );
	  
	  $form['saved_searches'] = array(
		'#type' => 'select',
		'#options' => array(
		  t('Any'),
		  t('At least 1'),
		  t('At least 5'),
		  t('At least 10')
		),
		'#title' => t('Saved searches')
	  );
	  
	  $form['post_date'] = array(
		'#type' => 'select',
		'#options' => array(
		  t('Any'),
		  t('One day ago'),
		  t('One week ago'),
		  t('One month ago'),
		  t('6 months ago')
		),
		'#title' => t('Post date (up to)')
	  );
	  
	  $form['access_status'] = array(
		'#type' => 'select',
		'#options' => array(
		  t('Any'),
		  t('Open'),
		  t('By request'),
		  t('Private')
		),
		'#title' => t('Access status')
	  );
	  
	  $form['popularity'] = array(
		'#type' => 'select',
		'#options' => array(
		  t('Any'),
		  t('Low'),
		  t('Moderate'),
		  t('High')
		),
		'#title' => t('popularity (# views)')
	  );
  
		$form['#submit'] = array('tow_search_outer_filter_form_submit');
	// x($_GET); 
	}
	
	return $form;
	
}
/**
 * Composes an URL upon submission of search-datasets form
 * URL is understandable by apachesolr and contains all the search parameters
 */
function tow_search_outer_filter_form_submit($form, &$form_state) {
  // x('tow_search_outer_filter_form_submit called');
  // x($form_state);
  $fv = $form_state['values'];
  
  $path = 'search/apachesolr_search/' . $fv['processed_keys'];
  $query = 'filters=';
  
  $filters = array();
  
  // search in datasets only
  $query .= 'type:dataset';
  
  // subjects
  // each consequent search option should be separated from previous by whitespace
  if ($form_state['values']['subjects']) {
    $query .= ' ' . 'tid:' . $form_state['values']['subjects'];
  }
    
  
  
  if ($query == 'filters=')
    $query = NULL;
  //x($path);
  //x($query);
  drupal_goto($path, $query);
}

/**
 * Implementation of hook__apachesolr_process_results()
 */
function tow_apachesolr_process_results(&$results) {
  x('tow_apachesolr_process_results');
  //x($results);
  foreach ($results as $key => $result) {
    $results[$key]['amount_tables'] = $result['node']->sis_tow_amount_tables . t(' tables');
    $results[$key]['amount_records'] = $result['node']->sis_tow_amount_records . t(' records');
    //$results[$key]['amount_views'] = $result['node']->sis_tow_amount_views . t(' views');
  }
  //print 'Search results';
}

function tow_apachesolr_modify_query(&$query, &$params) { 
  //x(tow_apachesolr_modify_query);
 // x($query);
 // x($params);
  $params['fl'] .= ',sis_tow_amount_tables';
  $params['fl'] .= ',sis_tow_amount_records';
  #$params['fl'] .= ',sis_tow_amount_saved_searches';
  $params['fl'] .= ',sis_tow_count_views';
  $params['fl'] .= ',sis_cck_field_access_type';
  // TODO:bookmarks 
}

/** 
 * Implementation of hook_apachesolr_update_index(). 
 **/ 
function tow_apachesolr_update_index(&$document, $node) {
  
  if ($node->type == 'dataset') {
	  $document->sis_tow_amount_tables = db_result(db_query("
		SELECT COUNT(*) FROM {content_type_table} WHERE field_dataset_value = %d
	  ", $node->nid));
	  
	  $document->sis_tow_amount_records = db_result(db_query("
		SELECT COUNT(*) AS qty
		FROM {content_type_record} r 
		INNER JOIN {content_type_table} t
		  ON r.field_table_value = t.nid
		  AND t.field_dataset_value = %d",
		$node->nid));
	  
	  // TODO: amount of saved searches
	  #$document->sis_tow_amount_saved_searches =
		
	  $document->sis_tow_count_views = db_result(db_query("
		SELECT count FROM {tow_dataset_counter} WHERE nid = %d
	  ", $node->nid));
	  
	  // TODO:bookmarks
  }
	
	// Add dataset_nid field to be indexed by apachesolr
	if ($node->type == 'record') {
    $document->sis_tow_dataset_nid = db_result(db_query("
      SELECT `ctt`.`field_dataset_value` AS `dataset_nid`
      FROM {content_type_table} `ctt`
      LEFT JOIN {content_type_record} `ctr`
        ON `ctr`.`field_table_value` = `ctt`.`nid`
      WHERE `ctr`.`nid` = %d", $node->nid));   
	}
  
}

function _tow_search_outer_default_subject($tids, $filters) {
  
  if ($filters) {
    $start = strpos($filters, 'tid:');
    if ($start !== FALSE) {
      $end = strpos($filters, ' ', $start + 4);
      
      if ($end === FALSE) {
        $tid = substr($filters, $start + 4);
      }else{
        $tid = substr($filters, $start + 4, $end - $start - 4);
      }
    }
    if (in_array($tid, $tids)) {
      return $tid;
    }
  }
  
  return 0;  

}
