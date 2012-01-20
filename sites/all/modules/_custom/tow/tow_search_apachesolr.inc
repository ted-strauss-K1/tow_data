<?php

/**
 * Implementation of hook__apachesolr_process_results()
 * @param: $results - non-empty array of search results
 */
function tow_apachesolr_process_results(&$results) {
  //x('tow_apachesolr_process_results');
  //x($results);
  
  if ($results[0]['node']->type == 'dataset') { // if search was made for datasets
    foreach ($results as $key => $result) {

		$results[$key]['amount_tables'] = $result['node']->sis_tow_amount_tables . t(' tables');
		$results[$key]['amount_records'] = $result['node']->sis_tow_amount_records . t(' records');
		$results[$key]['posted_date'] = t('posted ') . date('M d, Y', $result['node']->created);

		$account = new stdClass;
		$account->uid = $result['node']->uid;
		$account->name = $result['node']->name;
		$account->picture = $result['node']->ss_picture;
		$results[$key]['picture'] = theme('user_picture', $account);

		$results[$key]['teaser'] = $result['node']->ss_teaser;

		$tid = $result['fields']['im_vid_2']['0'];
		$results[$key]['category'] = l(taxonomy_get_term($tid)->name, 'taxonomy/term/' . $tid);
	  
		foreach ($result['fields']['im_vid_1'] as $tid) {
			$results[$key]['tags'][] = l(taxonomy_get_term($tid)->name, 'taxonomy/term/' . $tid);
		}
		//$results[$key]['amount_views'] = $result['node']->sis_tow_amount_views . t(' views');
    }
  }
  else { // search for records option
    foreach ($results as $key => $result) {
      // add record_nid to the display data, first index of this array corresponds 
      // to the table_nid of added record
      $displayed_rows[$result['node']->sis_tow_field_table][] = $result['node']->nid;
      unset($results[$key]);
      
    }  
    //x($displayed_rows);
    //  Display each table
    foreach ($displayed_rows as $table_nid => $record_nids) {
    
      // TODO: Specify, which records to extract
      $first_record_id = 0;
      $display_options['sort'] = array ('sort_index' => 'nid', 'sort_order' => 'ASC');
      $display_options['record_nids'] = $record_nids;
      
      //$display_options['limits'] = array ('first_elem_no' => $first_record_id, 'total_elements' => 1);

      $table_node = node_load($table_nid);
        
      $tables_to_display[] = array(
        'table_nid' => $result['node'],
        'title' => l($table_node->title, 'table/' . $table_nid) . t(' table'),
        'table' => tow_table($table_node, $display_options),
      ); 
           
      
    }
    
    $results = $tables_to_display;
  }
  
  
  //print 'Search results';
}

function tow_apachesolr_modify_query(&$query, &$params) { 
  //x(tow_apachesolr_modify_query);
  //x($query);
  //x($params);
  // Adding fields to field list (fl)
  
  // TODO: Add this only for datasets
  $params['fl'] .= ',sis_tow_amount_tables';
  $params['fl'] .= ',sis_tow_amount_records';
  $params['fl'] .= ',sis_tow_amount_saved_searches';
  $params['fl'] .= ',sis_tow_count_views';
  $params['fl'] .= ',sis_cck_field_access_type';
  $params['fl'] .= ',im_vid_1';
  $params['fl'] .= ',im_vid_2';
  $params['fl'] .= ',ss_teaser';
  // TODO:bookmarks 

  // TODO: add this only to records
  $params['fl'] .= ',sis_tow_field_table';

  $query->add_filter('type', 'dataset');  
  $query->set_solrsort('created', 'desc');
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
	  
	  $document->sis_tow_amount_saved_searches = db_result(db_query("SELECT COUNT(*)
                                                                   FROM {content_type_saved_search} `ss`
                                                                   WHERE field_dataset_nid_value = %d",
                                                                   $node->nid));
	  $document->ss_picture = $node->picture;
	  $alter=array(
			'html' => TRUE,
			'ellipsis' => TRUE,
			'word_boundary' => TRUE,
			'max_length' => 255,
	  ); 

	  $document->ss_teaser = views_trim_text($alter,$node->body);
	  
  }
	
	// Add dataset_nid field and table_nid field to be indexed by apachesolr
	if ($node->type == 'record') {
    $document->sis_tow_dataset_nid = db_result(db_query("
      SELECT `ctt`.`field_dataset_value` AS `dataset_nid`
      FROM {content_type_table} `ctt`
      LEFT JOIN {content_type_record} `ctr`
        ON `ctr`.`field_table_value` = `ctt`.`nid`
      WHERE `ctr`.`nid` = %d", $node->nid));   
    $document->sis_tow_field_table = $node->field_table[0]['value'];
    
  }
  
}

function tow_apachesolr_facets() {
  $facets['sis_tow_amount_tables'] = array(
    'info' => t('Amount of tables'),
    'facet_field' => 'sis_tow_amount_tables',
  );
  return $facets;
}

/*
*	
*Implementation of hook_apachesolr_prepare_query
*
*/
function tow_apachesolr_prepare_query(&$query, &$params, $caller) {

}