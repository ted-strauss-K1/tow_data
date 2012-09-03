<?php

/**
 * @file
 * 
 * Primary menu representation.
 */



/****************************************************************************************************
 * Hook handlers
 ****************************************************************************************************/

/**
 * Returns block containing menu links.
 */
function tow_pma_menu_block() {

  global $_tow_dataset;
  global $_tow_table;
  global $_tow_grey_pma_menu;

  // Stop if no dataset is set.
  if (!$_tow_dataset) {
    return;
  }

  // Define menu items.
  $items = array();
  $items['summary'] = l('Overview', 'node/' . $_tow_dataset->nid);
  $items['search'] = ($_tow_grey_pma_menu) ? t('Search') : l('Search', 'search_dataset/' . $_tow_dataset->nid);
  $items['browse'] = ($_tow_grey_pma_menu) ? t('Browse') : (($_tow_table) ? l('Browse', 'table/' . $_tow_table->nid) : l('Browse', 'dataset/' . $_tow_dataset->nid));
  $items['docs'] = l('Docs', 'docs');
  $items['forum'] = l('Forum', 'forum');

  // Theme the block.
  $block['content'] = theme_item_list($items, NULL, 'ul');

  // Set breadcrumbs.
  tow_pma_menu_breadcrumbs();

  return $block;
}



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Set breadcrumbs.
 */
function tow_pma_menu_breadcrumbs() {

  global $_tow_dataset;
  global $_tow_table;
  global $_tow_record;

  $args = arg();

  // Load dataset or die.
  if (!$_tow_dataset->nid) {
    $destination = explode('/', $_GET['destination']);
    $destination = $destination[1];
    if (!is_numeric($destination)) {
      die();
    }
    $_tow_dataset = node_load($destination);
  }

  $bc = array();

  // Select taxonomy term corresponding to dataset category.
  if ($_tow_dataset->taxonomy) {
    foreach ($_tow_dataset->taxonomy as $term_obj) {
      $q = db_result(db_query("SELECT v.vid AS vid FROM {vocabulary} v WHERE v.name = '%s'", 'Dataset categories'));
      if ($term_obj->vid == $q) {
        $bc[] = l($term_obj->name, 'taxonomy/term/' . $term_obj->tid);
          break;
      }
    }
  }
  $bc[] = $_tow_dataset->title;

  // Extend breadcrumbs.
  global $_tow_bc_attachment;
  if ($_tow_bc_attachment) {
    $bc[count($bc) - 1] .= $_tow_bc_attachment;
  }

  drupal_set_breadcrumb($bc);
}
