<?php

/**
 * Implementation of hook_default_fe_block_settings().
 */
function tow_blocks_menus_feature_default_fe_block_settings() {
  $export = array();

  // fusion_theory
  $theme = array();

  $theme['menu-menu-signing'] = array(
    'module' => 'menu',
    'delta' => 'menu-signing',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '0',
    'region' => 'header_top',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '-1',
  );

  $theme['menu-menu-tags'] = array(
    'module' => 'menu',
    'delta' => 'menu-tags',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-27',
    'region' => 'preface_bottom',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'tags*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['menu-menu-users'] = array(
    'module' => 'menu',
    'delta' => 'menu-users',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-25',
    'region' => 'preface_bottom',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'users*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['menu-primary-links'] = array(
    'module' => 'menu',
    'delta' => 'primary-links',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-27',
    'region' => 'header_top',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '-1',
  );

  $theme['menu-secondary-links'] = array(
    'module' => 'menu',
    'delta' => 'secondary-links',
    'theme' => 'fusion_theory',
    'status' => 0,
    'weight' => '-25',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '<front>
newest
popular
featured
debated',
    'title' => '<none>',
    'cache' => '-1',
  );

  $export['fusion_theory'] = $theme;

  $theme_default = variable_get('theme_default', 'garland');
  $themes = list_themes();
  foreach ($export as $theme_key => $settings) {
    if ($theme_key != $theme_default && empty($themes[$theme_key]->status)) {
      unset($export[$theme_key]);
    }
  }
  return $export;
}
