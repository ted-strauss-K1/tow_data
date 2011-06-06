<?php
// $Id: tow_import.inc,v 1.0.0.0 2011/02/28 23:42:15 tow Exp $

/**
 * @file
 * Functions for import
 */

######### FORM #########

function tow_import_form($form_state, $nid) {
  
  drupal_add_js('sites/all/libraries/jquery.filedrop/jquery.filedrop.js');
  drupal_add_js(drupal_get_path('module', 'tow') . '/js/import.js');
  
  $form['#attributes'] = array('enctype' => "multipart/form-data");
  $directory_path = file_directory_path();
  $d= file_check_directory($directory_path, FILE_CREATE_DIRECTORY, 'file_directory_path');
  
  global $dataset;
  $dataset = node_load($nid);
  if ($dataset->type !== 'dataset') {
    drupal_not_found();
    die();
  }
  
  drupal_set_title($dataset->title);
  
  // Extract and show names of tables from the dataset
  $tables_res = db_query("
    SELECT n.nid, n.title 
    FROM {content_type_table} t
    LEFT JOIN {node} n
      ON t.nid = n.nid
    WHERE t.field_dataset_value = %d
    ", $dataset->nid);
  while($table = db_fetch_array($tables_res))
    $tables[$table['nid']] = $table['title'];
      
  // way
  $form['to'] = array(
    '#type' => 'item',
    '#tree' => TRUE,
    '#title' => t('Import files from your data to...'),
  );
  $form['to']['way'] = array(
    '#type' => 'radios',
    '#default_value' => 0,
    '#options' => array(t('Create new table in this dataset'), t('Insert now data into a table')),
  );  
  $form['to']['table'] = array(
    '#type' => 'select',
    '#options' => $tables
  );
  $form['guide'] = array('#type' => 'item', '#value' => l('Learn about importing data', 'import_learn'));
  
  // file
  $form['file'] = array(
    '#type' => 'item',
    //'#tree' => TRUE,
    '#title' => t('Choose files to import (only XLS, TXT & CSV formats accepted)'),
  );
  $form['file']['upload'] = array(
    '#type' => 'file',
    '#size' => 1,
    '#description' => t('or drag your file here')
  );

  $form['file']['or_url'] = array('#type' => 'item', '#value' => t('or URL, where we can grab it'));
  $form['file']['url'] = array('#type' => 'textfield');
  $form['file']['get_it'] = array(
    '#type' => 'item',
    '#prefix' => '<div class="get-it">',
    '#suffix' => '</div>',
    '#value' => t('Get it!')
  );
  
  $rows = array(
    'Uploading',
    'Validating files',
    'Creating content',
  );
  foreach($rows as $i => $row) {
    $rows[$i] = array(
      'class' => $i + 1,
      'data' => array(
        $i + 1 . '.',
        t($row),
        array('class' => 'dots', 'data' => NULL), 
        array('class' => 'status', 'data' => NULL),
      ),
    );
  }
  $messages = array('Warnings', 'Keys');
  foreach ($messages as $message) {
    $i++;
    $rows[] = array(
      'class' => t(strtolower($message)) . ' ' . ($i + 1),
      'data' => array(
        t($message) . ':',
        array('class' => 'message', 'data' => NULL)
      ),
      'style' => 'display: none'
    );
  }  
  $rows[] = array(
    'class' => '6',
    'data' => array(
      '4. ' . t('Done'),
      array('class' => 'message', 'data' => NULL),
      array('class' => 'status', 'data' => NULL),
    ),
    'style' => 'display: none'
  );
  $rows[] = array(
    'class' => '7',
    'data' => array(l('New import', 'import/' . $dataset->nid) . t(' or back to your ') . l('dataset', 'dataset/' . $dataset->nid)    ),
    'style' => 'display: none'
  );
  
  $form['progress'] = array(
    '#prefix' => '<div class="progress-bar" style="display: none"><label class="progress-label">' . t('Progress') . '</label>',
    '#value' => theme('table', array(), $rows, array('class' => 'progress-table')),
    '#suffix' => '</div>'
  );
  
  $form['#validate'][] = 'tow_import_form_validate';
  
  $form['submit'] = array('#type' => 'submit', '#value' => 'Submit');
  
  return $form;
}

function tow_import_form_validate($form, &$form_state) {

}

function tow_import_form_submit($form, &$form_state) {
  $file = file_save_upload('upload');
  if ($file = file_save_upload('upload')) {
    time();
  }  
}

######### MESSAGING INTERFACE #########

function tow_import_progress_init($hash, $copied, $filename, $to) {
  global $user;
        
  db_query("INSERT INTO {tow_import_progress} (sid, hash) VALUES('%s', %d)", $user->sid, $hash);
  
  if ($copied) {
    tow_import_progress_set($hash, 1, 10);
    _test($hash, $filename, $to);
  }
  else {
    tow_import_progress_set($hash, 1, -1);
    tow_import_progress_set($hash, 4, -1); // stop
  }
 
  sleep(10);
  tow_import_progress_delete($hash);
  
}

function tow_import_progress_set($hash, $stage, $value) {
  global $user;
  db_query("UPDATE {tow_import_progress} SET stage%s = '%s' WHERE sid='%s' AND hash = %d", $stage, $value, $user->sid, $hash);
}

function tow_import_progress_get($hash, $stage) {
  global $user;
  
  $response = db_result(db_query("SELECT SQL_NO_CACHE stage%s FROM {tow_import_progress} WHERE sid='%s' AND hash=%d", $stage, $user->sid, $hash));
  
  if ($stage == 6)
    tow_import_progress_delete($hash);
    
  return drupal_json(array('status' => TRUE, 'response' => $response));  
}

function tow_import_progress_delete($hash) {
  global $user;
  db_query("DELETE FROM {tow_import_progress} WHERE sid='%s' AND hash = %d", $user->sid, $hash);
}

####### FILE TRANSFER INTERFACE ######

function tow_import_dragged() {

  $hash = $_POST['hash'];
  $dataset_nid = $_POST['dataset'];
  $table_nid = $_POST['table'];

  $to = tow_import_check_request($hash, $dataset_nid, $table_nid);  
  tow_import_upload($hash, $to);
  
}

function tow_import_selected($hash, $dataset_nid, $table_nid) {

  $to = tow_import_check_request($hash, $dataset_nid, $table_nid);
  tow_import_upload($hash, $to);
  
}

function tow_import_grab($hash, $dataset_nid, $table_nid) {
  
  $to = tow_import_check_request($hash, $dataset_nid, $table_nid);
  
  $grab_url = $_GET['grab_url'];
  $file = file_get_contents($grab_url);
  
  if ($file) {
  	file_put_contents('sites/default/files/import/' . $hash, $file);
  	tow_import_progress_init($hash, TRUE, $grab_url, $to);
  }
  else
    tow_import_progress_init($hash, FALSE, $grab_url, NULL, NULL);
  
}

function tow_import_upload($hash, $to) {
  
  foreach ($_FILES['files'] as $key => $value)
    $_FILES['files'][$key] = array('upload' => $value);
      
  $file = file_save_upload('upload');
  $filename = $file->filename;
  $copied = file_copy($file, 'import/' . $hash);
  
  tow_import_progress_init($hash, $copied, $filename, $to);
}

function tow_import_check_request($hash, $dataset_nid, $table_nid) {
  
  if ((int)$hash == $hash && $hash) {
  	$dataset = new stdClass;
  	$dataset->nid = $dataset_nid;
    if ((int)$dataset->nid == $dataset->nid && $dataset->nid) {
      $table = new stdClass;
      $table->nid = $table_nid;
      if ($table->nid != 'false') {
      	;
      	if ((int)$table->nid != $table->nid) {
      	  die();
      	}
      }
      else {
      	unset($table);
      }        	
    } 
    else {
      die();
    }
  } 
  else {
  	die();
  }
  
  global $user;
  $dataset->uid = db_result(db_query("SELECT uid FROM {node} WHERE nid=%d", $dataset->nid));
  if ($user->uid != $dataset->uid && !user_access('edit any table content'))
    die();
  
  if ($table) {
  	$table->field_dataset_value = db_result(db_query("SELECT field_dataset_value FROM {content_type_table} WHERE nid=%d", $table->nid));
  	if ($table->field_dataset_value != $dataset->nid)
      die();
  }
  else
    $table = FALSE;

  return array('dataset' => $dataset, 'table' => $table);
}

###### FILE PROCESSING ######

function _test($hash, $filename, $to) {
  
  $file = file_get_contents('sites/default/files/import/' . $hash);
  time();
  file_delete('sites/default/files/import/' . $hash);
  
  // PROCESS
  for ($stage = 2; $stage <= 2; $stage++) {
    for ($value = 1; $value <= 10; $value++) {
      usleep(500000);
      tow_import_progress_set($hash, $stage, $value);
    }
  }
  
  tow_import_progress_set($hash, 3, -1);
  
  sleep(1);
  tow_import_progress_set($hash, 4, /*'Some content was shortened'*/ -1);
  
  usleep(1);
  tow_import_progress_set($hash, 5, 'We found 2 keys');
  
  usleep(1);
  tow_import_progress_set($hash, 6, '4. Done - booklist table');
  
}