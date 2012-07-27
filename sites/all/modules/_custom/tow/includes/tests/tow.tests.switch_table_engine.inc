<?php

/**
 * @file
 * 
 * Switch table engine testing.
 */



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Page callback.
 */
function tow_tests_switch_table_engine() {
  return drupal_get_form('tow_tests_switch_table_engine_form');  
}

/**
 * Form.
 */
function tow_tests_switch_table_engine_form() {
  
  // Set options.
  $res = db_query('SHOW TABLE STATUS');
  $t = db_fetch_array($res);
  if ($t['Engine'] == "MyISAM") {
    $ops = array('InnoDB', 'MyISAM');
  }
  else {
    $ops = array('MyISAM', 'InnoDB');
  }
  
  // Form.
  $form['table_engine'] = array(
    '#type' => 'select',
    '#options' => $ops,
    '#description' => t('Choose new table engine'),
  );

  $form['Switch'] = array(
    '#type' => 'submit',
    '#value' => 'Switch',
  );
  
  return $form;
}

/**
 * Submission callback.
 */
function tow_tests_switch_table_engine_form_submit($form, &$form_state) {
  $time_start = time();
  
  // New type.
  $new_type = $form['table_engine']['#options'][$form_state['values']['table_engine']];
  
  // Switch engine.
  $res = db_query('SHOW TABLE STATUS');
  while ($t = db_fetch_array($res)) {
    if ($t['Engine'] != $new_type) {
      db_query("ALTER TABLE %s ENGINE = %s", $t['Name'], $new_type);
    }
  }
  
  // Messages.
  drupal_set_message(filter_xss("Success! Table engines were switched to " . $new_type));
  $elapsed_time = time() - $time_start;
  drupal_set_message(filter_xss("Elapsed time: " . $elapsed_time . " seconds."));
}