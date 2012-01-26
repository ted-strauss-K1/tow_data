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
  
  $facets['sis_tow_amount_records'] = array(
    'info' => t('Amount of rows'),
    'facet_field' => 'sis_tow_amount_records',
  );
  
  $facets['sis_tow_amount_saved_searches'] = array(
    'info' => t('Amount of saved searches'),
    'facet_field' => 'sis_tow_amount_saved_searches',
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

/**
 *  Helper function for retrieving number of results for facetted-like menus.
 *	Uses analyse of single solr request.
 *	Arguments:
 *	$facet_field -- facet field
 *	$values -- array of options of drop-down menu in form $key => $value, where $key is value of filter. 
 *		Array should has accending sorting on keys
 *  $is_range -- if TRUE,  search is done through range [value TO *], strict value is searched otherwise
 */

function tow_facet_menu($facet_field, $values, $is_range = FALSE) {
	$saved_response = apachesolr_static_response_cache();
	$saved_query = apachesolr_current_query();
	$query = clone $saved_query;
	
	$do_new_search = TRUE;xdebug_break();
	if ($facet_field == 'im_vid_2') {
		foreach (tow_search_outer_filter_taxonomy() as $tid => $term) {
			if ($query->has_filter('tid', $tid)) {
				$query->remove_filter('tid', $tid);
				$do_new_search = TRUE;
			}
		}
	} else {
		foreach($query->get_filters() as $set_filter) {
			if ($set_filter['#name'] == $facet_field) {
				$query->remove_filter($facet_field);
				$do_new_search = TRUE;
			}
		}
	}
		
	if ($do_new_search) {
		$params = apachesolr_search_basic_params($query);
		apachesolr_search_add_facet_params($params, $query);
		apachesolr_search_add_boost_params($params, $query, apachesolr_get_solr());
		list($final_query, $response) = apachesolr_do_query('apachesolr_search', $query, $params);
		if (empty($response)) {
		  return;
		}

		apachesolr_static_response_cache($saved_response);
		apachesolr_current_query($saved_query);	
		
	} else {	
		$response = clone $saved_response;
	}
	
	if ($is_range) {
		$sorted_keys = array_flip($values);
			
		$facet_counts = (array) $response->facet_counts->facet_fields->$facet_field;
		if (!empty($facet_counts)) {
			$current_key = array_pop($sorted_keys);
			
			krsort($facet_counts);
			
			$counts = 0;

			foreach ($facet_counts as $facet_value => $facet_count) {
				while ($current_key > $facet_value) {
					$values[$current_key] .= ' (' . $counts . ')';
					$current_key = array_pop($sorted_keys);
				}
				$counts += $facet_count;
			}
			
			while (!empty($sorted_keys)) {
				$values[$current_key] .= ' (' . $counts . ')';
				$current_key = array_pop($sorted_keys);
			}

			$values[$current_key] .= ' (' . $counts . ')';
			
		} else { // no results
			foreach ($values as $key => $value) {
				$values[$key] .= ' (0)';
			}		
		}
	} else {
		foreach ($values as $key => $value) {
			if ($key === 'any') {
				$num = $response->response->numFound;
			} else {
				$num = $response->facet_counts->facet_fields->$facet_field->$key;
			}
			if (!isset($num))
				$num = 0;
			$values[$key] .= ' (' . $num . ')';
		}
	}

	return $values;
}

/**
 *  Helper function for retrieving number of results for facetted-like menus by .
 *	Uses solr requests for each facet value.
 *	Arguments:
 *	$facet_field -- facet field
 *	$values -- array of options of drop-down menu in form $key => $value, where $key is value of filter
 */

function tow_facet_menu_solr($facet_field, $values) {
	foreach ($values as $key => $value) {
		if ($key === 'any') {
			$num = tow_facet_menu_item($facet_field, '[* TO *]');
		} else {
			$num = tow_facet_menu_item($facet_field, $key);
		}
		if (!isset($num)) {
			$num = 0;
		}
		$values[$key] .= ' (' . $num . ')';
	}
	return $values;
}

/**
 *  Helper function for retrieving number of results for facetted-like menus for range of values.
 *	Uses solr requests for each facet value.
 *	Arguments:
 *	$facet_field -- facet field
 *	$values -- array of options of drop-down menu in form $key => $value, where $key is value of filter
 */

function tow_facet_menu_solr_range($facet_field, $values) {
	foreach ($values as $key => $value) {
		$num = tow_facet_menu_item($facet_field, '[' . $key . ' TO *]');
		if (!isset($num)) {
			$num = 0;
		}
		$values[$key] .= ' (' . $num . ')';
	}
	return $values;
}

function tow_facet_menu_item($facet_field, $value) {
	$saved_response = apachesolr_static_response_cache();
	$saved_query = apachesolr_current_query();

	$query = clone $saved_query;
	
	$query->remove_filter($facet_field);
		
	$query->add_filter($facet_field,$value);

	$params = apachesolr_search_basic_params($query);
	apachesolr_search_add_facet_params($params, $query);
	apachesolr_search_add_boost_params($params, $query, apachesolr_get_solr());
	list($final_query, $response) = apachesolr_do_query('apachesolr_search', $query, $params);
	if (empty($response)) {
	  return;
	}
//	dpm($response->facet_counts->facet_fields->$facet_field);

	apachesolr_static_response_cache($saved_response);
	apachesolr_current_query($saved_query);	
	
	return $response->response->numFound;
}

/*
*	
*Implementation of hook_apachesolr_ajax_settings
*
*/
function tow_apachesolr_ajax_settings() {
  $settings = array(
    'content' => '#content-inner-inner',
    'regions' => array(
      'sidebar_last' => '.sidebar-last',
    ),
  );

  $modules = apachesolr_ajax_modules();
  foreach ($modules as $module) {
    if (module_exists($module)) {
      if ($list = module_invoke($module, 'block', 'list')) {
        foreach (array_keys($list) as $delta) {
          $settings['blocks'][$module .'_'. $delta] = '#block-'. $module .'-'. $delta;
        }
      }
    }
  }
  //dpm($settings);
  return $settings;
}