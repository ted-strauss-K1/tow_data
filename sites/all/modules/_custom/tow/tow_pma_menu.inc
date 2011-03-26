<?php

// $Id: tow_pma_menu.inc,v 1.0 .0.0 2011/03/17 22:13:21 tow Exp $


/**
 * Create menu
 */
function tow_pma_menu_block() {

  $menu_items = array('summary' => t("Summary"), 'browse' => t("Browse"), 
    'search' => t("Search"), 'export' => t("Export"), 'import' => t("Import"));
  $links = tow_pma_menu_links();
  
  // TODO: check if the number of links is equal to the number of menu items  
  foreach($links as $key => $link) {
    $items[] = l($menu_items[$key], $link);
  }

  //$block['subject'] = t('PMA menu'); 
  $block['content'] = theme_item_list($items, NULL, 'ul');
  
  return $block;
}

/**
 * Generates array of links for menu
 */
function tow_pma_menu_links() {
  $links = array();
  
  global $node_for_menu;

  $page_type = $node_for_menu->type;
 
  switch ($page_type) {
    case 'dataset':
      $dataset_nid = $node_for_menu->nid;
      
      $links['summary'] = "node/$dataset_nid";
      // TODO: make inactive
      $links['browse'] = "dataset/$dataset_nid";
      // TODO: input corresponding menu items
      $links['search'] =  "search/";
      $links['export'] =  "export/$dataset_nid";
      $links['import'] = "import/$dataset_nid";
      break;
    case 'table':
      $table_nid = $node_for_menu->nid;
      $dataset_nid = $node_for_menu->field_dataset['0']['nid'];

      $links['summary'] = "node/$dataset_nid";
      // TODO: make inactive
      $links['browse'] = "table/$table_nid";
      // TODO: input corresponding menu items
      $links['search'] =  "search/";
      $links['export'] =  "export/$dataset_nid";
      $links['import'] = "import/$dataset_nid";
      break;
    case 'record':
           
      $record_nid = $node_for_menu->nid;
      $table_nid = $node_for_menu->field_table['0']['nid'];
      global $dataset_nid;

      // TODO: decide if dataset id is required for record page menu construction 
      // TODO: decide which links are required

      $links['summary'] = "node/$dataset_nid";
      // TODO: make inactive
      $links['browse'] = "table/$table_nid";
      // TODO: input corresponding menu items
      $links['search'] =  "search/";
      $links['export'] =  "export/$table_nid";
      $links['import'] = "import/$table_nid";
      break;
  }
  return $links;
}
 