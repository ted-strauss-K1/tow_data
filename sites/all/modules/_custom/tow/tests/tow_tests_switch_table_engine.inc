<?php

//include_once('./' . drupal_get_path('module', 'tow') . '/tests/tow_tests_truncate_cck.inc');

function tow_tests_switch_table_engine() {
  return drupal_get_form('tow_tests_switch_table_engine_form');  
}

function tow_tests_switch_table_engine_form(){
  $res = db_query('SHOW TABLE STATUS');
  $t = db_fetch_array($res);
  if ($t['Engine'] == "MyISAM") 
    $ops = array('InnoDB', 'MyISAM');
  else
    $ops = array('MyISAM', 'InnoDB');
      
  $form['table_engine'] = array(
      '#type' => 'select',
      '#options' => $ops,
      '#description' => t('Choose new table engine'),
      /*'#size' => 5,
      '#maxlength' => 20,
  	  '#default_value' => 2, */
  );
  
  $form['Switch'] = array(
    '#type' => 'submit',
    '#value' => 'Switch',
  );
  return $form;
}

function tow_tests_switch_table_engine_form_submit($form, &$form_state){
  $time_start = time();
  
  $new_type = $form['table_engine']['#options'][$form_state['values']['table_engine']];
  
  $res = db_query('SHOW TABLE STATUS');
  while ($t = db_fetch_array($res))
    if ($t['Engine'] != $new_type)  
      db_query("ALTER TABLE %s ENGINE = %s", $t['Name'], $new_type);
  
  drupal_set_message("Success! Table engines were switched to " . $new_type);

  $elapsed_time = time() - $time_start;
  drupal_set_message("Elapsed time: " . $elapsed_time . " seconds.");
  //drupal_goto('<front>');

}