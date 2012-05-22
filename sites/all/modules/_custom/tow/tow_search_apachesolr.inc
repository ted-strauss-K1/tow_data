<?php

/**
 * Implementation of hook_apachesolr_process_results()
 * @param: $results - non-empty array of search results
 */
function tow_apachesolr_process_results(&$results) {
  if ($results[0]['node']->type == 'dataset') { // if search was made for datasets
    tow_apachesolr_dataset_process_results($results);
  }
  else {// search for records option
    tow_apachesolr_record_process_results($results);
  }
}

/**
 * Implementation of part of hook_apachesolr_process_results()
 * for the case of outer search (over all datasets)
 * @param: $results - non-empty array of search results
 */
function tow_apachesolr_dataset_process_results(&$results) {
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

/**
 * Implementation of part of hook_apachesolr_process_results()
 * for the case of inner search (within dataset)
 * @param: $results - non-empty array of search results
 */
function tow_apachesolr_record_process_results(&$results) {
  $response = apachesolr_static_response_cache(NULL, 'tow_inner_search');
//  dpm($response->facet_counts);
//  dpm($response->stats);
  //$response->facet_counts->facet_fields->$facet_field->$key;
  if (isset($results[0]))
    $table_nid = $results[0]['node']->ss_tow_field_table;
  else
    return NULL;
  
  $fields = tow_apachesolr_table_fields($table_nid);
  $rows = array();
  $headers = array();
  foreach ($results as $key => $result) {
    $row = array();
    foreach ($fields as $field_key => $field) {
      $value = $result['node']->$field['name'];
      switch ($field['type']) {
        case 'bool':
         $value = _tow_solr_to_bool($value);
         break;
        case 'date':
          _tow_solr_to_date($value);
          break;
        case 'time':
          _tow_solr_to_time($value);
          break;
        case 'datetime':
          _tow_solr_to_datetime($value);
          break;
        case 'timestamp':
          _tow_solr_to_timestamp($value);
          break;
        case 'enum':
          $value = $field['enum_options'][$value];
        default:
         break;
      }
      $row[$field_key] = array('data' => $value);
      $headers[$field_key] = array('data' => $field['title']);
    }
    $rows[$key] = array('data' => $row);
  }
  $results['headers'] = $headers;
  $results['rows'] = $rows;
} 


/**
 * Helper function converting date from solr datetime format yyyy-mm-ddThh:mm:ssZ 
 * to usual format yyyy-mm-dd
 */
function _tow_solr_to_date(&$value) {
  $value = str_replace('T', ' ', $value);
  $value = str_replace('Z', '', $value);
  $value = date('Y-m-d', strtotime($value));
}

/**
 * Helper function converting time from solr datetime format yyyy-mm-ddThh:mm:ssZ 
 * to usual format hh:mm:ss
 */
function _tow_solr_to_time(&$value) {
  $value = str_replace('T', ' ', $value);
  $value = str_replace('Z', '', $value);
  $value = date('H:i:s', strtotime($value));
}
/**
 * Helper function converting datetime from solr datetime format yyyy-mm-ddThh:mm:ssZ 
 * to usual format yyyy-mm-dd hh:mm:ss
 */
function _tow_solr_to_datetime(&$value) {
  $value = str_replace('T', ' ', $value);
  $value = str_replace('Z', '', $value);
  $value = date('Y-m-d H:i:s', strtotime($value));
}
/**
 * Helper function converting timestamp from solr datetime format yyyy-mm-ddThh:mm:ssZ 
 * to usual int timestamp
 */
function _tow_solr_to_timestamp(&$value) {
  $value = date('Y-m-d H:i:s', $value);
}
/**
 * Helper function converting timestamp from solr datetime format yyyy-mm-ddThh:mm:ssZ 
 * to usual int timestamp
 */
function _tow_solr_to_bool(&$value) {
  if ($value == '1')
    return 'true';
  if ($value == '0')
    return 'false';
  else
    return NULL;
  
}
/**
 * Implementation of hook_apachesolr_modify_query()
 */
function tow_apachesolr_modify_query(&$query, &$params) {
  $current_tab = tow_get_homepage_tab(); 
  if ($current_tab == 'search_dataset') {
    // Inner search
    tow_apachesolr_record_modify_query($query, $params);
  } else if ($current_tab == 'newest' || $current_tab == 'debated' || $current_tab == 'featured' || $current_tab == 'popular'){
    // Outer search
    tow_apachesolr_dataset_modify_query($query, $params);
  }
  // TODO:bookmarks 
}

/**
 * Implementation of part of hook_apachesolr_modify_query()
 * for the case of outer search (over all datasets)
 */
function tow_apachesolr_dataset_modify_query(&$query, &$params) {
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

/**
 * Implementation of part of hook_apachesolr_modify_query()
 * for the case of inner search (within dataset)
 */
function tow_apachesolr_record_modify_query(&$query, &$params) {
 // TODO: add this only to records
//    dpm(get_class_methods(get_class($query)));
  $nid = tow_search_inner_get_nid();
  $table_fields = tow_apachesolr_table_fields($nid, TRUE);

  $params['fl'] .= ',ss_tow_field_table';
  if (is_numeric($nid)) {
    $query->add_filter('type', 'record');  
  } else {
    $query->add_filter('type', 'linked_record');  
  }
  $params['rows'] = 10000000;
  $query->add_filter('ss_tow_field_table', $nid);
  
  $params['stats'] = 'true';
  $params['facet.limit'] = 100;
  $params['facet.missing'] = 'true';
  foreach ($table_fields as $index=>$field){
    $params['fl'] .= ',' . $field['name'];
    $params['facet.field'][] = '{!ex=' . $field['name'] . '}' . $field['name'];
    //add stats
    if ($field['type'] != 'bool' && $field['type'] != 'text' && $field['type'] != 'char')  {
      $params['stats.field'][] = $field['name'];
    }
    
    $field_filters = $query->get_filters($field['name']);
    $subquery = apachesolr_drupal_query();
    foreach ($field_filters as $field_filter) {
      $query->remove_filter($field_filter['#name'], $field_filter['#value'], $field_filter['#exclude']);
      if ($field['type'] == 'enum') {//wrap in quotes
        $field_filter['#value'] = '"' . $field_filter['#value'] . '"';
      }
      if ($field['type'] == 'text' || $field['type'] == 'char') {//wrap in quotes
        $field_filter['#value'] = preg_replace('/--/', '\\--', $field_filter['#value']);
        $field_filter['#value'] = preg_replace('/(?<!\\\\)\+/', '--', $field_filter['#value']);
      }
      $subquery->add_filter($field['name'], $field_filter['#value'], $field_filter['#exclude']);
    }
    $query->add_subquery($subquery, 'OR');          
  }
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
    
  } elseif ($node->type == 'record') {// Add dataset_nid field and table_nid field to be indexed by apachesolr
//    watchdog('inner_search', 'node: <pre>' . print_r($node, TRUE) . '</pre>');
    $document->sis_tow_dataset_nid = db_result(db_query("
      SELECT `ctt`.`field_dataset_value` AS `dataset_nid`
      FROM {content_type_table} `ctt`
      LEFT JOIN {content_type_record} `ctr`
        ON `ctr`.`field_table_value` = `ctt`.`nid`
      WHERE `ctr`.`nid` = %d", $node->nid));   
    $table_nid = $node->field_table[0]['value'];
    $document->ss_tow_field_table = $table_nid;
    
    foreach (tow_apachesolr_table_fields($table_nid) as $field) {
      $value = $node->{'field_' . $field['type']}[$field['delta']]['value'];
      if (isset($value)) {
        _tow_add_value_to_doc(&$document, $field['type'], $field['delta'], $value);
      }
    }
//    watchdog('inner_search', 'doc: <pre>' . print_r($document, TRUE) . '</pre>');
  } elseif ($node->type == 'linked_record') {
    $tables = array();
    foreach ($node->field_tables as $table) {
      $tables[] = $table['value'];
    }
    $sorted_tables = $tables;
    sort($sorted_tables);
    $document->ss_tow_field_table = implode('+', $sorted_tables);
    $records = array();
    foreach ($tables as $key => $table_nid) {
      $record_nid = $node->field_record_nid[$key]['value'];
      $record = node_load($record_nid);
      if ($table_nid == 53) 
        watchdog('linked_search', 'record from 53: <pre>' . print_r($record, TRUE) . '</pre>');

      $records[$table_nid] = $record;
//      watchdog('linked_search', 'record from ' . $table_nid . ': <pre>' . print_r($record, TRUE) . '</pre>');
    }
    foreach (tow_apachesolr_table_fields($tables) as $field) {
      foreach ($field['table'] as $table) {
        $record = $records[$table['nid']];
        if (!empty($record)) {
          //watchdog('linked_search', 'field: <pre>' . print_r($field, TRUE) . '</pre>');
          $value = $record->{'field_' . $field['type']}[$table['delta']]['value'];
          if (isset($value)) {
            _tow_add_value_to_doc(&$document, $field['type'], $field['delta'], $value);
          }
//          watchdog('linked_search', $field['name'] . '(t'. $table['nid'] . ', d' . $field['table'][0]['delta'] .') => ' . $document->$field['name']);
          break;
        }
      }
    }
//    watchdog('linked_search', 'doc: <pre>' . print_r($document, TRUE) . '</pre>');
  }
}

function _tow_add_value_to_doc(&$document, $field_type, $delta, $value) {
  switch ($field_type) {
    case 'time':
      _tow_time_to_solr($value);
      break;
    case 'date':
      _tow_date_to_solr($value);
      break;
    case 'datetime':
      _tow_datetime_to_solr($value);
      break;
    case 'char':
    case 'text':
    case 'code':
      $words_field_name = 'sis_tow_field_' . $field_type . '_' . $delta . '_words';
      $length_field_name = 'sis_tow_field_' . $field_type . '_' . $delta . '_length';
      $words = str_word_count($value);
      $length = strlen($value);
      $document->$words_field_name = $words;
      $document->$length_field_name = $length;
//      watchdog('inner_search', $words_field_name . ' => ' . $document->$words_field_name);
//      watchdog('inner_search', $length_field_name . ' => ' . $document->$length_field_name);
      break;
    default: 
      break;
  }
  $apachesolr_field_name = tow_apachesolr_field_name($field_type, $delta);
  $document->$apachesolr_field_name = $value;
//  watchdog('inner_search', $apachesolr_field_name . ' => ' . $document->$apachesolr_field_name);
}

/**
 * Helper function converting date from usual format yyyy-mm-dd
 * to solr datetime format yyyy-mm-ddThh:mm:ssZ 
 */
function _tow_date_to_solr(&$value) {
  $value = date('Y-m-d', strtotime($value)) .'T00:00:00Z';
}

/**
 * Helper function converting time from usual format hh:mm:ss 
 * to solr datetime format yyyy-mm-ddThh:mm:ssZ 
 */
function _tow_time_to_solr(&$value) {
  $value = '0001-01-01T' . date('H:i:s',strtotime($value)) .'Z';
}
/**
 * Helper function converting datetime from usual format yyyy-mm-dd hh:mm:ss
 * to solr datetime format yyyy-mm-ddThh:mm:ssZ 
 */
function _tow_datetime_to_solr(&$value) {
  $int_time = strtotime($value);
  $value = date('Y-m-d', $int_time) . 'T' . date('H:i:s', $int_time) .'Z';
}

/**
 * Helper function converting timestamp from timestamp 
 * to solr datetime format yyyy-mm-ddThh:mm:ssZ 
 */
function _tow_timestamp_to_solr(&$value) {
  $value = gmdate('Y-m-d', $value) . 'T' . gmdate('H:i:s', $value) .'Z';
}

/** 
 * Returns prefix of ApacheSolr dynamic single-valued field 
 * corresponding to ToW field of table record.
 **/ 
function tow_apachesolr_field_type($type) {
  switch ($type) {
    case 'int': 
    case 'timestamp': 
    case 'enum': 
      return 'sis'; //integer

    case 'code': 
      return 'ss'; //string

    case 'float': 
      return 'fs'; //sfloat

    case 'bool': 
      return 'bs'; //boolean
      break;

    case 'date': 
    case 'datetime': 
    case 'time': 
      return 'ds'; //date

    case 'char': 
    case 'text': 
    default: 
      return 'sts'; //text (no break)
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
 *  Uses analyse of single solr request.
 *  Arguments:
 *  $facet_field -- facet field
 *  $values -- array of options of drop-down menu in form $key => $value, where $key is value of filter. 
 *    Array should has accending sorting on keys
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
    
    $params['facet.limit'] = 100;
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
 *  Uses solr requests for each facet value.
 *  Arguments:
 *  $facet_field -- facet field
 *  $values -- array of options of drop-down menu in form $key => $value, where $key is value of filter
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
 *  Uses solr requests for each facet value.
 *  Arguments:
 *  $facet_field -- facet field
 *  $values -- array of options of drop-down menu in form $key => $value, where $key is value of filter
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

/*
 * Returns array of fields of table with given nid
 * name, type, title, delta
 */
function tow_apachesolr_table_fields($nid, $add_facet_string_fields = FALSE) {
  static $current_nid = NULL;
  static $fields = array();
  static $f_fields = array();
  if ($nid != $current_nid || empty($fields)) {
    $fields = array();
    $f_fields = array();
    $current_nid = $nid;
    if (is_numeric($nid)) { // separate table
      list($fields, $f_fields) = _tow_apachesolr_separate_table_fields($nid);
    } else { // nid not numeric - linked table
      if (!is_array($nid)) {
        $nid = explode('+', $nid);
      }
      sort($nid);
      $deltas = array();
      $added_indexes = array();
      $key = 0;
      foreach($nid as $table_nid) {
        list($table_fields, $table_f_fields) = _tow_apachesolr_separate_table_fields($table_nid);
        foreach ($table_fields as $field) {
          $add_to_list = TRUE;
          if ($field['index']) {
            if (!isset($added_indexes[$field['type']]))
              $added_indexes[$field['type']] = array('titles' => array(), 'keys' => array());
            $duplicate = array_search($field['title'], $added_indexes[$field['type']]['titles']);
            if ($duplicate === FALSE) {
              $added_indexes[$field['type']]['titles'][] = $field['title'];
              $added_indexes[$field['type']]['keys'][] = $key;
            } else {
              $key_ = $added_indexes[$field['type']]['keys'][$duplicate]; // key of added index field 
              $add_to_list = FALSE;
              $fields[$key_]['table'][] = array(
                'nid' => $table_nid, 
                'delta' => $field['delta']
              );
            }
          }
          if ($add_to_list) {//field is either not index or is not index duplicate
            if (isset($deltas[$field['type']])) {
              $deltas[$field['type']]++;
            } else {
              $deltas[$field['type']] = 0;
            }
            $field['table'] = array();
            $field['table'][0] = array(
              'nid' => $table_nid, 
              'delta' => $field['delta']
            );
//            $field['table_nid'] = $table_nid;
//            $field['table_delta'] = $field['delta'];
            $field['delta'] = $deltas[$field['type']];
            $field['name'] = tow_apachesolr_field_name($field['type'], $field['delta']);
            $fields[] = $field;
            tow_search_apachesolr_add_facet_field($f_fields, $field);
            $key++;
          }
        }
      }
    }
  }
  return $add_facet_string_fields ? $f_fields : $fields; 
}

/*
 * Returns array of fields of separate table with given nid
 * name, type, title, delta
 */
function _tow_apachesolr_separate_table_fields($nid) {xdebug_break();
//  watchdog('linked_search', $nid);
  $fields = array();
  $f_fields = array();
  $table_node = node_load($nid, NULL, TRUE);
  if ($table_node->type == 'table') {
    $index = 0;
    $weights_array = unserialize($table_node->field_weights[0]['value']);
    $indexes_array = unserialize($table_node->field_indexes[0]['value']);
    foreach (tow_possible_fields() as $field_type) {
      $field_title_name = 'field_title_' . $field_type;
      foreach ($table_node->$field_title_name as $delta => $field_title) {
        if ($field_title['value'] != NULL) {
          if ($field_type == 'enum') {
            $enum_options = unserialize($table_node->field_enum_options[$delta]['value']);
          } else {
            $enum_options = NULL;
          }
          $x = $weights_array[$index];
          $is_index = $indexes_array[$x];
          $field = array(
            'name' => tow_apachesolr_field_name($field_type, $delta), 
            'type' => $field_type, 
            'title' => $field_title['value'], 
            'delta' => $delta,
            'enum_options' => $enum_options,
            'index' => $is_index//$indexes_array[$weights_array[$index]],
          );
          $fields[$weights_array[$index]] = $field;
          tow_search_apachesolr_add_facet_field($f_fields, $field);
          $index++;
        }
      }
    }
    ksort($fields);
  } else {
    drupal_not_found();
    die();
  }
  return array($fields, $f_fields);
}

function tow_search_apachesolr_add_facet_field(&$f_fields, $field) {
  $f_fields[] = $field;  
  if ($field['type'] == 'text' || $field['type'] == 'char') {
    $field['name'] = 'f_' . $field['name'];
    $f_fields[] = $field;

    $field['name'] = 'sis_tow_field_' . $field['type'] . '_' . $field['delta'] . '_words';
    $f_fields[] = $field;

    $field['name'] = 'sis_tow_field_' . $field['type'] . '_' . $field['delta'] . '_length';
    $f_fields[] = $field;
  }
}

function tow_apachesolr_field_name($field_type, $delta) {
  return tow_apachesolr_field_type($field_type) . '_tow_field_' . $field_type . '_' . $delta;  
}