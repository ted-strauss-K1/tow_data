<?php

/**
 * @file
 * 
 * Interface between tow.module and views.module.
 * Api documentation is at http://views.doc.logrus.com/
 */

module_load_include('inc', 'tow', 'includes/views/default_views/tow.views.default_views.dataset');
module_load_include('inc', 'tow', 'includes/views/default_views/tow.views.default_views.datasets');
module_load_include('inc', 'tow', 'includes/views/default_views/tow.views.default_views.tags');
module_load_include('inc', 'tow', 'includes/views/default_views/tow.views.default_views.users');



/****************************************************************************************************
 * Views hooks
 ****************************************************************************************************/

/**
 * Implementation of hook_views_default_views().
 */
function tow_views_default_views() {
  $views = array();
  
  // Dataset.
  $dataset = tow_views_default_views_dataset();
  $views[$dataset->name] = $dataset;
  
  // Datasets.
  $datasets = tow_views_default_views_datasets();
  $views[$datasets->name] = $datasets;
  
  // Tags.
  $tags = tow_views_default_views_tags();
  $views[$tags->name] = $tags;

  // Users.
  $users = tow_views_default_views_users();
  $views[$users->name] = $users;

  return $views;
}
