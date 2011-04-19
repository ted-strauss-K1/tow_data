<?php

// $Id: tow_pma_menu.inc,v 1.0 .0.0 2011/03/17 22:13:21 tow Exp $


/**
 * @return block containing menu links.
 */
function tow_pma_menu_block() {
  
  global $dataset;
  
  if (!$dataset)
    return;
  
  global $table;  
  global $grey_pma_menu;
  
  $items = array(
    'top' => l('Top', 'data'),
    'summary' => l('Summary', 'node/' . $dataset->nid)
  );
  
  if ($table) {
    if ($grey_pma_menu) {
      $items['structure'] = t('Structure');
      $items['browse'] = t('Browse');
    }else {
      $items['structure'] = l('Structure', 'node/' . $table->nid);
      $items['browse'] = l('Browse', 'table/' . $table->nid);
    }
  }else {
    if ($grey_pma_menu) {
      $items['browse'] = t('Browse');
    }else {
      $items['browse'] = l('Browse', 'dataset/' . $dataset->nid);
    }
  }
      
  if ($grey_pma_menu) {
    $items += array(
      'search' => t("Search"), 
      'export' => t("Export"), 
      'import' => t("Import")
    );
  }else {
    $items += array(
      'search' => l('Search', 'search'), 
      'export' => l('Export', 'export'), 
      'import' => l('Import', 'import')
    );
  }
 
  $block['content'] = theme_item_list($items, NULL, 'ul');
  
  tow_pma_breadcrumbs();
  
  return $block;
}

/**
 * Set breadcrumbs
 */
function tow_pma_breadcrumbs() {
  
  global $dataset;
  global $table;
  global $record;
  
  $args = arg();
  
  if (!$dataset->nid) {
    $destination = explode('/', $_GET['destination']);
    $destination = $destination[1];
    if (!is_numeric($destination))
      die();
    $dataset = node_load($destination);
  }
  
  $bc = array();
  $bc[] = l('Top', 'data');
  
  if ($table)
    $bc[] = l($dataset->title, 'dataset/' . $dataset->nid);
  elseif (    ($args[0] == 'node' && $args[1] == 'add' && $args[2] == 'table')     ||     ($args[0] == 'dataset' && $args[2] == 'maintenance')    )
      $bc[] = l($dataset->title, 'dataset/' . $dataset->nid);
    else
      $bc[] = $dataset->title;  
  
  if ($record)
    $bc[] = l($table->title, 'table/' . $table->nid);
  elseif ($table)
    if (    ($args[0] == 'node' && ($args[2] == 'edit' || $args[2] == 'delete'))   ||   ($args[0] == 'table' && $args[2] == 'truncate')    )
      $bc[] = l($table->title, 'table/' . $table->nid);
    else
      $bc[] = $table->title;
  
   global $bc_attachment;
   if ($bc_attachment)   
     $bc[count($bc) -1] .= $bc_attachment;
     
  drupal_set_breadcrumb($bc);
}