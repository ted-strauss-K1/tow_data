<?php
/**
 * Inner search simple search block
 */
 
function tow_search_inner_simple_search_block() {
	
  $block = array();
  
  $block['title'] = 'Simple search';
  $block['content'] = drupal_get_form('tow_search_inner_simple_search_form');
	
	return $block;

}

function tow_search_inner_simple_search_form() {
	$form = array();

	$form['nid'] = array(
		'#type' => 'hidden',
		'#value' => tow_search_inner_get_nid(),
	); 	
	$form['keywords'] = array(
		'#type' => 'textfield',
		'#value' => tow_search_inner_get_keys(),
	); 	

	return $form;
}

/**
 * Inner search facets block
 */
function tow_search_inner_widgets_block() {
	
  $block = array();
  
  $block['title'] = 'Filters';

	$saved_response = apachesolr_static_response_cache(NULL, 'tow_inner_search');
	$saved_query = apachesolr_current_query(NULL, 'tow_inner_search');
	$query = clone $saved_query;
	$params = apachesolr_search_basic_params($query);

	if (isset($saved_response->response->docs[0]))
		$table_nid = $saved_response->response->docs[0]->sis_tow_field_table;
	else {
		$table_nid = tow_search_inner_get_nid();
		return $block;
	}
	
// get stats for full range
	foreach (tow_apachesolr_table_fields($table_nid, TRUE) as $index=>$field){
		$query->remove_filter($field['name']);
	}
	$query->set_keys('');
//	dpm(get_class_methods('Solr_Base_Query'));
	list($final_query, $stats_response) = apachesolr_do_query('tow_inner_search', $query, $params);
	if (empty($stats_response))
		return;
	// apachesolr_static_response_cache($saved_response, 'tow_inner_search');
	// apachesolr_current_query($saved_query, 'tow_inner_search');	
	
	
	$query = clone $saved_query;
	$params = apachesolr_search_basic_params($query);

	// save query and stats to db
	$serialized_query = serialize($query); 
	$serialized_stats = serialize($stats_response->stats->stats_fields);
	$hash = mt_rand(1000, 9999);
 	db_query("INSERT INTO {tow_apachesolr_requests} (tnid, query, stats, hash, timestamp)
              VALUES ('%d', '%s', '%s', '%d', '%s')", $table_nid, $serialized_query, $serialized_stats, $hash, date('Y-m-d H:i:s', time()));
	$rid = db_last_insert_id('tow_apachesolr_requests', 'rid');
	$block['content'] = drupal_get_form('tow_seach_inner_hash_form', $rid, $hash);
	
	//building solr query for facets for each field
	$table_fields = tow_apachesolr_table_fields($table_nid);
	foreach ($table_fields as $index=>$field){
		$stats = isset($stats_response->stats->stats_fields->$field['name']) ? $stats_response->stats->stats_fields->$field['name'] : NULL;
		$facet_counts = (array) $saved_response->facet_counts->facet_fields->$field['name'];
//		$empty = $saved_response->facet_counts->facet_fields->$field['name']->_empty_;
		tow_search_inner_facet_query($params, $field['name'], $field['type'], $stats, count($facet_counts) < 0);
	}
	
	//request to solr using request built
	$params['rows'] = 0;
	$params['facet'] = 'true';
	$params['facet.limit'] = 100;
	$params['facet.missing'] = 'true';
	$params['facet.range.include'][] = 'edge';
	$params['facet.range.include'][] = 'lower';
	$params['facet.range.hardend'] = 'true';
	
	list($final_query, $response) = apachesolr_do_query('tow_inner_search', $query, $params);
	if (empty($response))
		return;
	
	apachesolr_static_response_cache($saved_response, 'tow_inner_search');
	apachesolr_current_query($saved_query, 'tow_inner_search');	
	
	//creating facet for each field
	foreach ($table_fields as $index=>$field){
		global $tow_search_inner_field_delta;
		$tow_search_inner_field_delta = $field['delta'];
		
		$field_filters = $query->get_filters('{!tag=' . $field['name'] . '}' . $field['name']);
		foreach ($field_filters as $field_filter) {
			$query->remove_filter('{!tag=' . $field['name'] . '}' . $field['name'], $field_filter['#value']);
			$query->add_filter($field['name'], $field_filter['#value']);
		}			
		$newquery = clone $saved_query;
		$newquery->remove_filter($field['name']);
		$newquery->remove_filter('f_'. $field['name']);
		$queryvalues = $newquery->get_url_queryvalues();

		$querystring = tow_search_inner_compose_query($queryvalues['filters'], FALSE);
		$reset = l(t('Reset'), $_GET['q'], array('query' => $querystring));
		$whole_counts = count((array) $stats_response->facet_counts->facet_fields->$field['name']) - 1; // exclude _empty_ field
		$widget = tow_search_inner_widget($field['name'], $field['type'], $response, $query, $stats_response->stats->stats_fields->$field['name'], $whole_counts, 'sis_tow_field_' . $field['type'] . '_' . $field['delta']);
		$block['content'] .= '<div class="tow-inner-search-widget"><b>' . $field['title'] . '</b><br />' . $widget . '<br>' . $reset . '</div>';
		
	}
	return $block;
}

/**
 * Part of hook_cron implementation 
 * Clears old requests to solr from db table {tow_apachesolr_requests}
 */
function tow_search_inner_cron(){
	$time = date('Y-m-d H:i:s', time() - 2*24*60*60); // 2 days before
	$result = db_query("DELETE FROM {tow_apachesolr_requests} 
			WHERE timestamp < '%s'", $time);
	watchdog('inner search', 'Solr requests addressed before ' . $time . ' are cleared from database (' . $result . ')');
}
/**
 * Form at seach inner page containing hidden values for rid and hash 
 * for chart updating while zooming
 */
function tow_seach_inner_hash_form($form_state, $rid, $hash){
	$form = array();

	$form['rid'] = array(
		'#type' => 'hidden',
		'#value' => $rid,
	); 	
	$form['hash'] = array(
		'#type' => 'hidden',
		'#value' => $hash,
	); 	
	return $form;
}

/**
 * Builds part of solr query for getting facet numbers for table field
 */
function tow_search_inner_ajax_field_zooming() {xdebug_break();
	$rid = $_GET['rid'];
	$hash = $_GET['hash'];
	if ($_GET['op'] == 'zoom') {
		$visible_min = $_GET['visible_min'];
		$visible_max = $_GET['visible_max'];
		$field_type = $_GET['field_type'];
		$field = $_GET['field'];

		db_query("UPDATE {tow_apachesolr_requests} 
			SET timestamp='%s' WHERE rid='%d' AND hash='%d'", date('Y-m-d H:i:s', time()), $rid, $hash);

		$result = db_fetch_array(db_query("SELECT 
		query, stats, tnid FROM {tow_apachesolr_requests} WHERE rid='%d' AND hash='%d'", $rid, $hash));


		
		$serialized_query = $result['query'];
		$serialized_stats = $result['stats'];
		$table_nid = $result['tnid'];
		
		$_GET['q'] = 'search_dataset/' . $table_nid;
		
		$query = apachesolr_drupal_query(NULL,NULL,NULL,NULL);
		$query = unserialize($serialized_query);

		$stats = unserialize($serialized_stats);
		$params = array(); 
		
		//building solr query for facets for each field
		$stats =  isset($stats->$field) ? $stats->$field : NULL;
		tow_search_inner_facet_query($params, $field, $field_type, $stats, FALSE, $visible_max - $visible_min);
		
		//request to solr using request built
		$params['rows'] = 0;
		$params['facet'] = 'true';
		$params['facet.limit'] = 100;
		$params['facet.missing'] = 'true';
		$params['facet.range.include'][] = 'edge';
		$params['facet.range.include'][] = 'lower';
		$params['facet.range.hardend'] = 'true';
		
		list($final_query, $response) = apachesolr_do_query('tow_inner_search', $query, $params);
		if (empty($response))
			return drupal_json(NULL);

		list($data, $facets_list, $start, $end, $max_count) = tow_search_inner_chart_data($field, $field_type, $response->facet_counts->facet_ranges->$field);
		$series = array();
		foreach($data as $index => $point) {
			$series[] = array(
				'x' => $point[0],
				'y' => $point[1],
				'name' => $facets_list[$index],
			);
		}
		return drupal_json(array('data' => $series, 'yMax' => $max_count));
	} else {
		db_query("DELETE FROM {tow_apachesolr_requests} 
			WHERE rid='%d' AND hash='%d'", $rid, $hash);
	}
}

/**
 * Builds part of solr query for getting facet numbers for table field
 */
function tow_search_inner_facet_query(&$params, $field, $type, $stats, $barchart, $visible_range = 0) {
	if (!$barchart){
		switch ($type) {
			case 'int':
				$min = $stats->min;
				$max = $stats->max;
				$gap = _tow_search_inner_facet_gap($max - $min, $visible_range, _tow_search_inner_natural_gap($type));
				_tow_search_inner_add_facet_range(&$params, $field, $min, $max, $gap);
				break;
			case 'float': 
				$min = $stats->min;
				$max = $stats->max;
				$gap = _tow_search_inner_facet_gap($max - $min, $visible_range);
				_tow_search_inner_add_facet_range(&$params, $field, $min, $max, $gap);
				break;
			case 'date': 
				if ($stats){
					$min = $stats->min;
					$max = $stats->max;
					$timestamp_min = strtotime($min);
					$timestamp_max = strtotime($max);
					$gap = _tow_search_inner_facet_gap($timestamp_max - $timestamp_min, $visible_range/1000, _tow_search_inner_natural_gap($type));
					$gap = '+' . $gap . 'SECOND';
					_tow_search_inner_add_facet_range(&$params, $field, $min, $max, $gap);
				}
				break;
			case 'datetime': 
				if ($stats){
					$min = $stats->min;
					$max = $stats->max;
					$timestamp_min = strtotime($min);
					$timestamp_max = strtotime($max);
					$gap = _tow_search_inner_facet_gap($timestamp_max - $timestamp_min, $visible_range/1000, _tow_search_inner_natural_gap($type));
					_tow_search_inner_add_facet_range(&$params, $field, $min, $max, '+' . $gap . 'SECOND');
				}
				break;
			case 'time': 
				$gap = _tow_search_inner_facet_gap(24*60*60, $visible_range/1000, _tow_search_inner_natural_gap($type), 24);
				_tow_search_inner_add_facet_range(&$params, $field, '0001-01-01T00:00:00Z', '0001-01-01T23:59:59Z', '+' . $gap . 'SECOND');
				break;
			case 'timestamp': 		
				$min = $stats->min;
				$max = $stats->max;
				$gap = _tow_search_inner_facet_gap($max - $min, $visible_range/1000, _tow_search_inner_natural_gap($type));
				_tow_search_inner_add_facet_range(&$params, $field, $min, $max, $gap);
				break;
			case 'bool': 
			case 'enum': 
				$params['facet.field'][] = '{!ex=' . $field . '}' . $field; // 
				break;
			case 'text': 
				$field = 'f_' . $field;
				$params['facet.field'][] = '{!ex=' . $field . '}' . $field;
				break;
			case 'char': 
			case 'code': 
			default:
				$params['facet.field'][] = '{!ex=' . $field . '}' . $field;
				break;
		}
	} else {
		$params['facet.field'][] = '{!ex=' . $field . '}' . $field;
		switch ($type) {
			case 'int':
			case 'float': 
			case 'date': 
			case 'datetime': 
			case 'time': 
			case 'timestamp': 		
				$params['f.' . $field . '.facet.sort'] = 'index';
				break;
			case 'bool': 
			case 'enum': 
			case 'char': 
			case 'code': 
			case 'text': 
			default: 
				break;
		}
	}
}

/**
 * Calculates optimal facet gap
 */
function _tow_search_inner_facet_gap($range, $visible_range, $step = NULL, $number_of_points = 40){
	if ($visible_range == 0)
		$visible_range = $range;
		
	$minstep = $visible_range/$number_of_points;
	if (is_null($step)) {
		return $minstep;
	} else if ($minstep < $step) {
		return $step;
	} else {
		return $step * ceil($minstep/$step);
	}
}

/**
 * Adds facet range for specific field to solr request
 */
function _tow_search_inner_add_facet_range(&$params, $field, $min, $max, $gap){
	if (!isset($params['facet.range']) || !in_array($field, $params['facet.range']))
		$params['facet.range'][] = '{!ex=' . $field . '}' . $field;

	$params['f.' . $field . '.facet.range.start'] = $min;
	$params['f.' . $field . '.facet.range.end'] = $max;
	$params['f.' . $field . '.facet.range.gap'] = $gap;
}

/**
 * Builds part of inner search facets block for specific field
 */
function tow_search_inner_widget($field, $type, $response, $query, $stats, $whole_counts, $length_field = NULL) {
	$empty = $response->stats->stats_fields->$field->missing;
	switch ($type) {
		case 'int':
		case 'float':
		case 'date':
		case 'datetime':
		case 'time':
		case 'timestamp':
			return tow_search_inner_graph_widget($field, $type, $response->facet_counts, $empty, $stats);
		case 'bool': 
		case 'code': 
			$facets_list = tow_search_inner_facet_widget($response, $query, 'tow_inner_search', $field, $field, '');
			return $facets_list['content'];
		case 'enum': 
			$facets_list = tow_search_inner_facet_widget($response, $query, 'tow_inner_search', $field, $field, '', 'tow_search_facet_enum_option');
			return $facets_list['content'];
		case 'text': 
			$facet_field = 'f_' . $field; 
			$facet_list = tow_search_inner_facet_widget($response, $query, 'tow_inner_search', $facet_field, $facet_field, '', 'tow_search_facet_trim_text');
			$form = drupal_get_form('tow_search_inner_text_length_widget_form', $length_field) . drupal_get_form('tow_search_inner_text_widget_form', $field, $type);
			return $facet_list['content'] . $form;
		case 'char': 
			$facet_field = 'f_' . $field;
			$facet_list = tow_search_inner_facet_widget($response, $query, 'tow_inner_search', $facet_field, $facet_field, '');
			$form = drupal_get_form('tow_search_inner_text_length_widget_form', $length_field) . drupal_get_form('tow_search_inner_text_widget_form', $field, $type);
			return ($whole_counts > 50) ? $facet_list['content'] . $form : $facet_list['content'];
		default: 
			return NULL;
	}
}

/*
 *	Returns graphical widget
 */
function tow_search_inner_graph_widget($field, $type, $facet_counts, $empty, $stats){
	if (isset($facet_counts->facet_ranges->$field)) 
		return tow_search_inner_chart_widget($field, $type, $facet_counts->facet_ranges->$field, $empty);
	else 
		return tow_search_inner_bar_widget($field, $type, $facet_counts->facet_fields->$field, $empty, $stats);
}

/*
 *	Returns line chart widget
 */
function tow_search_inner_chart_widget($field, $field_type, $facet_counts, $empty){

	list($data, $facets_list, $start, $end, $max_count) = tow_search_inner_chart_data($field, $field_type, $facet_counts);
	$data = drupal_to_js($data);
	$facets_list = drupal_to_js($facets_list);
	
	// limits of selection range
	$selection_limits = tow_search_inner_range_limits($field);
	if ($selection_limits['min']) 
		$selection_limits['min'] = _tow_solr_to_js($field_type, $selection_limits['min']);
	if ($selection_limits['max']) 
		$selection_limits['max'] = _tow_solr_to_js($field_type, $selection_limits['max']);

		$form = drupal_get_form('tow_search_inner_numeric_facet_form', $field, $start, $end, $empty, $selection_limits['min'], $selection_limits['max'], $data, $facets_list, $field_type, $max_count);
	$container = '<div class="tow-inner-search-highcharts-container" id="' . $field . '_highchart"></div>';
	return $container . $form;
}

/**
 * Returns array of data for chart (points and tooltips)
 */
function tow_search_inner_chart_data($field, $field_type, $facet_counts){

	$facets_list = array();
	$gap = _tow_solr_gap_to_js($field_type, $facet_counts->gap);
	$start = _tow_solr_to_js($field_type, $facet_counts->start);
	$end =  _tow_solr_to_js($field_type, $facet_counts->end);
	$is_natural_gap = ($gap == _tow_search_inner_natural_gap($field_type, TRUE));
	
	$group_start = $start;
	$counts = $facet_counts->counts;
	$data = array();
	$sum = 0;
	$max_count = 0;
	if ($field_type == 'float'){
		$counts = (array) $counts;
		
		$count_in_bin = array();
		foreach ($counts as $group_start => $count) {
			$number_of_bin = round(($group_start - $start)/$gap);
			$count_in_bin[$number_of_bin] = $count;
		}
		
		$data = array();
		$ii = 0;
		$group_start = $start;
		while ($group_start + $gap/2 < $end) {
			$group_end = $start + ($ii + 1) * $gap;
			if (isset($count_in_bin[$ii])) {
				$group_counts = $count_in_bin[$ii];
				if ($group_counts > $max_count)
					$max_count = $group_counts;
			} else
				$group_counts = 0;
			
			$facets_list[] = _tow_search_inner_range($group_start, $group_end, $is_natural_gap) . _tow_search_inner_number_of_results($group_counts);
			$data[] = array(($group_start + $group_end)/2, $group_counts);
			$sum += $group_counts;
			$ii++;
			$group_start = $group_end;
		};
		
	} else {
		while ($group_start < $end) {
			$interval_start = _tow_js_to_solr($field_type, $group_start);
			$interval_end = _tow_js_to_solr($field_type, $group_start + $gap - 1);
			$function = '_tow_solr_to_' . $field_type;
			if (function_exists($function)) { //turn date and time to displayed view
				$function($interval_start);
				$function($interval_end);
			}
			
			//turn group start to solr format
			$solr_group_start = _tow_js_to_solr($field_type, $group_start);

			if (isset($counts->$solr_group_start)) {
				$group_counts = $counts->$solr_group_start; 
				if ($group_counts > $max_count)
					$max_count = $group_counts;
			} else
				$group_counts = 0;
				
			$facets_list[] = _tow_search_inner_range($interval_start, $interval_end, $is_natural_gap) . _tow_search_inner_number_of_results($group_counts);
			$data[] = array($group_start + ($gap-1)/2,  $group_counts);
			$sum += $group_counts;
			$group_start += $gap;
		}
	}
	//include end value to last point
	array_pop($facets_list);
	array_pop($data);
	$interval_start = _tow_js_to_solr($field_type, $group_start - $gap);
	$interval_end = _tow_js_to_solr($field_type, $end);
	$function = '_tow_solr_to_' . $field_type;
	if (function_exists($function)) { //turn date and time to displayed view
		$function($interval_start);
		$function($interval_end);
	}
	array_push($facets_list, _tow_search_inner_range($interval_start, $interval_end, $is_natural_gap) . 
		_tow_search_inner_number_of_results($group_counts));
	array_push($data, array(($group_start - $gap + $end)/2, $group_counts));
	
	return array($data, $facets_list, $start, $end, $max_count);
}

function _tow_search_inner_range($start, $end, $is_natural_step){
	if ($is_natural_step)
		return $start . ':\n';
	else
		return $start . '...\n' . $end . ':\n';
}

/**
 * Returns natural gap for specifiec field type
 * 1 for int, +1SECOND for date and datetime etc.
 */
function _tow_search_inner_natural_gap($field_type, $js = FALSE){
	switch ($field_type){
		case 'int':
			return 1;
		case 'date':
			if ($js)
				return 24*60*60*1000;
			else 
				return 24*60*60;
		case 'datetime':
		case 'time':
		case 'timestamp':
			if ($js)
				return 1000;
			else 
				return 1;
		case 'float':
		default:
			return NULL;
	}
}


/**
 * Theming function. Return 'N results' for N!=1 and 1 result for N=1
 */
function _tow_search_inner_number_of_results($N){
	if ($N == 1)
		return t('1 result');
	else
		return $N . t(' results');
}
/**
 * Helper function. Converts value from js format to solr format 
 * (actions on date and time formats only)
 */
function _tow_js_to_solr ($type, $value) {
	switch ($type){
		case 'int':
		case 'float':
			return $value;
		case 'date':
		case 'datetime':
			$value = round($value/1000);
			_tow_timestamp_to_solr($value);
			return $value;
		case 'timestamp':
			return round($value/1000);
		case 'time':
			$value = round($value/1000);
			$value = gmdate('H:i:s', $value);
			return '0001-01-01T' . $value . 'Z';
		default:
			return NULL; 
	}
}

/**
 * Helper function. Converts value from solr format to js format 
 * (actions on date and time formats only)
 */
function _tow_solr_to_js($type, $value) {
	switch ($type){
		case 'int':
		case 'float':
			return $value;
		case 'date':
		case 'datetime':
			//$value = str_replace('T', ' ', $value);
			//$value = str_replace('Z', '', $value);
			return strtotime($value) * 1000;
		case 'timestamp':
			return $value * 1000;
		case 'time':
			_tow_solr_to_time($value);
			return strtotime('1970-01-01 ' . $value . 'Z') * 1000;
		default:
			return NULL; 
	}
}



/**
 * Helper function. Converts gap value from solr format to js format 
 * (actions on date and time formats only)
 */
function _tow_solr_gap_to_js($type, $value) {
	switch ($type){
		case 'int':
		case 'float':
			return $value;
		case 'date':
		case 'datetime':
		case 'time':
			$value = preg_replace('/\D/', '', $value);
			return $value * 1000;
		case 'timestamp':
			return $value * 1000;
		default:
			return NULL; 
	}
}

/*
 *	Returns bar chart widget
 */
function tow_search_inner_bar_widget($field, $field_type, $facet_counts, $empty, $stats){
	$data = array();
	$function = '_tow_solr_to_' . $field_type;
	$function_exists = function_exists($function);
	foreach ($facet_counts as $value=>$count) {
		if ($count)  {
			if ($function_exists) {
				$function($value);
			}
			$facets_list[] = $value;
			$data[] = $count;
		}
	}
	
// limits of selection range
	$selection_limits = tow_search_inner_range_limits($field);
	if ($function_exists) {
		if ($selection_limits['min']) 
			$function($selection_limits['min']);
		if ($selection_limits['max']) 
			$function($selection_limits['max']);
		$function($stats->min);
		$function($stats->max);
	}
	
	$data = drupal_to_js($data);
	$facets_list = drupal_to_js($facets_list);
	
		
	$form = drupal_get_form('tow_search_inner_numeric_facet_form', $field, $stats->min, $stats->max, $empty, $selection_limits['min'], $selection_limits['max'], $data, $facets_list, $field_type);
	$container = '<div class="tow-inner-search-highcharts-bar-container" id="' . $field . '_highchart"></div>';
	return $container . $form;
}


function tow_search_inner_numeric_facet_form($form_state, $field, $min, $max, $empty, $selection_min, $selection_max, $data = NULL, $tooltips = NULL, $fieldtype = 'int', $max_count = 0) {
	$form = array();

	$saved_query = apachesolr_current_query(NULL, 'tow_inner_search');
	$query = clone $saved_query;
	
	//handling empty fields	
	$empty_query = clone $saved_query;
	
	$empty_included = FALSE;
	$no_other_filters = TRUE;
	foreach ($empty_query->get_filters($field) as $filter) {
		if ($filter['#value'] == '[* TO *]')	{
			$empty_included = TRUE;
		} else {
			$value = $filter['#value'];
			$no_other_filters = FALSE;
		}
	}
	
	if ($no_other_filters)
		$empty_included = !$empty_included;
		
	if ($no_other_filters && $empty_included) {// 1-1: default state
		// link for button submit
		$query->add_filter($field, '[* TO *]', TRUE);
		$query->add_filter($field, $field . '_placeholder');
		// link for click on checkbox
		$empty_query->add_filter($field, '[* TO *]');
	} elseif (!$no_other_filters && $empty_included) {// 0-1
		// link for button submit
		$query->remove_filter($field, $value);
		$query->add_filter($field, $field . '_placeholder');
		// link for click on checkbox
		$empty_query->remove_filter($field, '[* TO *]');
	} elseif ($no_other_filters && !$empty_included) {// 1-0
		// link for button submit
		$query->remove_filter($field, '[* TO *]');
		$query->add_filter($field, $field . '_placeholder');
		// link for click on checkbox
		$empty_query->remove_filter($field, '[* TO *]');
		$empty_query->remove_filter($field, $value);
	} else {// 0-0
		// link for button submit
		$query->remove_filter($field, $value);
		$query->add_filter($field, $field . '_placeholder');
		// link for click on checkbox
		$empty_query->add_filter($field, '[* TO *]', TRUE);
	}

	$filters = $query->get_url_queryvalues();
	$querystring = tow_search_inner_compose_query($filters['filters'], FALSE);
	$url = url($_GET['q'], array('query' => $querystring));

	$filters = $empty_query->get_url_queryvalues();
	$querystring = tow_search_inner_compose_query($filters['filters'], FALSE);
	$include_empty_url= url($_GET['q'], array('query' => $querystring));

	$form['data'] = array(
		'#type' => 'hidden',
		'#value' => $data,
	); 
	
	$form['tooltips'] = array(
		'#type' => 'hidden',
		'#value' => $tooltips,
	); 
	
	$form['max_count'] = array(
		'#type' => 'hidden',
		'#value' => $max_count,
	);

	$form['url'] = array(
		'#type' => 'hidden',
		'#value' => $url,
	); 

	$form['field'] = array(
		'#type' => 'hidden',
		'#value' => $field,
	); 

	$form['fieldtype'] = array(
		'#type' => 'hidden',
		'#value' => $fieldtype,
	); 

	$form['include_empty_url'] = array(
		'#type' => 'hidden',
		'#value' => $include_empty_url,
	); 
	
	$filters = $_GET['filters'] ? 'filters=' . $_GET['filters'] : '';
	 
	$form['include_empty'] = array(
		'#type' => 'checkbox',
		'#title' => t('Empty') . ' (' . $empty . ')',
		'#value' => $empty_included,
	); 
	
	$form['global_min'] = array(
		'#type' => 'hidden',
		'#value' => $min,
	); 
	
	$form['global_max'] = array(
		'#type' => 'hidden',
		'#value' => $max,
	); 	
	
	$form['selection_min'] = array(
		'#type' => 'hidden',
		'#value' => ($selection_min === FALSE) ? $min : $selection_min,
	); 
	
	$form['selection_max'] = array(
		'#type' => 'hidden',
		'#value' => ($selection_max === FALSE) ? $max : $selection_max,
	); 	
	
	$form['min'] = array(
		'#type' => 'hidden',
		'#value' => $min,
	); 
	
	$form['max'] = array(
		'#type' => 'hidden',
		'#value' => $max,
	); 

	$form['search'] = array(
		'#type' => 'button',
		'#title' => t('Search'),
		'#value' => t('Search'),
		'#attributes' => array('onclick' => 'return false;'),
	); 	
	
	return $form;
}



function tow_search_inner_text_widget_form($form_state, $field, $fieldtype) {
	
	$form = array();

/* field info */

	$form['field'] = array(
		'#type' => 'hidden',
		'#value' => $field,
	); 

	$form['fieldtype'] = array(
		'#type' => 'hidden',
		'#value' => $fieldtype,
	);  

	$form['length_field'] = array(
		'#type' => 'hidden',
		'#value' => $length_field,
	);  


/* search by substring */

 	$saved_query = apachesolr_current_query(NULL, 'tow_inner_search');
	$query = clone $saved_query;
	$active_filters = $query->get_filters($field);
	if ($active_filters) {
		$option = preg_replace('/[^\*]++/', '_', $active_filters[0]['#value']);
		$value = preg_replace('/\*/', '', $active_filters[0]['#value']);
		if ($active_filters[0]['#exclude'])
			$option = '-' . $option;
		$value = preg_replace('/(?<!\\\\)\+/', ' ', $value);
//		$value = preg_replace('/\\\\-/', '-', $value);
		$value = preg_replace('/\\\\(\+|-|&&|\|\||!|\(|\)|\{|\}|\[|\]|\^|"|~|\*|\?|:|\\\\)/', '$1', $value);

		dpm($value);
	}
	
	// positive search (string includes substring)
	$query->remove_filter($field);
	$placeholder = $field . '_placeholder';
	$query->add_filter($field, $field . '_placeholder');
	$filters = $query->get_url_queryvalues();
	$querystring = tow_search_inner_compose_query($filters['filters'], FALSE);
	$url = url($_GET['q'], array('query' => $querystring));

	$form['url'] = array(
		'#type' => 'hidden',
		'#value' => $url,
	); 
		
	// negative  search (string doesn't include substring)
	$query->remove_filter($field, $field . '_placeholder');
	$query->add_filter($field, $field . '_placeholder', TRUE);
	$filters = $query->get_url_queryvalues();
	$querystring = tow_search_inner_compose_query($filters['filters'], FALSE);
	$negative_url =  url($_GET['q'], array('query' => $querystring));

	$form['negative_url'] = array(
		'#type' => 'hidden',
		'#value' => $negative_url,
	); 

	// input fields for search by substring
	$form['option'] = array(
		'#type' => 'select',
 		'#options' => array(
							'_*' => 'starts with', 
							'*_' => 'ends with', 
							'*_*' => 'contains', 
							'-*_*' => 'not contains', 
							'_' => 'equals', 
							'-_' => 'not equals',
		),
	); 
	
	if (isset($option)) 
		$form['option']['#value'] = $option;

	$form['value'] = array(
		'#type' => 'textfield',
	); 	
	if (isset($value)) 
		$form['value']['#value'] = $value;

/* Search button	*/
	$form['search'] = array(
		'#type' => 'button',
		'#title' => t('Search'),
		'#value' => t('Search'),
		'#attributes' => array('onclick' => 'return false;'),
	); 	 
	
	return $form;
}



function tow_search_inner_text_length_widget_form($form_state, $length_field) {
	$form = array();

/* field info */

	$form['fieldtype'] = array(
		'#type' => 'hidden',
		'#value' => 'length',
	);  

	$form['field'] = array(
		'#type' => 'hidden',
		'#value' => $length_field,
	);  


/* search by # of words/characters  */
	$saved_query = apachesolr_current_query(NULL, 'tow_inner_search');
	$query = clone $saved_query;
	$placeholder = $length_field . '_placeholder';

	$prefix = '_words';
	$active_filters = $query->get_filters($length_field . $prefix);
	if (!$active_filters){
		$prefix = '_length';
		$active_filters = $query->get_filters($length_field . $prefix);
	}
	if ($active_filters) {
		if (is_numeric($active_filters[0]['#value'])) {
			// = 
			$value = $active_filters[0]['#value'];
			$option = $prefix . ':#';
		} else if ($active_filters[0]['#start'] != '*') {
			// >
			$value = $active_filters[0]['#start'] - 1;
			$option = $prefix . ':[# TO *]';
		} else {
			// <
			$value = $active_filters[0]['#end'] + 1;
			$option = $prefix . ':[* TO #]';
		}
	}
	
	// positive search (string includes substring)
	$query->remove_filter($length_field . '_words');
	$query->remove_filter($length_field . '_length');
	
	$query->add_filter($length_field, $placeholder);
	$filters = $query->get_url_queryvalues();
	$querystring = tow_search_inner_compose_query($filters['filters'], FALSE);
	$url = url($_GET['q'], array('query' => $querystring));

	$form['url'] = array(
		'#type' => 'hidden',
		'#value' => $url,
	); 	
	// input fields for search by	# of words/characters
	$form['option'] = array(
		'#type' => 'select',
 		'#options' => array(
			'_length:#' => '# characters =', 
			'_length:[* TO #]'=> '# characters <', 
			'_length:[# TO *]'=> '# characters >', 
			'_words:#' => '# words =', 
			'_words:[* TO #]'=> '# words <', 
			'_words:[# TO *]'=> '# words >',
		),
	); 

	if (isset($option)) 
		$form['option']['#value'] = $option;

	$form['value'] = array(
		'#type' => 'textfield',
	); 	
	if (isset($value)) 
		$form['value']['#value'] = $value;
		

/* Search button	*/
	$form['search'] = array(
		'#type' => 'button',
		'#title' => t('Search'),
		'#value' => t('Search'),
		'#attributes' => array('onclick' => 'return false;'),
	); 	 
	
	return $form;
}

function tow_search_facet_trim_text($facet, &$options){
	$alter=array(
			'html' => TRUE,
			'ellipsis' => TRUE,
			'word_boundary' => FALSE,
			'max_length' => 15,
	  ); 

	  return views_trim_text($alter,$facet);

}

function tow_search_facet_enum_option($facet, &$options){
	global $tow_search_inner_field_delta;

	$response = apachesolr_static_response_cache(NULL, 'tow_inner_search');

	if (isset($response->response->docs[0]))
		$table_nid = $response->response->docs[0]->sis_tow_field_table;

	$table_node = node_load($table_nid);

	$enum_options = unserialize($table_node->field_enum_options[$tow_search_inner_field_delta]['value']);
	

	if ($enum_options[$facet]) {
		return $enum_options[$facet];
	} else {
		$options['html'] = TRUE;
		return theme('placeholder', t('<empty>'));
	}
}

/**
 * Menu callback
 */
function tow_search_within_dataset_callback($nid) {
//  x('tow_search_within_dataset_callback called');
//  x($_GET);
	
  $type = 'apachesolr_search';
  $keys = trim(tow_search_inner_get_keys());

  // We did not redirect, so run the search if needed.
  $results = '';

  $filters = trim($_GET['filters']);
/* 	$filters = tow_get_search_outer_filters();
	$filters = $filters['query'];
	$_GET['filters'] = $filters;
 */	
	// Log the search keys:
	$log = $keys;
	if ($filters) {
	  $log .= 'filters='. $filters;
	}
	watchdog('search', '%keys (@type).', array('%keys' => $log, '@type' => t('Search')), WATCHDOG_NOTICE, l(t('results'), 'search/'. $type .'/'. $keys));

	// Collect the search results:
	$results = search_data($keys, 'tow_inner_search');
	drupal_add_js(drupal_get_path('module', 'tow') . '/js/jquery.ba-hashchange.min.js');
	drupal_add_js(drupal_get_path('module', 'tow') . '/js/inner_search.js');
	drupal_add_js(drupal_get_path('module', 'tow') . '/js/highcharts.js');

	if ($results) {
	  return $results;
	}
	else {
	  return theme('box', t('Your search yielded no results'), variable_get('apachesolr_search_noresults', apachesolr_search_noresults()));
	}
	
	
	////////////////////////////
	
	
  // x($_POST);
  $output = '';
  
  // 0 Check URL 
  // 0.1 if nid belongs to dataset 
  $node = node_load($nid);
  if (!$node || $node->type != 'dataset') {
    drupal_not_found();
    die();
  }
  
/*  global $dataset;
  global $table;
  global $user;
  */
  //$dataset = node_load($nid);
  //$dataset = (object)array('nid' => $dataset->nid, 'title' => $dataset->title, 'uid' => $dataset->uid);
  //$table = $node;
    
  /*$output = $node->teaser;
  if ($user->uid == $dataset->uid || user_access('edit any table content'))
    $output .= "<div>" . l('Edit', 'node/' . $node->nid . '/edit', array('query' => 'destination=table/' . $node->nid)) . "</div>";
  */

  // 0.2 Check search parameters
  foreach ($_GET as $param => $value) {
    
  }  
  
  
  //http://43_tow_local/search_dataset/1?scan_tables=3,4&disp_tables=3,4&disp_fields=t3-char-2,t3-enum-3,t3-char-4,t3-int-5,t4-char-1&save=&ssname=&ovwr=
  $search_parameters = array (
    'scan_tables' => explode(',', $_GET['scan_tables']),
  	'disp_tables' => explode(',', $_GET['disp_tables']),
  	'disp_fields' => explode(',', $_GET['disp_fields']),
    'save_search' => $_GET['save'],
    'ss_name' => $_GET['ssname'],
    'overwrite' => $_GET['ovwr'],
    
  );
  
  // 1 Add search form
  // 1.1 Add empty form
  $output .= drupal_get_form('tow_search_form', $nid);
  
  // 1.2 Fill in the form with search parameters
  
  
  
//  $search_options = tow_search_within_dataset_options();
  
  /*
  $display_options['sort'] = array ('sort_index' => 2, 'sort_order' => 'ASC');
  $display_options['filter'] = array ('filter_column_index' => 3, 'filter_operator_key' => 0, 'filter_value' => 2004);
  $display_options['limits'] = array ('first_elem_no' => 0, 'total_elements' => 10);
  $display_options['paged_view'] = true;
  
  $display_options['paged_view'] = TRUE;
  $display_options['limits']['total_elements'] = 10;                   
  $display_options['click_sortable'] = TRUE;
  */
//  $output .= tow_search_within_dataset($node, $search_options);
  
  /*if ($user->uid == $dataset->uid)
    $output .= _tow_table_actions_list($node->nid, 'edit');
  */
  return $output;
}

function tow_search_inner_get_keys() {
  static $return;
  if (!isset($return)) {
	// Extract keys as remainder of path
	// Note: support old GET format of searches for existing links.
		$path = explode('/', $_GET['q'], 3);
		$keys = empty($_REQUEST['keys']) ? '' : $_REQUEST['keys'];
		$return = count($path) == 3 ? $path[2] : $keys;
  }
  return $return;
}

/**
 * Helper function for displaying a facet block.
 */
function tow_search_inner_facet_widget($response, $query, $module, $delta, $facet_field, $filter_by, $facet_callback = FALSE) {
  if (!empty($response->facet_counts->facet_fields->$facet_field)) {
    $contains_active = FALSE;
    $items = array();
    foreach ($response->facet_counts->facet_fields->$facet_field as $facet => $count) {
      $options = array();
      $exclude = FALSE;

      // Solr sends this back if it's empty.
      if ($facet == '_empty_') {
        $exclude = TRUE;
        $facet = '[* TO *]';
        $options['html'] = TRUE;
      }
			$options['html'] = TRUE;
			
      if ($facet_callback && function_exists($facet_callback)) {
        $facet_text = $facet_callback($facet, $options);
      }
      elseif ($exclude) {
        $facet_text = theme('placeholder', t('<empty>'));
      }
      else {
        $facet_text = $facet;
      }

      $active = $query->has_filter($facet_field, $facet);

      if ($active) {
        // '*' sorts before all numbers.
        $sortpre = '*';
      }
      elseif ($exclude) {
        // '-' sorts before all numbers, but after '*'.
        $sortpre = '-';
      }
      else {
        $sortpre = 1000000 - $count;
      }

      $new_query = clone $query;
      if ($active) {
        $contains_active = TRUE;
        $new_query->remove_filter($facet_field, $facet);
        $options['query'] = $new_query->get_url_queryvalues();
        $link = theme('tow_search_inner_unclick_link', $facet_text, $new_query->get_path(), $options, $count, FALSE, $response->response->numFound);
      }
      else {
        $new_query->add_filter($facet_field, $facet, $exclude);
        $options['query'] = $new_query->get_url_queryvalues();
        $link = theme('tow_search_inner_facet_link', $facet_text, $new_query->get_path(), $options, $count, FALSE, $response->response->numFound);
      }

      if ($count || $active) {
        $items[$sortpre . '*' . $facet_text] = $link;
      }
    }
    // Unless a facet is active only display 2 or more.
    if ($items && ($response->response->numFound >= 0 || $contains_active)) {
      ksort($items, SORT_STRING);
      // Get information needed by the rest of the blocks about limits.
      $initial_limits = variable_get('apachesolr_facet_query_initial_limits', array());
      $limit = isset($initial_limits[$module][$delta]) ? $initial_limits[$module][$delta] : variable_get('apachesolr_facet_query_initial_limit_default', 10);
      $output = theme('apachesolr_facet_list', $items, $limit);
      return array('subject' => $filter_by, 'content' => $output);
    }
  }
  return NULL;
}

function theme_tow_search_inner_unclick_link($facet_text, $path, $options = array(), $count = 0, $active = FALSE, $num_found = NULL) {
  if (empty($options['html'])) {
    $facet_text = check_plain(html_entity_decode($facet_text));
  }
  else {
    // Don't pass this option as TRUE into apachesolr_l().
    //unset($options['html']);
  }
  $options['attributes']['class'] = 'tow-inner-search-selected';
  return tow_search_inner_l($facet_text ." ($count)", $path, $options);
}


function theme_tow_search_inner_facet_link($facet_text, $path, $options = array(), $count, $active = FALSE, $num_found = NULL) {
  $options['attributes']['class'][] = 'apachesolr-facet';
  if ($active) {
    $options['attributes']['class'][] = 'active';
  }
  $options['attributes']['class'] = implode(' ', $options['attributes']['class']);
	$link = tow_search_inner_l($facet_text ." ($count)",  $path, $options);
  return $link;
}

/**
 * A replacement for l()
 *  - doesn't add the 'active' class
 *  - retains all $_GET parameters that ApacheSolr may not be aware of
 *  - if set, $options['query'] MUST be an array
 *
 * @see http://api.drupal.org/api/function/l/6 for parameters and options.
 *
 * @return
 *   an HTML string containing a link to the given path.
 */
function tow_search_inner_l($text, $path, $options = array()) {
  // Merge in defaults.
  $options += array(
    'attributes' => array(),
    'html' => FALSE,
    'query' => array(),
  );

  // Don't need this, and just to be safe.
  unset($options['attributes']['title']);
  // Double encode + characters for clean URL Apache quirks.
  if (variable_get('clean_url', '0')) {
    $path = str_replace('+', '%2B', $path);
  }

  // Retain GET parameters that ApacheSolr knows nothing about.
  $query = apachesolr_current_query(NULL, 'tow_inner_search');
  $get = array_diff_key($_GET, array('q' => 1, 'page' => 1), $options['query'], $query->get_url_queryvalues());
  $options['query'] += $get;

  return '<a href="'. check_url(url($path, $options)) .'"'. drupal_attributes($options['attributes']) .'>'. ($options['html'] ? $text : check_plain(html_entity_decode($text))) .'</a>';
}

/*
/**
 * Build form for simple search.
 
function tow_search_within_dataset_simple() {
  $form['dataset_nid'] = array(
    '#type' => 'value',
    '#value' => $dataset_nid,
  );
  $form['simple_search'] = array(
    '#tree' => TRUE,
    'textfield' => array(
      '#type' => 'textfield',
      '#title' => t('Simple search'),
	  '#default_value' => $form['basic']['inline']['keys']['#default_value'],
  ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Go'),
	  //'#submit' => array('tow_search_within_dataset_simple_submit'),
  //'file' => 'tow_search_inner.inc',
    )
  );
  return $form;
}

/**
 * Build form for advanced search.
 
function tow_search_within_dataset_advanced() {
 
}
*/

/**
 * Creates form elements for the search_within_dataset page
 */
function tow_search_within_dataset_altering_form(&$form, &$form_state, $dataset_nid) {

  global $dataset;
  $dataset = node_load($dataset_nid);
  if ($dataset->type != 'dataset') {
    drupal_access_denied();
    die();
  }

  drupal_add_js(drupal_get_path('module', 'tow') . '/js/search.js');
 
  drupal_set_title($dataset->title);

  $form['dataset_nid'] = array(
    '#type' => 'value',
    '#value' => $dataset_nid,
  ); 
  
  $form['simple_search'] = array(
    '#tree' => TRUE,
    'textfield' => array(
      '#type' => 'textfield',
      '#title' => t('Simple search'),
	  '#default_value' => $form['basic']['inline']['keys']['#default_value'],
  ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Go'),
	  '#submit' => array('tow_search_within_dataset_submit_simple'),
  //'file' => 'tow_search_inner.inc',
  )
  );

  $form['advanced_search'] = array(
    '#tree' => TRUE,
    '#title' => t('Advanced search form'),
    'title' => array(
      '#type' => 'item',
      '#title' => t('Advanced search form'),
  )
  );

  $form['clear'] = array(
    '#type' => 'item',
    '#value' => t('Clear form'),
    '#prefix' => '<div class="clear-form">',
    '#suffix' => '</div>'
    );
    $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
	'#submit' => array('tow_search_within_dataset_submit'),
    //'file' => 'tow_search_inner.inc',
    );

    $form['saved_searches'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Saved searches'),
    'select' => array(
      '#type' => 'select',
      '#options' => tow_search_get_saved_searches($dataset->nid),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Load results'),
	  '#submit' => 'tow_search_within_dataset_load_saved_search',
    )
    );

    $form['tables'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Which tables are you interested in?'),
    'select' => array(
      '#type' => 'select',
      '#options' => tow_table_list_with_links($dataset_nid),
      '#multiple' => TRUE
    )
    );

    // FILTERS
    $form['filters'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Constrain & filter the data; search for terms and values'),
    'items' => array(
      '#prefix' => '<div id="filter-items">',
      '#suffix' => '</div>',
    ),
    '#prefix' => '<div class="clear-block" id="filter-wrapper">',
    '#suffix' => '</div>',
    );

    if (is_array($form_state['values']['filters']['items'])) {
      $form['filters']['existing_fields'] = array(
      '#type' => 'value',
      '#value' => $form_state['values']['filters']['existing_fields'],
      //'#attributes' => array('class' => 'filters-inactive'),
      );
      $form['filters']['existing_enum_options'] = array(
      '#type' => 'value',
      '#value' => $form_state['values']['filters']['existing_enum_options'],
      //'#attributes' => array('class' => 'filters-inactive'),
      );
    }
    else {
      $form['filters']['existing_fields'] = array(
      '#type' => 'value',
      '#value' => tow_search_get_fields($dataset_nid), 
      //'#attributes' => array('class' => 'filters-inactive'),
      );
      $form['filters']['existing_enum_options'] = array(
      '#type' => 'value',
      '#value' => tow_search_get_enum_options($dataset_nid), 
      //'#attributes' => array('class' => 'filters-inactive'),
      );
      $form['filters']['existing_operators'] = array(
      '#type' => 'value',
      '#value' => tow_search_get_operators(),
      );

    }

    $form['filters']['add_more'] = array(
    '#type' => 'button',
    '#value' => t('Add more'),
    '#weight' => 100,
    '#ahah' => array(
      'path' => ahah_helper_path(array(
        'filters', 'filter-items-wrapper', 'filter-items'
        )),
      'wrapper' => 'filter-items-wrapper',
      'effect' => 'fade',
      'event' => 'click',
      'method' => 'append',
        ),
        );

        $form['filters']['filter-items-wrapper'] = array(
    '#prefix' => '<div id="filter-items-wrapper">',
    '#suffix' => '</div>',
        //'filter-items' => _tow_search_get_filter_element($form['filters']['existing_fields']['#value'], $form['filters']['existing_operators']['#value'])
    'filter-items' => _tow_search_get_filter_element($form['filters']['existing_fields']['#value'], $form['filters']['existing_enum_options']['#value']),
        );

        $form['fields'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Select which fields to display in the results'),
    'select_tables' => array(
      '#type' => 'checkboxes',
      '#options' => $form['tables']['select']['#options'],
      '#title' => t('Tables')
        ),
    'select_fields' => array(
      '#type' => 'checkboxes',
      '#options' => $form['filters']['existing_fields']['#value'],
      '#title' => t('Fields'),
        //'#disabled' => true,
        )
        );

        $form['agsp'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Averaging, grouping, sorting and processing the results'),
    'result_type' => array(
      '#type' => 'select',
      '#options' => array('Individual results', 'Group by'),
      '#title' => t('Result type')
        ),
    'sort_by' => array(
      '#type' => 'select',
      '#options' => $form['filters']['existing_fields']['#value'],
      '#title' => t('Sort by')
        ),
    'filter_null' => array(
      '#type' => 'checkbox',
      '#title' => t('Filter out rows that have one or more null records')
        ),
    'limit' => array(
      '#type' => 'select',
      '#prefix' => '<span class="display-amount">' . t('Display') . '</span>',
      '#suffix' => '<span>' . t('results per page') . '</span>',
      '#options' => array(25, 50, 100, 250) 
        ),
        );

        $form['save_search'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Save this search'),
    'save' => array(
      '#type' => 'checkbox',
      '#title' => t('Save this search')
        ),
    'textfield' => array(
      '#type' => 'textfield',
      '#description' => t('Name this search something descriptive'),
        ),
    'overwrite' => array(
      '#type' => 'checkbox',
      '#title' => t('Overwrite an existing saved search with this name (only allowed if you created original)')
        ),
    'note' => array(
      '#type' => 'item',
      '#value' => t('Note: the administrator of this data set can remove your saved search if they decided to')
        )
        );

        $form['clear2'] = $form['clear'];
        $form['submit2'] = $form['submit'];

        // Turn off apachesolr form features, if they are set
        if (isset($form['basic'])) {
          unset($form['basic']);
        }
        if (isset($form['#submit'])) {
          unset($form['#submit']);
        }
        if (isset($form['#validate'])) {
          unset($form['#validate']);
        }
        if (isset($form['#action'])) {
          unset($form['#action']);
        }
        // x($form);
        return $form;

}

/**
 * Returns list of saved searches, connected with dataset
 */
function tow_search_get_saved_searches($dataset_nid) {
  $saved_search_res = db_query("SELECT `ctss`.`nid`, `n`.`title`
                                     FROM content_type_saved_search `ctss`
                                     LEFT JOIN node `n`
                                       ON `n`.`nid` = `ctss`.`nid`
                                     WHERE `ctss`.`field_dataset_nid_value` = %d" , $dataset_nid);
  // TODO: pass saved search nids (?)
  while ($saved_search = db_fetch_array($saved_search_res)) {
    $saved_searches[] = $saved_search['title'];
  }
  if ($saved_searches) {
    return $saved_searches;
  }
  else
  return array();
}

/**
 * Returns filter block elements
 */
function _tow_search_get_filter_element($fields, $enum_options) {

  // All elements that may be displayed in constraint area are described here. Furher hiding or
  // displaying is managed by js.
  $element = array(
    '#tree' => TRUE,
    '#prefix' => '<div>',
    '#suffix' => '</div>',
    'remove' => array(
      '#value' => t('Remove'),
      '#prefix' => '<div class="remove-filter">',
      '#suffix' => '</div>',
  	  '#weight' => 0,	  
      ),
    'field' => array(
      '#type' => 'select',
      //'#attributes' => array('style' => 'display:none'),
      '#options' => $fields,
      '#title' => t('Field'),
      '#weight' => 1,
      ),
    'operator' => array(
      '#type' => 'select',
      '#options' => tow_search_get_operators(),
      '#title' => t('operator'),
      '#weight' => 2,
      ),
    'value_1' => array(
      '#type' => 'textfield',
      '#title' => t('Value 1'),
      '#prefix' => '<div class="gen-value-field-1">',
      '#suffix' => '</div>',
      '#weight' => 3,
      ),
    'value_2' => array(
      '#type' => 'textfield',
      '#title' => t('Value 2'),
      '#prefix' => '<div class="gen-value-field-2">',
      '#suffix' => '</div>',
      '#weight' => 4,
      ),
    'date-within' => array(
  		'#type' => 'select',
      	'#options' => tow_search_get_date_within_operators(),
      	'#title' => t('Date within'),
        '#weight' => 5,
      ),
    'date_value_1' => array(
  		'#type' => 'textfield',
  		'#title' => 'Date value',
      	'#weight' => 6,
      ),
    'date_value_2' => array(
  		'#type' => 'textfield',
  		'#title' => 'Second date value',
      	'#weight' => 8,
      ),
    'enum_list' => array(
  		'#type' => 'select',
      	'#options' => $enum_options,
      	'#title' => t('Enum options'),
        '#multiple' => true,
        '#weight' => 10,
      ),

      );
      // nice date element
      yuicalendar_build_form($element, 'date_value_1', time(), 7, NULL, TRUE);
      yuicalendar_build_form($element, 'date_value_2', time(), 9, NULL, TRUE);
      return $element;
}

/**
 * Returns field titles of all the tables from the dataset $nid
 */
function tow_search_get_fields($nid) {

  $possible_fields = tow_possible_fields();
  $res = db_query("SELECT nid FROM {content_type_table} WHERE field_dataset_value = %d", $nid);

  while ($node = db_fetch_object($res)) {
    $node = node_load($node->nid);
    $table_fields = _tow_get_table_fields($node);

    $field_id = 0;
    foreach($table_fields as $field) {
      $value = 't' . $node->nid . '-' . $field['type']. '-' . $field_id++;
      $fields[$value] = $field['title'];
    }
  }
  return $fields;
}

/**
 * Returns comparison operators, that may be used in constraints
 */
function tow_search_get_operators() {
  $operators = array(	'text-equal'    => 'equals',
  				'text-contains' => 'contains',
  				'text-notcontains'  => 'doesn\'t contain',
  				'text-length-equal' => 'length =',
  				'text-length-greater' => 'length >',
                'text-length-less'     => 'length <',

                'int-equal'    => 'equals', 
  				'int-greater' => 'greater than',
                'int-less'     => 'less than',
                'int-between' => 'between',
  				'int-notequals'  => 'not equals',

                'date-equal'    => 'equals', 
  				'date-less'     => 'before',
                'date-greater' => 'after',
                'date-between' => 'between',
  				'date-within'  => 'within',

  				'enum-equal'    => 'equals', 
  				'enum-notequals'  => 'not equals',
                'enum-anyof'     => 'any of',
  );
  return $operators;
}

/**
 * Returns some more operators for date-within filtering operator
 */
function tow_search_get_date_within_operators() {
  $operators = array(	'3-days'    => '3 days',
  				'3-weeks' => '3 weeks',
  				'3-months'  => '3 months',
  				'1-year' => '1 year',
  				'2-years' => '2 years',
                '5-years'     => '5 years',

  );
  return $operators;
}

/**
 * Returns all enum options for all the tables from the dataset $nid
 */
function tow_search_get_enum_options($nid) {

  $all_enum_options = array();

  $res = db_query("SELECT `cfeo`.`nid` as `table_nid`, `cfeo`.`field_enum_options_value` as `enum_opt`
  					FROM {content_field_enum_options} `cfeo`
  					LEFT JOIN {content_type_table} `ctt`
  					ON `cfeo`.`nid` = `ctt`.`nid`
  					WHERE `ctt`.`field_dataset_value` = %d", $nid);

  while ($enum_options = db_fetch_array($res)) {
    if ($enum_options['enum_opt']) {
      $table_enum_options = unserialize($enum_options['enum_opt']);
      foreach ($table_enum_options as $old_key => $option) {
        $new_key = 't' . $enum_options['table_nid'] . '-' . $old_key;
        $table_enum_options[$new_key] = $option;
        unset($table_enum_options[$old_key]);
      }

      $all_enum_options = array_merge($all_enum_options, $table_enum_options);
    }
  }
  return $all_enum_options;
}

/**
 * Returns list of dataset tables together with links between these tables
 */
function tow_table_list_with_links($dataset_nid) {
  // 0. Get flat list of tables of a dataset
  $table_list = tow_table_list($dataset_nid);
  // 1. Get titles of indexed fields
  $table_index_fields = array();
  foreach ($table_list as $table_nid => $table_title) {

    $table_node = node_load($table_nid);

    $table_fields = _tow_get_table_fields($table_node);

    $key_column_titles = array();
    foreach ($table_fields as $col_index => $field) {
      if ($field['index']) {
        $key_column_titles[$col_index] = $field['title'];
      }
    }
    $table_index_fields[$table_nid] = $key_column_titles;
  }

  // 2.0
  $linked_tables = array();
  // 2.1 Compare index fields of the tables and determine linked tables
  foreach ($table_index_fields as $table_nid => $index_fields) {
    foreach ($index_fields as $field_index => $field_title) {
      foreach ($table_index_fields as $table_nid_2 => $index_fields_2) {
        // go to comparing field titles only for different tables
        if ($table_nid != $table_nid_2) {
          foreach ($index_fields_2 as $field_index_2 => $field_title_2) {
            if ($field_title ==  $field_title_2) {
              // titles match
              $linked_tables[$table_nid][$table_nid_2] = $table_list[$table_nid_2];
              // stop checking this table
              break;
            } // if

          } // 2nd field loop
        }  // if
      } // 2nd table loop
    } // 1st field loop
  } // 1st table loop

  // 3. Format list of tables for display
  $linked_tables_title_list = array();
  foreach ($table_list as $table_nid => $table_title) {
    // display string contains table title itself
    $linked_table_titles = $table_title;
    // and titles of linked tables, if there are any linked tables
    if (array_key_exists($table_nid, $linked_tables)) {
      $linked_table_titles .= ' (linked to ';
      foreach ($linked_tables[$table_nid] as $linked_table_nid => $linked_table_title) {
        $linked_table_titles .= $linked_table_title . ', ';
      }
      $linked_table_titles = substr($linked_table_titles, 0, -2);
      $linked_table_titles .= ')';
    }
    $linked_tables_title_list[$table_nid] = $linked_table_titles;
  }

  return $linked_tables_title_list;
}

/**
 * Redirects to the page with results. Final URL contains search string and
 * additional filter: search in records only
 */
function tow_search_within_dataset_submit_simple($form, &$form_state) {
  //x('submit simple search called');

  $args = implode($_GET);

  // Submit may be called from two types of URL:
  // search/apachesolr_search/test?filters=type:record,sis_cck_field_dataset_nid:[dataset-nid] and
  // search_dataset/[dataset-nid]
  // The following lines extract dataset-nid from both URLs

  $matches_qty = preg_match('/(?<=dataset_nid:|search_dataset\/)\d{1,}$/', $args, $dataset_nids);
  //x($dataset_nids);
  if ($matches_qty) {
    $dataset_nid = $dataset_nids[0];
  }
  else {
    // TODO: GOTO 404 page
  }

  // tell apachesolr to perform search
  $path = 'search/apachesolr_search/' . $form_state['clicked_button']['#post']['simple_search']['textfield'];

  // search in records only
  $query = 'filters=type:record';

  // limit search to the specified dataset only, separate search option by space
  $query .= ' ' . 'sis_tow_dataset_nid:' . $dataset_nid;
  drupal_goto($path, $query);

}

/**
 * Composes URL with all the search parameters and redirects user to it
 */
function tow_search_within_dataset_submit($form, &$form_state) {
  x($form_state);
  // query example
  /*$query .= 'filter=' . $form_state['values']['field'] . '&op=' . $form_state['values']['operator'] . '&value=' . $form_state['values']['value'];
  drupal_goto('table/' . $table->nid, $query);
  */
  
  $query = '';
  // dataset nid
  //$query .= 'dataset=' . $form_state['values']['dataset_nid'];
  
  // scanned tables
  $query .= 'scan_tables=';
  foreach ($form_state['values']['tables']['select'] as $value) {
       $query .= $value . ','; 
  }
   $query = substr($query, 0, -1);
  
  // tables to be displayed
  $query .= '&disp_tables=';
  $selected_tables = array_keys($form_state['values']['fields']['select_tables'], true);
  foreach ($selected_tables as $table_nid) {
     $query .= $table_nid . ','; 
  }
  $query = substr($query, 0, -1);
    
  // fields to be displayed
  $query .= '&disp_fields=';
  $selected_tables = array_keys($form_state['values']['fields']['select_fields'], true);
  foreach ($selected_tables as $table_nid) {
     $query .= $table_nid . ','; 
  }
  $query = substr($query, 0, -1);
  
  // save search?
  if ($form_state['values']['save_search']['save']) {
    $query .= '&save=' . $form_state['values']['save_search']['save'];  
    $query .= '&ssname=' . $form_state['values']['save_search']['textfield'];
    // TODO: do not add if false 
    $query .= '&ovwr=' . $form_state['values']['save_search']['overwrite'];
  
  }
  
  
  
  
  drupal_goto('search_dataset/' . $form_state['values']['dataset_nid'], $query);
  
  //x('tow_search_submit called');
  //x($form_state['values']['dataset_nid']);
  // Save search 
  /*if ($form_state['values']['save_search']['save']) {
    $new_ss = new stdClass();
    // common node parameters 
    $new_ss->type = 'saved_search';
    // TODO:
    //$new_ss->uid = 
    $new_ss->title = $form_state['values']['save_search']['textfield'];
    // nid field, if saved search with the same name exists and we are overwriting it
    // TODO:
    if (in_array($form_state['values']['save_search']['textfield'], $form_state['values']['saved_searches']) &&
      $form_state['values']['save_search']['overwrite']) {
        // TODO: $new_ss->nid = 
//        $new_ss->title = $form_state['values']['save_search']['textfield'];
      }
   
    // searched tables field
    foreach($form_state['values']['tables']['select'] as $value) {
      $new_ss->field_scanned_tables[]['value'] = $value; 
    }   
    // dataset field 
    $new_ss->field_dataset_nid[0]['value'] = $form_state['values']['dataset_nid'];
    // displayed tables
    $selected_tables = array_keys($form_state['values']['fields']['select_tables'], 1);
    foreach ($selected_tables as $table_nid) {
      $new_ss->field_displayed_tables[]['value'] = $table_nid;   
    }  
    // displayed fields
    $selected_fields = array_keys($form_state['values']['fields']['select_fields'], 1);
    $new_ss->field_displayed_fields[0]['value'] = serialize($selected_fields); 

    // search constraints 
    $new_ss->field_search_constraints[0]['value'] = serialize($form_state['values']['filters']['filter-items-wrapper']['filter-items']);
    
  }
  
  
  //x($form_state);
  // TODO: add number of results
  // 
  node_save($new_ss);*/
}

/**
 * Retrieves table nid from GET[q] search_dataset/nid -> nid
 */
function tow_search_inner_get_nid() {
	$path = explode('/', $_GET['q']);
	$return = count($path) >= 2 ? $path[1] : FALSE;
	return $return;
}

/**
 * Retrieves zooming factor from GET[zoom]
 */
function tow_search_inner_zoom($current_field) {
	$zoom_string = $_GET['zoom'];
	$zoom_array = explode(' ', $zoom_string);
	$current_zoom = 0;
	foreach ($zoom_array as $fieldzoom) {
		list($field, $zoom) = explode(':', $fieldzoom);
		if ($field == $current_field) {
			$current_zoom = $zoom;
			break;
		}
	}
	$num_of_points = tow_search_inner_zoom_points($current_zoom);
	return $num_of_points ? $num_of_points : tow_search_inner_zoom_points('0');
}

/**
 * Retrieves zooming factor from GET[zoom]
 */
function tow_search_inner_timezoom($current_field) {
	$zoom_string = $_GET['zoom'];
	$zoom_array = explode(' ', $zoom_string);
	$current_zoom = 0;
	foreach ($zoom_array as $fieldzoom) {
		list($field, $zoom) = explode(':', $fieldzoom);
		if ($field == $current_field) {
			$current_zoom = $zoom;
			break;
		}
	}
	$num_of_points = tow_search_inner_timezoom_points($current_zoom);
	return $num_of_points ? $num_of_points : tow_search_inner_zoom_points('0');
}

function tow_search_inner_zoom_points($zoom) {
		$num_of_points = array(
		'-5' => 10,
		'-4' => 15,
		'-3' => 21,
		'-2' => 28,
		'-1' => 39,
		'0' => 50,
		'1' => 67,
		'2' => 89,
		'3' => 119,
		'4' => 160,
	);
	return isset($num_of_points[$zoom]) ? $num_of_points[$zoom] : FALSE;
}

function tow_search_inner_timezoom_points($zoom) {
		$num_of_points = array(
		'-5' => 4,
		'-4' => 6,
		'-3' => 8,
		'-2' => 12,
		'-1' => 24,
		'0' => 48,
		'1' => 72,
		'2' => 96,
		'3' => 144,
		'4' => 288,
	);
	return isset($num_of_points[$zoom]) ? $num_of_points[$zoom] : FALSE;
}

function tow_search_inner_changezoom($current_field, $increase) {
	$new_zoom_array = array();
	$active = FALSE;

	$zoom_string = $_GET['zoom'];
	if ($zoom_string) {
		$zoom_array = explode(' ', $zoom_string);
		foreach ($zoom_array as $fieldzoom) {
			list($field, $zoom) = explode(':', $fieldzoom);
			if ($field == $current_field) {
				$active = TRUE;
				$increase ? $zoom++ : $zoom--;
				if (!tow_search_inner_zoom_points($zoom))
					return FALSE;
			}
			$new_zoom_array[] = $field . ':' . $zoom;
		}
	}
	if (!$active) {
		$new_zoom_array[] = $current_field . ':' . ($increase ? '1' : '-1');
	}
	return implode(' ', $new_zoom_array);
}

function tow_search_inner_remove_zoom($current_field) {
	$new_zoom_array = array();
	$active = FALSE;

	$zoom_string = $_GET['zoom'];
	if ($zoom_string) {
		$zoom_array = explode(' ', $zoom_string);
		foreach ($zoom_array as $fieldzoom) {
			list($field, $zoom) = explode(':', $fieldzoom);
			if ($field != $current_field) {
				$new_zoom_array[] = $field . ':' . $zoom;
			}
		}
	}
	return implode(' ', $new_zoom_array);
}

/* 
 * Builds GET query as ('filters' => filters, 'zoom' => zoom), 
 * omitting param with empty value and getting value from GET 
 * if value is FALSE
 */
function tow_search_inner_compose_query($filters = FALSE, $zoom = FALSE) {
	$query = array();
	_tow_search_inner_add_queryvalue($query, 'filters', $filters);
	_tow_search_inner_add_queryvalue($query, 'zoom', $zoom);
	return $query;
}

/* 
 * Builds GET query for $param, 
 * omitting param with empty value and getting value from GET 
 * if value is FALSE
 */
function _tow_search_inner_add_queryvalue(&$query, $param, $value){
	if (!empty($value) && $value !== FALSE) {
		$query[$param] = $value;
	} elseif (!empty($_GET[$param]) && $value === FALSE) {
		$query[$param] = $_GET[$param];
	}
}

function tow_search_inner_range_limits($current_field){

	$query = apachesolr_current_query(NULL, 'tow_inner_search');
	$filters = $query->get_filters($current_field);
	foreach ($filters as $filter){
		if (!$filter['#exclude'] && $filter['#start'] != '*' && $filter['#end'] != '*'){
			$limits = array('min' => $filter['#start'], 'max' => $filter['#end']);
			return $limits;
		}
	}
	$limits = array('min' => FALSE, 'max' => FALSE);
	return $limits;
}