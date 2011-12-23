<?php

// $Id: tow_table_theme.inc,v 1.0.0.0  2011/02/28 15:32:21 tow Exp $

/**
 * @file
 */

/*
 * Theme callback.
 */
function theme_tabular_form($form) {
  $rows = array();
  foreach (element_children($form['data']) as $key) {
    foreach (element_children($form['data'][$key]) as $name) {
      $rows[$key][] = drupal_render($form['data'][$key][$name]);
    }
  }
  return theme('table', $form['header']['#value'], $rows);
}

/*
 * Theme callback.
 */
function theme_sortable_table($header, $rows, $nid, $uid = NULL, $attributes = array(), $caption = NULL) {

  $active = tow_table_headers($header, $nid); 

  // hide first column for all users except table owner 
  global $user;
  if ($user->uid == $uid || user_access('edit any table content')) {
    $header[0]['data'] = 'Action';
  
    foreach($rows as $key => $value) {
      $record_nid = $rows[$key]['data'][0]['data'];
      $rows[$key]['data'][0] = l('edit', 'node/' . $record_nid . '/edit', array('query' => 'destination=table/' . $nid));
      $rows[$key]['data'][0] .= ' ' . l('delete', 'node/' . $record_nid . '/delete', array('query' => 'destination=table/' . $nid));
    }
  }else{
    unset($header[0]);
    foreach($rows as $key => $value) {
      unset($rows[$key]['data'][0]);
    }
  }
  
  // Add sticky headers, if applicable.
  if (count($header)) {
    drupal_add_js('misc/tableheader.js');
    // Add 'sticky-enabled' class to the table to identify it for JS.
    // This is needed to target tables constructed by this function.
    $attributes['class'] = empty($attributes['class']) ? 'sticky-enabled' : ($attributes['class'] .' sticky-enabled');
  }

  $output = '<table'. drupal_attributes($attributes) .">\n";

  if (isset($caption)) {
    $output .= '<caption>'. $caption ."</caption>\n";
  }

  // Format the table header:
  if (count($header)) {
    $ts = tablesort_init($header);
    // HTML requires that the thead tag has tr tags in it followed by tbody
    // tags. Using ternary operator to check and see if we have any rows.
    $output .= (count($rows) ? ' <thead><tr>' : ' <tr>');

    foreach ($header as $cell) {
      //$cell = tablesort_header($cell, $header, $ts);
      $output .= _theme_table_cell($cell, TRUE);
    }
    // Using ternary operator to close the tags based on whether or not there are rows
    $output .= (count($rows) ? " </tr></thead>\n" : "</tr>\n");
  }
  else {
    $ts = array();
  }

  // Format the table rows:
  if (count($rows)) {
    $output .= "<tbody>\n";
    $flip = array('even' => 'odd', 'odd' => 'even');
    $class = 'even';
    foreach ($rows as $number => $row) {
      $attributes = array();

      // Check if we're dealing with a simple or complex row
      if (isset($row['data'])) {
        foreach ($row as $key => $value) {
          if ($key == 'data') {
            $cells = $value;
          }
          else {
            $attributes[$key] = $value;
          }
        }
      }
      else {
        $cells = $row;
      }
      if (count($cells)) {
        // Add odd/even class
        $class = $flip[$class];
        if (isset($attributes['class'])) {
          $attributes['class'] .= ' '. $class;
        }
        else {
          $attributes['class'] = $class;
        }

        // Build row
        $output .= ' <tr'. drupal_attributes($attributes) .'>';
        $i = 0;
        if (isset($active))
          $cells[$active]['class'] = 'active';
        foreach ($cells as $cell) {
          $cell = tablesort_cell($cell, $header, $ts, $i++);
          $output .= _theme_table_cell($cell);
        }
        $output .= " </tr>\n";
      }
    }
    $output .= "</tbody>\n";
  }

  $output .= "</table>\n";
  return $output;
}

function tow_table_headers(&$headers, $nid) {
  $count = 0;
  $titles = $headers;
  foreach ($headers as $field => $header) {
    if (!$header) {
      $offset++;
      continue;
    }
    $headers[$field]['data'] = l($header['data'], 'table/' . $nid, array(
      //'html' => TRUE, 
      'query' => 'order=' . $count . '&sort=asc'
    ));
    $count++;
  }
  
  if (is_numeric($_GET['order']) && (int)$_GET['order'] < $count && (int)$_GET['order'] >= 0 ){
    $order = $_GET['order'];
    $active = $order + $offset;
    if ($_GET['sort'] == 'asc'){
      $headers[$active]['data'] = l($titles[$active]['data'], 'table/' . $nid, array('query' => 'order=' . $order . '&sort=desc')) . '<img width="13" height="13" title="sort descending" alt="sort icon" src="/misc/arrow-desc.png">';
    }  
    elseif ($_GET['sort'] == 'desc'){
      $headers[$active]['data'] = l($titles[$active]['data'], 'table/' . $nid, array('query' => 'order=' . $order . '&sort=asc')) . '<img width="13" height="13" title="sort descending" alt="sort icon" src="/misc/arrow-asc.png">';
    }
    return $active;
  }
  
}