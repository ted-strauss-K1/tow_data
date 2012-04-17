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
  
  $items['tests/tow_tests_truncate_linked_records'] = array(
    'title' => 'Truncate linked records',
    'access arguments' => array('administer site configuration'),
    'page callback' => 'tow_tests_truncate_linked_records',
    'menu_name' => 'menu-tests',
    'file' => '/tests/tow_tests_truncate_linked_records.inc',
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
  
  $items['tests/switch_table_engine'] = array(
    'title' => 'Switch table engine',
    'access arguments' => array('administer site configuration'),
    'page callback' => 'tow_tests_switch_table_engine',
    'menu_name' => 'menu-tests',
    'file' => '/tests/tow_tests_switch_table_engine.inc',
  );
  
  $items['tests/fk'] = array(
    'title' => 'Foreign keys',
    'access arguments' => array('administer site configuration'),
    'page callback' => 'tow_tests_fk',
    'menu_name' => 'menu-tests',
    'file' => '/tests/tow_tests_fk.inc',
  );
  
  return $items;
}

/**
 * Initializes timer for performance analysis
 */
function tow_timer_init($timer_name, $action) {
  global $timer;
  global $timer_index;
  global $print_timings;
  
  if (!$timer_index) {
    $print_timings[$timer_name] = 1;
    $timer = array();
    $timer_index = 0;
    $timer[$timer_index]['action'] = $action;
    $timer[$timer_index]['time'] = microtime(true);
  }
  else  {
    // when timer is already initialized
    tow_timer_tick($action); 
  }
}

/**
 * Timer tick
 */
function tow_timer_tick($action_finished) {
  global $timer;
  global $timer_index;
  
  if ($timer_index || $timer_index === 0) { 
    $timer[++$timer_index]['action'] = $action_finished;
    $timer[$timer_index]['time'] = microtime(true);
    $timer[$timer_index]['elapsed'] = $timer[$timer_index]['time']-$timer[$timer_index-1]['time'];
  }
}
/**
 * Shows timings of the registered actions
 */
function tow_timer_show_results($timer_name) {
  global $timer;
  global $timer_index;
  global $print_timings;
  
  // print timings only if $timer_name corresponds to the initial name.
  if ($print_timings[$timer_name]) {
 
    $timings = array();
    foreach ($timer as $value) {
      $timings[$value['action']] = $value['elapsed'];
    }
    drupal_set_message("<pre>" . print_r($timings). "</pre>");
    //drupal_set_message("<pre>" . print_r($print_timings). "</pre>");
    
    unset($timings);
    unset($timer);
    unset($timer_index);
    unset($print_timings);
  }
}

function x($variable) {
  drupal_set_message('<pre>' . print_r($variable, TRUE) . '</pre>');
  //dpm($variable);
} 