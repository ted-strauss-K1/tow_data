<?php

/**
 * Implementation of hook_menu_default_menu_custom().
 */
function tow_menu_tags_feature_menu_default_menu_custom() {
  $menus = array();

  // Exported menu: menu-tags
  $menus['menu-tags'] = array(
    'menu_name' => 'menu-tags',
    'title' => 'Tags',
    'description' => 'Tags lists.',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Tags');
  t('Tags lists.');


  return $menus;
}
