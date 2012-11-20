<?php

/**
 * Implementation of hook_default_fe_block_settings().
 */
function tow_blocks_feature_default_fe_block_settings() {
  $export = array();

  // fusion_theory
  $theme = array();

  $theme['tow-create_table'] = array(
    'module' => 'tow',
    'delta' => 'create_table',
    'theme' => 'fusion_theory',
    'status' => 0,
    'weight' => '-21',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-dataset_badge'] = array(
    'module' => 'tow',
    'delta' => 'dataset_badge',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-26',
    'region' => 'sidebar_last',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-dataset_share'] = array(
    'module' => 'tow',
    'delta' => 'dataset_share',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-25',
    'region' => 'sidebar_last',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-dataset_stats'] = array(
    'module' => 'tow',
    'delta' => 'dataset_stats',
    'theme' => 'fusion_theory',
    'status' => 0,
    'weight' => '-20',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-dataset_votes'] = array(
    'module' => 'tow',
    'delta' => 'dataset_votes',
    'theme' => 'fusion_theory',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-pma_menu'] = array(
    'module' => 'tow',
    'delta' => 'pma_menu',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-27',
    'region' => 'preface_bottom',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => 'node/add/dataset',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-saved_searches_description'] = array(
    'module' => 'tow',
    'delta' => 'saved_searches_description',
    'theme' => 'fusion_theory',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-saved_searches_list'] = array(
    'module' => 'tow',
    'delta' => 'saved_searches_list',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-28',
    'region' => 'content_top',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '2',
    'pages' => '<?php  
        if (arg(0) != "search_dataset") {
          return FALSE;
        }
        else {
          $nid = arg(1);
          if (!is_numeric($nid)) {
            return TRUE;
          }
          else {
            $node = node_load($nid);
            return (($node->type == "dataset") ? FALSE : TRUE);
          }
        }
      ?>',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-saved_searches_save_search'] = array(
    'module' => 'tow',
    'delta' => 'saved_searches_save_search',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-28',
    'region' => 'content_top',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'search_dataset/*',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-search_inner_facets'] = array(
    'module' => 'tow',
    'delta' => 'search_inner_facets',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-24',
    'region' => 'sidebar_last',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'search_dataset/*',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-search_inner_field_list'] = array(
    'module' => 'tow',
    'delta' => 'search_inner_field_list',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-27',
    'region' => 'content_top',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '2',
    'pages' => '<?php  
        if (arg(0) != "search_dataset") {
          return FALSE;
        }
        else {
          $nid = arg(1);
          if (!is_numeric($nid)) {
            return FALSE;
          }
          else {
            $node = node_load($nid);
            return (($node->type == "dataset") ? TRUE : FALSE);
          }
        }
      ?>',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-search_inner_list_of_tables'] = array(
    'module' => 'tow',
    'delta' => 'search_inner_list_of_tables',
    'theme' => 'fusion_theory',
    'status' => 0,
    'weight' => '-26',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'search_dataset/*',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-search_inner_simple_search'] = array(
    'module' => 'tow',
    'delta' => 'search_inner_simple_search',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-26',
    'region' => 'content_top',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'search_dataset/*',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-search_outer_filter'] = array(
    'module' => 'tow',
    'delta' => 'search_outer_filter',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-27',
    'region' => 'sidebar_last',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => '<front>
newest
popular
featured
debated
newest/*
popular/*
featured/*
debated/*',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-search_outer_number_of_results'] = array(
    'module' => 'tow',
    'delta' => 'search_outer_number_of_results',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-28',
    'region' => 'sidebar_last',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => '<front>
newest
popular
featured
debated
newest/*
popular/*
featured/*
debated/*',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-search_outer_results'] = array(
    'module' => 'tow',
    'delta' => 'search_outer_results',
    'theme' => 'fusion_theory',
    'status' => 0,
    'weight' => '-5',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => '<front>',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-search_outer_searchinput'] = array(
    'module' => 'tow',
    'delta' => 'search_outer_searchinput',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-24',
    'region' => 'header_top',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-search_outer_sponsored'] = array(
    'module' => 'tow',
    'delta' => 'search_outer_sponsored',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-28',
    'region' => 'sidebar_last',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => '<front>
newest
popular
featured
debated',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-search_outer_tabs'] = array(
    'module' => 'tow',
    'delta' => 'search_outer_tabs',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '0',
    'region' => 'preface_top',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => '<front>
newest
popular
featured
debated',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-table_display_switch'] = array(
    'module' => 'tow',
    'delta' => 'table_display_switch',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-25',
    'region' => 'content_top',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '1',
  );

  $theme['tow-user_view'] = array(
    'module' => 'tow',
    'delta' => 'user_view',
    'theme' => 'fusion_theory',
    'status' => '1',
    'weight' => '-26',
    'region' => 'preface_bottom',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'user/*',
    'title' => '',
    'cache' => '1',
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
