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
  else {dpm($results); // search for records option
		if (isset($results[0]))
			$table_nid = $results[0]['node']->sis_tow_field_table;
		
		$table_node = node_load($table_nid);dpm($table_node);
		$weights_array = unserialize($table_node->field_weights[0]['value']);
		$rows = array();
		$headers = array();
		foreach ($results as $key => $result) {xdebug_break();
			$index = 0;
			$row = array();
			foreach (tow_possible_fields() as $field_type) {
				$delta = 0;
				$apachesolr_field_type = tow_apachesolr_field_type($field_type);
				if (!is_array($table_node->{'field_title_' . $field_type}))
					continue;
				foreach($table_node->{'field_title_' . $field_type} as $title_value) {
					if ($title_value['value']) {
						$apachesolr_field_name = $apachesolr_field_type . '_tow_field_' . $field_type . '_' . $delta;	
						$value = $result['node']->$apachesolr_field_name;
						switch ($field_type) {
							case 'bool':
							 $value = _tow_format_bool($value);
							 break;
							case 'date':
								$value = str_replace('T', ' ', $value);
								$value = str_replace('Z', '', $value);
								$value = date('Y-m-d', strtotime($value));
								break;
							case 'time':
								$value = str_replace('T', ' ', $value);
								$value = str_replace('Z', '', $value);
								$value = date('H:i:s', strtotime($value));
								break;
							case 'datetime':
								$value = str_replace('T', ' ', $value);
								$value = str_replace('Z', '', $value);
								$value = date('Y-m-d H:i:s', strtotime($value));
								break;
							case 'timestamp':
								$value = date('Y-m-d H:i:s', $value);
								break;
							case 'enum':
								// $title_enum = $table_node['titles']['values'][$key];
								// foreach($node->field_title_enum as $enum_delta => $enum_value) {
									// if ($enum_value['value'] == $title_enum) {
										// unset($node->field_title_enum[$enum_delta]);
										// break;
									// }
								// }
								$enum_options = unserialize($table_node->field_enum_options[$delta]['value']);
								$value = $enum_options[$value];
							default:
							 break;
						}
						$row[$weights_array[$index]] = array('data' => $value);
						$headers[$weights_array[$index]] = array('data' => $title_value['value']);
						$index++;
						$delta++;
					}
				}
			}
			if ($row)
				ksort($row);
			$rows[$key] = array('data' => $row);
    }
		if ($headers)
			ksort($headers);		
		$results['headers'] = $headers;
		$results['rows'] = $rows;
		
//		dpm($results);
  }
}

function tow_apachesolr_modify_query(&$query, &$params) {xdebug_break();
  $current_tab = tow_get_homepage_tab();
	if ($current_tab == 'search_dataset') {
	// Inner search
  // TODO: add this only to records
//		dpm(get_class_methods(get_class($query)));
		$nid = tow_search_inner_get_nid();
		$table_node = node_load($nid);
		if ($table_node->type == 'table') {
			$params['fl'] .= ',sis_tow_field_table';
			$query->add_filter('sis_tow_field_table', $nid);
//			dpm($table_node);
			
			$possible_types = tow_possible_fields();
			foreach ($possible_types as $field_type) {
				$field_name = 'field_' . $field_type;
				$field_title_name = 'field_title_' . $field_type;
				$apachesolr_field_type = tow_apachesolr_field_type($field_type);			
				foreach ($table_node->$field_title_name as $delta => $field_title) {
					if ($field_title != NULL) {
						$apachesolr_field_name = $apachesolr_field_type . '_tow_' . $field_name . '_' . $delta;	
						$params['fl'] .= ',' . $apachesolr_field_name;
					}
				}
			}

			$query->add_filter('type', 'record');  
			$params['rows'] = 10000000;
//			dpm($params);
//			dpm($query);
		}
	} else {
		// Outer search
		
		// TODO: Add this only for datasets
		// Adding fields to field list (fl)
		$params['fl'] .= ',sis_tow_amount_tables';
		$params['fl'] .= ',sis_tow_amount_records';
		$params['fl'] .= ',sis_tow_amount_saved_searches';
		$params['fl'] .= ',sis_tow_count_views';
		$params['fl'] .= ',sis_cck_field_access_type';
		$params['fl'] .= ',im_vid_1';
		$params['fl'] .= ',im_vid_2';
		$params['fl'] .= ',ss_teaser';
		
		$query->add_filter('type', 'dataset');  
		$query->add_filter('-sis_cck_field_access_type', '2');  
		
		switch ($current_tab) {
			case 'newest':
				$query->set_solrsort('created', 'desc');
			break;
			
			case 'popular':
				$query->set_solrsort('sort_title', 'asc');
			break;
			
			default:
			break;
		}
	}
  // TODO:bookmarks 
}

/** 
 * Implementation of hook_apachesolr_update_index(). 
 **/ 
function tow_apachesolr_update_index(&$document, $node) {
//  watchdog('apachesolr', 'tow_apachesolr_update_index');
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
		watchdog('inner_search', 'node: <pre>' . print_r($node, TRUE) . '</pre>');
    $document->sis_tow_dataset_nid = db_result(db_query("
      SELECT `ctt`.`field_dataset_value` AS `dataset_nid`
      FROM {content_type_table} `ctt`
      LEFT JOIN {content_type_record} `ctr`
        ON `ctr`.`field_table_value` = `ctt`.`nid`
      WHERE `ctr`.`nid` = %d", $node->nid));   
    $document->sis_tow_field_table = $node->field_table[0]['value'];
		
		$possible_fields = tow_possible_fields();
		foreach ($possible_fields as $field_type) {
			$field_name = 'field_' . $field_type;
			$apachesolr_field_type = tow_apachesolr_field_type($field_type);
			$pos = 0;
			foreach ($node->$field_name as $field) {
				if (isset($field['value'])) {
					$apachesolr_field_name = $apachesolr_field_type . '_tow_' . $field_name . '_' . $pos;
					switch ($field_type) {
						case 'time':
							$value = '0001-01-01T' . date('H:i:s',strtotime($field['value'])) .'Z';
							break;
						case 'date':
							$value = date('Y-m-d', strtotime($field['value'])) .'T00:00:00Z';
							break;
						case 'datetime':
							$value = date('Y-m-d', strtotime($field['value'])) . 'T' . date('H:i:s', strtotime($field['value'])) .'Z';
							break;
						default:
							$value = $field['value'];
							break;
					}
					$document->$apachesolr_field_name = $value;
					$pos++;
					watchdog('inner_search', $apachesolr_field_name . ' => ' . $document->$apachesolr_field_name);
				}
			}
		}
		watchdog('inner_search', 'doc: <pre>' . print_r($document, TRUE) . '</pre>');
  }
  
}

/** 
 * Returns prefix of ApacheSolr dynamic single-valued field 
 * corresponding to ToW field of table record.
 **/ 
function tow_apachesolr_field_type($type) {
	switch ($type) {
		case 'int': 
		case 'timestamp': 
			return 'is'; //integer

		case 'char': 
		case 'code': 
			return 'ss'; //string

		case 'float': 
			return 'fs'; //sfloat

		case 'bool': 
			return 'bs'; //boolean
			break;

		case 'enum': 
			return 'tis'; //tint

		case 'date': 
		case 'datetime': 
		case 'time': 
			return 'ds'; //date

		case 'text': 
		default: 
			return 'ts'; //text
	}
}

/** 
 *
 **/ 
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
	
	$do_new_search = TRUE;
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
  return $settings;
}