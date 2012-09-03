<?php

/**
 * @file
 * 
 * Interface between tow.module and views.module.
 * Api documentation is at http://views.doc.logrus.com/
 */

module_load_include('inc', 'tow', 'includes/views/tow.views.default_views');



/****************************************************************************************************
 * Views hooks
 ****************************************************************************************************/

/**
 * Implementation of hook_views_data().
 */
function tow_views_data() {
  
  
  // User 'Top Taxonomy' field.
  $data['users']['top_taxonomy'] = array(
    'title' => t('Top Taxonomy'), 
    'help' => t('User\'s most usable taxonomy terms. The amount of terms and vocabularies can be specified.'),
    'field' => array(
      'handler' => 'tow_views_handler_field_top_taxonomy',
    ), 
  );
  
  // User 'Top Taxonomy' filter.
  $data['users']['creation_interval'] = array(
    'title' => t('Creation Interval'), 
    'help' => t('User\'s creation interval.'),
    'filter' => array(
      'handler' => 'tow_views_handler_filter_creation_interval',
    ), 
  );
  
  // User 'Content Count' sorting.
  $data['users']['content_count'] = array(
    'title' => t('Content Count'), 
    'help' => t('User\'s content amount.'),
    'sort' => array(
      'handler' => 'tow_views_handler_sort_content_count',
    ), 
  );
  
  // User 'Vote Count' sorting.
  $data['users']['vote_count'] = array(
    'title' => t('Vote Count'), 
    'help' => t('User\'s votes amount.'),
    'sort' => array(
      'handler' => 'tow_views_handler_sort_vote_count',
    ), 
  );
  
  return $data;
}

/**
 * Implementation of hook_views_handlers().
 */
function tow_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'tow') . '/includes/views/handlers',
    ),
    'handlers' => array(
      'tow_views_handler_field_top_taxonomy' => array(
        'parent' => 'views_handler_field',
      ),
      'tow_views_handler_filter_creation_interval' => array(
        'parent' => 'views_handler_filter',
      ),
      'tow_views_handler_sort_content_count' => array(
        'parent' => 'views_handler_sort',
      ),
      'tow_views_handler_sort_vote_count' => array(
        'parent' => 'views_handler_sort',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_query_alter().
 */
function tow_views_query_alter(&$view, &$query) {
  
  // Alter the query for the dataset view.
  if ($view->name == 'dataset') {
    time();
    
    // Table queue.
    $query->table_queue['content_type_table'] = array(
      'table' => 'content_type_table',
      'alias' => 'ctt',
      'join' => new views_join,
      'relationship' => 'node' 
    );
    $query->table_queue['content_type_table']['join']->construct();
    $query->table_queue['content_type_table']['join']->table = 'content_type_table';
    $query->table_queue['content_type_table']['join']->left_table = 'node';
    $query->table_queue['content_type_table']['join']->left_field = 'nid';
    $query->table_queue['content_type_table']['join']->field = 'nid';
    $query->table_queue['content_type_table']['join']->type = 'LEFT';
    
    // Where.
    $query->where[0]['clauses'][2] = 'ctt.field_dataset_value=%d';
    $query->where[0]['args'][1] = $view->args[0];
  }
}

/**
 * Implementation of hook_views_pre_render().
 */
function tow_views_pre_render(&$view) {
  
  // Dataset view.
  if ($view->name == 'dataset') {
    
    // Load dataset node.
    $node = node_load($view->args[0]); 
    
    // Set HTTP 404 error if the node is not a dataset.
    if ($node->type !== 'dataset') {
      drupal_not_found();
      die();
    }
    
    // Save necessary parametres only.
    global $_tow_dataset;
    $_tow_dataset = (object)array('nid' => $node->nid, 'title' => $node->title, 'uid' => $node->uid);
    
    // Attachments.
    $view->attachment_before = tow_views_dataset_attachment($view, $node);
  }
}



/****************************************************************************************************
 * Helper functions
 ****************************************************************************************************/

/**
 * Prepare dataset view attachment: description, taxonomy with edit links for dataset owner.
 */
function tow_views_dataset_attachment($view, $node) {
  
  // Store in global scope text like "( 5 tables)".
  global $_tow_bc_attachment;
  $count = count($view->result);
  $_tow_bc_attachment = ' (' . $count;
  if ($count == 1) {
    $_tow_bc_attachment .= t( ' table') . ')';
  }
  else {
    $_tow_bc_attachment .= t( ' tables') . ')';
  }
  
  // Description.
  global $user;
  $description = $node->teaser . ' ' . l('more', 'node/' . $node->nid);  
  if ($user->uid == $node->uid  || user_access('edit any dataset')) {
    $edit_description = l('edit description', 'node/' . $node->nid . '/edit');
  }
  
  // Taxonomy.
  if (is_array($node->taxonomy) && !empty($node->taxonomy)) {
    foreach ($node->taxonomy as $item) {
      $categories[] = $item->name;
    }
    $categories = implode(', ', $categories);
    if ($user->uid == $node->uid) {
      $edit_categories = l('edit categories', 'node/' . $node->nid . '/edit');
    }
  }
  elseif ($user->uid == $node->uid) {
    $edit_categories = t('This dataset does not have any categories. ') . l('Add categories', 'node/' . $node->nid . '/edit');
  }
  
  // Theme the output.
  return theme('tow_theme_dataset_attachment', $description, $edit_description, $categories, $edit_categories);
}

/*
 * To dataset view add row displaying amount of tables, sum of records and overall size.
 */
function tow_preprocess_views_view_table(&$vars) {
  
  // Dataset view.
  if ($vars['view']->name == 'dataset') {

    // Row count.
    $count = count($vars['rows']);

    // Prepare classes.
    $vars['class'] = 'views-table cols-' . strval($count + 1);
    unset($vars['row_classes'][$count - 1][1]);
    if ($vars['row_classes'][$count - 1][0] == 'even') {
      $vars['row_classes'][][] = 'odd';
    }
    else {
      $vars['row_classes'][][] = 'even';
    }
    $vars['row_classes'][$count][] = 'views-row-last';
    $vars['row_classes'][$count][] = 'dataset-total';
    
    foreach ($vars['rows'] as $row) {
      $count_records += $row['phpcode'];
    } 
    
    if ($count == 1) {
      $table_label = t('table');
    }
    else {
      $table_label = t('tables');
    }
    
    // Rows.
    $vars['rows'][] = array(
      'title' => $count . ' ' . $table_label,
      'phpcode_2' => '',
      'phpcode_1' => $count_bytes,
    );
  }
}