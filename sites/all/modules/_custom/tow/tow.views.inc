<?php
// $Id: tow.views.inc,v 1.0 .0.0 2011/02/28 15:32:21 tow Exp $

/**
 * @file
 * Interface between tow.module and views.module.
 * Api documentation is at http://views.doc.logrus.com/
 */

/**
 * Implementation of hook_views_pre_render().
 */
function tow_views_pre_render(&$view) {
  global $node_for_menu;
  global $node;
  if ($view->name == 'dataset') {
    $node = node_load($view->args[0]);
    $node_for_menu = $node; 
    if ($node->type !== 'dataset') {
      drupal_not_found();
      die();
    }
    $view->attachment_before = $node->title;
    $view->attachment_before .= "<div>" . $node->teaser . "</div>";
  }
 
}

/*
 * To dataset view add row displaying amount of tables, sum of records and overall size
 */
function tow_preprocess_views_view_table(&$vars) {
  if ($vars['view']->name == 'dataset') {

    $count = count($vars['rows']);

    $vars['class'] = 'views-table cols-' . strval($count + 1);

    
    unset($vars['row_classes'][$count - 1][1]);
    
    if ($vars['row_classes'][$count - 1][0] == 'even')
      $vars['row_classes'][][] = 'odd';
    else
      $vars['row_classes'][][] = 'even';

    $vars['row_classes'][$count][] = 'views-row-last';
    $vars['row_classes'][$count][] = 'dataset-total';
    
    foreach ($vars['rows'] as $row) {
      $count_records += $row['phpcode'];      
      $count_bytes += strpos($row['phpcode_1'], 'KiB') ? floatval($row['phpcode_1']) * 1024 : floatval($row['phpcode_1']);
    } 
    
    if ($count_bytes <= 1024)
      $count_bytes .= ' B';
    else
      $count_bytes = round($count_bytes*10/1024)/10 . ' KiB';
    
    if ($count == 1)
      $table_label = t('table');
    else
      $table_label = t('tables');
      
    $vars['rows'][] = array(
      'title' => $count . ' ' . $table_label,
      'phpcode_2' => '',
      'phpcode' => $count_records,
      'phpcode_1' => $count_bytes,
    );
  }
}