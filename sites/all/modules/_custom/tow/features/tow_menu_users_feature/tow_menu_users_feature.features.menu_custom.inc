<?php

/**
 * Implementation of hook_menu_default_menu_custom().
 */
function tow_menu_users_feature_menu_default_menu_custom() {
  $menus = array();

  // Exported menu: menu-users
  $menus['menu-users'] = array(
    'menu_name' => 'menu-users',
    'title' => 'Users',
    'description' => 'User lists.',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('User lists.');
  t('Users');


  return $menus;
}
