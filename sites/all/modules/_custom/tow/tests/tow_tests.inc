<?php
/** 
 *@file
 * Testing interface
 */

/**
 * @return array of menu items.
 * Each item for single test.
 * Items will be used inside hook_menu implementation.
 */
function tow_tests_menu() {
  $items = array();
  
  $items['tests/tow_tests_truncate_cck'] = array(
    'title' => 'Truncate cck tables',
    'access arguments' => array('administer site configuration'),
    'page callback' => 'tow_tests_truncate_cck',
    'menu_name' => 'menu-tests',
    'file' => '/tests/tow_tests_truncate_cck.inc',
  );
  
  $items['tests/tow_tests_truncate_nodes'] = array(
    'title' => 'Truncate nodes',
    'access arguments' => array('administer site configuration'),
    'page callback' => 'tow_tests_truncate_nodes',
    'menu_name' => 'menu-tests',
    'file' => '/tests/tow_tests_truncate_nodes.inc',
  );
  
   $items['tests/tow_tests_generate_data'] = array(
    'title' => 'Generate Data',
    'access arguments' => array('administer site configuration'),
    'page callback' => 'tow_tests_generate_data',
    'menu_name' => 'menu-tests',
    'file' => '/tests/tow_tests_generate_data.inc',
  );
  
  return $items;
}
