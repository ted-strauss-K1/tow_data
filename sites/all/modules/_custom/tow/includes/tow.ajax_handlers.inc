<?php

/**
 * @file
 * 
 * Ajax handlers.
 */



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Handles the change of access type to the dataset with Ajax.
 */
function tow_ajax_handlers_dataset_access_type($dnid) {
  
  // Get the dataset.
  $dataset = node_load($dnid);
  
  // Get the arguments.
  $access = $_GET['access'];
  $confirm = (isset($_GET['confirm']) && $_GET['confirm'] == 'true') ? TRUE : FALSE;
  
  // If the change was confirmed or the confirmation message was not shown (for 'by request' and 'private' types).
  if ($confirm) {
    $dataset->field_access_type[0]['value'] = $access;
    node_save($dataset);
  }
  
  return drupal_json(array(
    'selected' => $dataset->field_access_type[0]['value']
  ));
}

/**
 * Handles refresh of table samples with Ajax.
 */
function tow_ajax_handlers_dataset_refresh_table_samples($dnid) {
  
  // Prepare dataset.
  $dataset = node_load($dnid);
  tow_dataset_summary($dataset);
  
  // Generate the output.
  $output = '';
  if (isset($dataset->summary['view_table'])) {
    $output .= '<div class="sample-tables">';
    $output .= '<span class="field-label">Some random samples from this data set</span>';
    $output .= '<span>&nbsp;(<a href= "node/' . $dataset->nid . '">refresh</a> for a new sample)</span>';
    foreach ($dataset->summary['view_table'] as $t) {
      $output .= '<div class="random-table">';
      $output .= $t['title'];
      $output .= $t['table'];
      $output .= '</div>';
    }
    $output .= '</div>';
  }
  
  // Return the output.
  return drupal_json(array(
    'samples' => $output
  ));
}

/**
 * Builds part of solr query for getting facet numbers for table field.
 */
function tow_ajax_handlers_field_zooming() {
  $rid = $_GET['rid'];
  $hash = $_GET['hash'];
  
  if ($_GET['op'] == 'zoom') {
    $visible_min = $_GET['visible_min'];
    $visible_max = $_GET['visible_max'];
    $field_type = $_GET['field_type'];
    $field = $_GET['field'];

    db_query("
    
      UPDATE 
        {tow_apachesolr_requests} 
      
        SET 
          timestamp = '%s' 
        
        WHERE 
            rid = %d 
          AND 
            hash = %d
    
    ", time(), $rid, $hash);

    $result = db_fetch_array(db_query("
    
      SELECT 
        t.query AS query, 
        t.stats AS stats, 
        t.tnid AS tnid
        
        FROM 
          {tow_apachesolr_requests} t 
        
        WHERE 
            t.rid = %d 
          AND 
            hash = %d
    
    ", $rid, $hash));
    
    $serialized_query = $result['query'];
    $serialized_stats = $result['stats'];
    $table_nid = $result['tnid'];
    $_GET['q'] = 'search_dataset/' . $table_nid;
    
    $query = apachesolr_drupal_query(NULL, NULL, NULL, NULL);
    $query = unserialize($serialized_query);

    $stats = unserialize($serialized_stats);
    $params = array(); 
    
    // Building solr query for facets for each field.
    $stats =  isset($stats->$field) ? $stats->$field : NULL;
    tow_search_inner_facet_query($params, $field, $field_type, $stats, FALSE, $visible_max - $visible_min);
    
    // Request to solr using request built.
    $params['rows'] = 0;
    $params['facet'] = 'true';
    $params['facet.limit'] = 100;
    $params['facet.missing'] = 'true';
    $params['facet.range.include'][] = 'edge';
    $params['facet.range.include'][] = 'lower';
    $params['facet.range.hardend'] = 'true';
    
    list($final_query, $response) = apachesolr_do_query('tow_inner_search', $query, $params);
    if (empty($response)) {
      return drupal_json(NULL);
    }
    
    list($data, $facets_list, $start, $end, $max_count) = tow_search_inner_chart_data($field, $field_type, $response->facet_counts->facet_ranges->$field);
    $series = array();
    foreach ($data as $index => $point) {
      $series[] = array(
        'x' => $point[0],
        'y' => $point[1],
        'name' => $facets_list[$index],
      );
    }
    
    return drupal_json(array(
      'data' => $series, 
      'yMax' => $max_count
    ));
  } 
  else {
    db_query("
    
      DELETE 
        FROM 
          {tow_apachesolr_requests} 
      
        WHERE 
            rid = %d 
          AND 
            hash = %d
    ", $rid, $hash);
  }
}

/**
 * Gets progress stage.
 */
function tow_ajax_handlers_import_progress_get($hash, $stage) {
  global $user;

  // Get stage.
  $response = db_result(db_query("SELECT stage%s FROM {tow_import_progress} WHERE sid = '%s' AND hash = %d", $stage, $user->sid, $hash));

  // Delete actions on delete satge.
  if ($stage == 6) {
    tow_import_progress_delete($hash);
  }

  return drupal_json(array(
    'status' => TRUE, 
    'response' => $response
  ));
}

/**
 * When user clicks 'add more' return rebuilt form.
 * This is just interface, rebuilding goes in tow_register_form_alter().
 */
function tow_ajax_handlers_register_add_more() {
  
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);
  
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  
  drupal_process_form($form_id, $form, $form_state);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
   
  $items = $form['feeds']['items'];
  $output = drupal_render($items);
  
  return drupal_json(array(
    'status' => TRUE, 
    'data' => $output
  ));
}

/**
 * Refresh search results according to filters set with Ajax.
 */
function tow_ajax_handlers_refresh_search_results($nid) {
  
  // Emulate we are on the search page.
  $_GET['q'] = str_replace('/refresh_ajax', '', $_GET['q']);
  
  // Define search data.
  $results = tow_search_inner_within_dataset_callback($nid);

  // Generate new block output.
  $block = tow_search_inner_widgets_block();
  
  // Return a JSON object containing a new content of the block.
  return drupal_json(array(
    'search' => $results, 
    'block' => $block['content']
  ));
}

/**
 * Proxy for outer search using Ajax.
 */
function tow_ajax_handlers_search_outer() {
  $_GET['q'] = str_replace('search_outer_ajax/', '', $_GET['q']);
  $results = tow_search_outer_view();
  $form = drupal_get_form('tow_search_outer_filter_form');
  $number_block = tow_search_outer_number_of_results_block();
  $tabs_block = tow_search_outer_tabs_block();
  return drupal_json(array(
    'results' => $results,
    'form' => $form, 
    'number' => $number_block['content'], 
    'tabs' => $tabs_block['content'],
  ));
} 
