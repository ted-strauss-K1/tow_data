<?php

/**
 * Implementation of hook_menu_default_menu_custom().
 */
function tow_menu_signing_feature_menu_default_menu_custom() {
  $menus = array();

  // Exported menu: menu-signing
  $menus['menu-signing'] = array(
    'menu_name' => 'menu-signing',
    'title' => 'Signing',
    'description' => 'Login / Register links for anonymous users. User and About links for authenticated users.',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Login / Register links for anonymous users. User and About links for authenticated users.');
  t('Signing');


  return $menus;
}
