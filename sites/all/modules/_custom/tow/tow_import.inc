<?php
// $Id: tow_import.inc,v 1.0.0.0 2011/02/28 23:42:15 tow Exp $

/**
 * 
 * Part of impelementation of hook_cron. Clears old data from database table tow_import_progress.
 */

function tow_import_cron() {
	db_query("DELETE FROM {tow_import_progress} WHERE NOW() > ADDTIME(`timestamp`, '1:00:00')");
}

/**
 * @file
 * Functions for import
 */

######### FORM #########

function tow_import_form($form_state, $nid) {

  global $dataset;
  $dataset = node_load($nid);
  if ($dataset->type !== 'dataset') {
    drupal_not_found();
    die();
  }
  
  drupal_add_js('sites/all/libraries/jquery.filedrop/jquery.filedrop.js');
  drupal_add_js(drupal_get_path('module', 'tow') . '/js/import.js');

  drupal_set_title($dataset->title);
  
  $form['#attributes'] = array('enctype' => "multipart/form-data");
  $directory_path = file_directory_path();
  $d= file_check_directory($directory_path, FILE_CREATE_DIRECTORY, 'file_directory_path');

  $form['dataset'] = array(
    '#type' => 'hidden',
    '#value' => $nid,
  );

  $form['hash'] = array(
    '#type' => 'hidden',
    '#value' => (isset($form_state['values']['hash'])?$form_state['values']['hash']:(isset($form_state['post']['hash'])?$form_state['post']['hash']:0)),
  );

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
    '#options' => tow_table_list($nid)
  );
  $form['guide'] = array('#type' => 'item', '#value' => l('Learn about importing data', 'import_learn'));

  // file
  $form['file'] = array(
    '#type' => 'item',
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
  if ($file) {
	$hash = $form_state['values']['hash'];
	$form_state['rebuild'] = TRUE;
	$table_nid = ($form_state['values']['to']['way'] == '0') ? 'false' : $form_state['values']['to']['table'];
	$dataset_nid = $form_state['values']['dataset'];
	$to = tow_import_check_request($hash, $dataset_nid, $table_nid);
	$filename = $file->filename;
	$copied = file_copy($file, 'import/' . $hash);

	tow_import_progress_init($hash, $copied, $filename, $to);
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
  //tow_import_progress_delete($hash);

}

function tow_import_progress_set($hash, $stage, $value) {
  global $user;
  db_query("UPDATE {tow_import_progress} SET stage%s = '%s' WHERE sid='%s' AND hash = %d", $stage, $value, $user->sid, $hash);
  watchdog('import','Stage ' . $stage . ' set: ' . $value . '(hash ' . $hash . ')');
}

function tow_import_progress_get($hash, $stage) {
  global $user;

  $response = db_result(db_query("SELECT SQL_NO_CACHE stage%s FROM {tow_import_progress} WHERE sid='%s' AND hash=%d", $stage, $user->sid, $hash));
  watchdog('import','Stage ' . $stage . ' get: ' . $response . '(hash ' . $hash . ')');

  if ($stage == 6)
    tow_import_progress_delete($hash);

  return drupal_json(array('status' => TRUE, 'response' => $response));
}

function tow_import_progress($hash, $stage, $action = 'increase', $total_stages = 10) {
  static $state;
  static $update_step;
  switch ($action) {
    case 'set': 
      if ($state >= 0) {
        $state = 0;
        $update_step = 10 / $total_stages;
      }
      break;
    case 'increase':
      $state += $update_step;
      break;
    case 'finish':
      if ($state >= 0) {
        $state = 10;
        $update_step = 0;
      }      
       break;
    case 'fail':
      $state = -1;
      $update_step = 0;
      break;
    case 'message':
      tow_import_progress_set($hash, $stage, $total_stages);    
      return;
  }
  tow_import_progress_set($hash, $stage, round($state));
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
	watchdog('import','File ' . $filename. ' uploaded');
  $copied = file_copy($file, 'import/' . $hash);
	watchdog('import','File ' . $filename. ' copied with result:' . $copied);

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

  require_once 'sites/all/libraries/PHPExcelExtension/PHPExcel/IOFactory.php';

  // Default column title, used in case if data file contains no column title
  $default_column_title = "Column_";

  // File to be imported
  $imported_file_name = 'sites/default/files/import/' . $hash;
  $info = pathinfo($filename);
  
  // TODO: dataset nid
  $dataset_nid = $to['dataset']->nid;

  // TODO: User id
  $uid = $to['dataset']->uid;

  if ($to['table']) {
    // Action to be done during import: create or append
    $import_action = "append";
    // TODO: table nid
    $table_nid = $to['table']->nid;
  }
  else {
    $import_action = "create";
  }

  $warning_messages = array();

  // PROCESS

  // 2. Data reading and validating
  $stage = 2;
   
  tow_import_progress($hash, $stage, 'set', 5);

  /* PHPExcel_IOFactory data reading engine */
  error_reporting(E_ALL);

  // 2.1 Reading files
  // 2.1.1 Automatically detect filetype and load it into the $objPHPExcel object
  // 2.1.1.1 Detect filetype
  
  $inputFileType = PHPExcel_IOFactory::identify("$imported_file_name");
  // 2.1.1.2 Create reader of detected type
  $objReader = PHPExcel_IOFactory::createReader("$inputFileType");
  // 2.1.1.3 Set properties of Reader object depending on its type    
  // Read no formatting 
  if (method_exists($objReader, 'setReadDataOnly')) {
    $objReader->setReadDataOnly(true);
  }
  // txt-file. Value delimiter may be tab or comma. 
  if (($inputFileType == 'CSV') && (strtolower($info['extension']) == "txt")) {
    
    // read first line
    $file = fopen($imported_file_name, 'r');
    $first_row = fgets($file);
    fclose($file);
    // Determine value delimiter
    if (!strpos($first_row, ',')) {
      $objReader->setDelimiter("\t");  
    }
    elseif (!strpos($first_row, "\t")) {
      $objReader->setDelimiter(",");
    }
    else {
      // default value
      $objReader->setDelimiter("\t");
    }
    
    // Set value enclosure 
    $objReader->setEnclosure('');

	$objPHPExcel = $objReader->load("$imported_file_name");

	} else {
		$objPHPExcel = PHPExcel_IOFactory::load("$imported_file_name");
	}
  

  
  // check, if first row contains more than 
  
  
  // Working with active sheet
  $worksheet_index = 0;
  $data = array();

  foreach ($objPHPExcel->getWorksheetIterator() as $worksheet) {
    if ($import_action == 'append' && $worksheet_index) {

      $warning_messages[] = '0nly first sheet will be inserted into table';
      break;
    }

    // new table titles
    $data['table_data'][$worksheet_index]['table_title'] =  trim($worksheet->getTitle());
    $data['table_data'][$worksheet_index]['record_cols_qty'] = array();
    $data['table_data'][$worksheet_index]['str_value'] = array();
    $data['table_data'][$worksheet_index]['column_titles'] = array();
    //$data_read = false;
    $row_index = 0;
    foreach ($worksheet->getRowIterator() as $row) {
      $cellIterator = $row->getCellIterator();
      $cellIterator->setIterateOnlyExistingCells(false); // loop through empty cells as well
      if ( ($row->getRowIndex() == 1)  && ($import_action == 'create')) {
        foreach ($cellIterator as $cell) {
			$isDateTime=PHPExcel_Shared_Date::isDateTime($cell);
			if ($isDateTime) {  
				$timestamp = PHPExcel_Shared_Date::ExcelToPHP($cell->getValue());
				echo $cellvalue = date("Y-m-d H:i:s",$timestamp - date('Z', $timestamp));
			} else {
				echo $cellvalue = $cell->getValue();
			}
          $data['table_data'][$worksheet_index]['column_titles'][] =  $cellvalue;
        }
      }
      else {
        foreach ($cellIterator as $cell) {
			$isDateTime=PHPExcel_Shared_Date::isDateTime($cell);
			if ($isDateTime) {  
				$timestamp = PHPExcel_Shared_Date::ExcelToPHP($cell->getValue());
				echo $cellvalue = date("Y-m-d H:i:s",$timestamp - date('Z', $timestamp));
			} else {
				echo $cellvalue = $cell->getValue();
			}
          $data['table_data'][$worksheet_index]['str_value'][$row_index][] = $cellvalue;
        }

        $data['table_data'][$worksheet_index]['record_cols_qty'][$row_index] =
            count($data['table_data'][$worksheet_index]['str_value'][$row_index]);
        $row_index++;

      } // cell loop
       
    } // row loop
    $data['table_data'][$worksheet_index]['rows_qty'] = $row_index;
    if ((count($data['table_data'][$worksheet_index]['column_titles']) == 1) &&
    ($data['table_data'][$worksheet_index]['column_titles'][0] == 'null')) {
      $data['table_data'][$worksheet_index]['titles_cols_qty'] = 0;
      unset($data['table_data'][$worksheet_index]['column_titles'][0]);
    }
    else {
      $data['table_data'][$worksheet_index]['titles_cols_qty'] =
          count($data['table_data'][$worksheet_index]['column_titles']);
    }

    $worksheet_index++;
  } // worksheet loop
  $data['table_qty'] = $worksheet_index;

  $objPHPExcel->disconnectWorksheets();
  unset($objPHPExcel, $objReader);
  file_delete('sites/default/files/import/' . $hash);

  // 2.2 Extracting table name from file name
  tow_import_progress($hash, $stage);

  if (!in_array($info['extension'], array('xls', 'xlsx'))) {
    $data['table_data'][0]['table_title'] =  trim($info['filename']);
  }

  // 2.3 Check whether a table with this titles already exists in the present dataset.
  tow_import_progress($hash, $stage);

  if ($import_action == 'create') {
    $existing_table_titles_res = db_query("
  	SELECT title 
  	FROM {node} `n` 
  	INNER JOIN {content_type_table} `t` 
  	ON `n`.nid = `t`.nid
  	AND `t`.field_dataset_value = %d", $dataset_nid);

    $existing_table_titles = array();
    while ($rs = db_fetch_array($existing_table_titles_res)) {
      $existing_table_titles[] = strtolower($rs['title']);
    }

    // Checking and changing table title
    foreach ($data['table_data'] as &$table) {
      $table_name_index = 1;
      $table_title_unique = $table['table_title'];
      while (in_array(strtolower($table_title_unique), $existing_table_titles)) {
        $table_title_unique = $table['table_title'] . '_' . $table_name_index;
        $table_name_index++;
      }
      if ($table_name_index <> 1) {
        $warning_messages[] = "Table \"" . $table['table_title'] . "\" already exists, new table title will be set to \"" . $table_title_unique . "\"";
        $table['table_title'] = $table_title_unique;
        $existing_table_titles[] = $table_title_unique;

      }
    }
    unset($table_title_unique, $rs, $table_name_index, $existing_table_titles_res, $existing_table_titles, $table);
  }

  // TODO: handle absence of data in the data file

  $types = tow_possible_fields();
  $types_id = array_flip($types);

  // 2.4 Titles validation
  tow_import_progress($hash, $stage);

  foreach ($data['table_data'] as $table_index => &$table) {

    if ((!$table['titles_cols_qty']) && ($import_action == 'create')) {
      // No titles, table to be created case
      $warning_messages[] = "No data found for the table " . $data['table_data'][$table_index]['table_title'] .
      		". An empty table will be created.";
      //$data['table_qty']--;
      //unset($data['table_data'][$table_index]);
      $consider_column_titles_as_data = false;
    }
    elseif ((!$table['rows_qty']) && ($import_action == 'append')) {
      // No data, table to be appended case
      $warning_messages[] = "No data found in the imported file. Nothing will be appended.";
      tow_import_progress($hash, $stage, 'fail');
      $data['table_qty']--;
      unset($data['table_data'][$table_index]);
      break;
    }
    elseif (($table['rows_qty']) && ($import_action == 'append')) {
      // Data present, table to be appended case
      $consider_column_titles_as_data = false;
    }
    elseif ($table['rows_qty']) {
      // Titles present and data rows present, table to be created case
      $consider_column_titles_as_data = false;  // set to true if column names appear to be numbers or dates
      //  Checking that column names are neither numbers nor dates
      foreach  ($table['column_titles'] as $key => &$column_title) {
        // trimmming current element in the array
        $column_title = trim($column_title);

        // old condition, includes taboo on column titles being a date
        // works bad, because values like 'y' are treated as date  
        // if (is_numeric($column_title) || (strtotime($column_title) !== false) || (strtolower($column_title) === "null")) {
        if (is_numeric($column_title) || (strtolower($column_title) === "null")) {
          $consider_column_titles_as_data = true;
          break;
        }
      }
    }
    else {
      // Titles present, no data, table to be created case
      $consider_column_titles_as_data = true;
    }

    if ($consider_column_titles_as_data) {
      // consider the first first line from the file as data
      array_unshift($table['str_value'], $table['column_titles']);
      // increase row counter
      $table['rows_qty']++;
      // update column counter array
      array_unshift($table['record_cols_qty'], $table['titles_cols_qty']);
      // and set new default column titles
      unset($table['column_titles']);
      for ($col_index = 0; $col_index <= $table['titles_cols_qty'] - 1; $col_index++) {
        $table['column_titles'][$col_index] = $default_column_title . $col_index;
      }
      $warning_messages[] = "No column titles found in data file, using default column titles";
    }
    else {
      // Set empty column titles to default name
      foreach  ($table['column_titles'] as $key => &$column_title) {
        if ($column_title == '') {
          $column_title = $default_column_title . $key;
        }
      } // foreach
    } // else
  }
  // End of titles validation

  // 2.5 Data validation
  tow_import_progress($hash, $stage);

  foreach ($data['table_data'] as $table_index => &$table) {
    // detect type and count no of columns for each record
    foreach ($table['str_value'] as $record_index => $record) {

      foreach ($record as $col_index => $cell) {

        // detect cell type, get its value of required type
        $res = _tow_import_detect_type($cell);
        $data['table_data'][$table_index]['str_value'][$record_index][$col_index] = trim($cell);
        $data['table_data'][$table_index]['value'][$record_index][$col_index] = $res['value'];
        $data['table_data'][$table_index]['type'][$record_index][$col_index] = $res['type'];
        $data['table_data'][$table_index]['type_id'][$record_index][$col_index] = $types_id[$res['type']];

      } // cell loop
    } // row loop
  }



  foreach ($data['table_data'] as $table_index => &$table) {

    // Data columns qty validation
    if ($import_action == 'create') { // create case
      if ($table['rows_qty'] && (count(array_unique($table['record_cols_qty'])) - 1)) {
        $warning_messages[] = "Records in the table " . $data['table_data'][$table_index]['table_title'] .
          " contain different number of columns. Table will not be created.";
        $data['table_qty']--;
        unset($data['table_data'][$table_index]);
      }
      // Data columns qty and titles qty validation
      elseif ($table['rows_qty'] && ($table['record_cols_qty'][0] != $table['titles_cols_qty'])) {
        $warning_messages[] = "The number of column titles in the table " . $data['table_data'][$table_index]['table_title'] .
          " does not match the number of columns in records. Table will not be created.";
        $data['table_qty']--;
        unset( $data['table_data'][$table_index] );
      }
      // Column types validation
      elseif ($table['rows_qty']) {
        $invalid_types = false;
        foreach ($table['type'] as $record_index => $record_types) {
          foreach ($record_types as $col_index => $cell_type) {
            /*if (!$record_index) { 
              $data['table_data'][$table_index]['col_type'][$col_index] = $cell_type;
            }
            elseif ($cell_type != $data['table_data'][$table_index]['col_type'][$col_index]) {
              $invalid_types = true;
              break;
            }*/
            if (!$record_index) { 
              // first record in the table, its types will be used as first type option 
              $table['col_type'][$col_index] = $cell_type;
            }
            // determine and set as column type the 'greatest common' type for previous records 
            // and one consequent record
            elseif (!( $table['col_type'][$col_index] = _tow_common_type($cell_type, $table['col_type'][$col_index]))) {
              // if does not exist, set error, stop iterations
              $invalid_types = true;
              break;              
            }
          } // cell loop
          if ($invalid_types) {
            $warning_messages[] = "The types of values in different rows in the table " . $data['table_data'][$table_index]['table_title'] .
              " do not match each other. Table will not be created.";
            unset( $table );
            $data['table_qty']--;
            break;
          }
        } // row loop
      }
      else { // now rows case ( == no records in the table) 
        $data['table_data'][$table_index]['col_type'] = array();
      } // if
    } // create case
    else { // append case
      $table['titles_cols_qty'] = $table['record_cols_qty'][0];
      
      $table_node_to_be_updated = node_load($table_nid);
      $weights_array = unserialize($table_node_to_be_updated->field_weights[0]['value']);
      $index = 0;

      foreach (tow_possible_fields() as $field_type) {
        //$delta = 0;
        if (!is_array($table_node_to_be_updated->{'field_title_' . $field_type}))
        continue;
        foreach($table_node_to_be_updated->{'field_title_' . $field_type} as $title_value) {
          if ($title_value['value']) {
            //$aux_array[$weights_array[$index]]['value'] = $title_value['value'];
            $data['table_data'][$table_index]['col_type'][$weights_array[$index]] = $field_type;
            //$aux_array[$weights_array[$index]]['delta'] = $delta;
            $index++;
            //$delta++;
          }
        }
      }
      ksort($data['table_data'][$table_index]['col_type']);

      $invalid_types = false;
      foreach ($table['type'] as $record_index => $record_types) {
        foreach ($record_types as $col_index => $cell_type) {
          if ($cell_type != $data['table_data'][$table_index]['col_type'][$col_index]) {
            $invalid_types = true;
            break;
          }
        } // cell loop
        if ($invalid_types) {
          $warning_messages[] = "The types of imported data do not match that of an existing table. No data will be appended.";
          unset( $data['table_data'][$table_index]);
          $data['table_qty']--;
          break;
        }
      } // row loop

    }
  } // table loop
  unset($table);
  // End of data validation

  if ($data['table_qty']) {
    tow_import_progress($hash, $stage, 'finish');
    $progress_indicator = 1;
  }
  else {
    tow_import_progress($hash, $stage, 'fail');
  }
  
  // Creating content
  $stage = 3;
  // Count total rows qty for progress indicator updating  
  $total_rows = 0;
  foreach ($data['table_data'] as $table) {
    $total_rows += $table['rows_qty'];
  }
  tow_import_progress($hash, $stage, 'set', $total_rows + 1);

  if ($import_action == 'create') {
    // reading key titles from other tables
    $existing_table_nids_res = db_query("
      SELECT `nid`
      FROM {content_type_table} `t` 
      WHERE `t`.field_dataset_value = %d", $dataset_nid);

    $column_titles_keys = array();
    while ($rs = db_fetch_array($existing_table_nids_res)) {
      $table_node = node_load($rs['nid']);
      $weights_array = unserialize($table_node->field_weights[0]['value']);

      $index = 0;
      foreach (tow_possible_fields() as $field_type) {
        //$delta = 0;
        if (!is_array($table_node->{'field_title_' . $field_type}))
        continue;
        foreach($table_node->{'field_title_' . $field_type} as $title_value)
        if ($title_value['value']) {
          $aux_array[$weights_array[$index]]['value'] = $title_value['value'];
          //$aux_array[$weights_array[$index]]['type'] = $field_type;
          //$aux_array[$weights_array[$index]]['delta'] = $delta;
          $index++;
          //$delta++;
        }
      }
      ksort($aux_array);
      $indexes_array = unserialize($table_node->field_indexes[0]['value']);
      foreach ($indexes_array as $key => $is_index)
      if ($is_index) {
        $column_titles_keys[] = $aux_array[$key]['value'];
      }
    }
    unset($existing_table_nids_res, $rs, $table_node, $weights_array, $indexes_array);
  }

  tow_import_progress($hash, $stage);
  // $tables

  $created_table_titles = array();
  $created_table_nids = array();
  $found_key_titles = array();
  foreach ($data['table_data'] as $table_index => &$table) {

    if ($import_action == 'create') {
      // Get parameters for the present table
      // $data = $data_all[$table_index];
      //$table_title = $table['table_title'];
      //$column_titles = $table['col_titles'];

      // Fill in new table node
      $new_table = new stdClass();
      $new_table->type = 'table';
      $new_table->title = $table['table_title'];
      // TODO:
      //$new_table->field_dataset_value = $dataset_nid;
      $new_table->uid = $uid;
      $new_table->body = "The table was imported from file \"" . $info['filename'] . '.' . $info['extension'] . '".';
      $new_table->teaser = $new_table->body;

      // 3.2 Specify columns, their types and titles

      $column_type = array();
      $indexes_array = array();

      for ($col_index = 0; $col_index <= $table['titles_cols_qty'] - 1; $col_index++) {

        //3.3 types of columns
        //$type_id = mt_rand(0, $types_qty-1);

        $column_type[$col_index]['type_id'] = $types_id[$table['col_type'][$col_index]];
        $field_type = "field_title_" . $table['col_type'][$col_index];
        $column_type[$col_index]['type'] = $table['col_type'][$col_index];

        // 3.4 column titles
        $new_table->{$field_type}[]['value'] = $table['column_titles'][$col_index];
        // 3.5 enum options if needed
        /*    if ($field_type == "field_title_enum") {
        $new_table->{field_enum_options}[]['value']=
        serialize(array(_tow_tests_get_random_string($max_leng['enum_option'] ),
        _tow_tests_get_random_string($max_leng['enum_option'] ),
        _tow_tests_get_random_string($max_leng['enum_option'] )));
        }
        */
      }
      // TODO: random weights
      //   3.6 field_dataset_nid, weights generate
      $weights_array = array ();
      foreach ($types as $type_id => $type) {
        foreach ($table['col_type'] as $index => $col_type) {
          if ($col_type == $type) {
            $delta++;
            $weights_array[] = $index;
          }
        }
      }

      // searching for keys
      $indexes_array = array();
      foreach ($table['column_titles'] as $col_index => $col_title) {
        if (in_array($col_title, $column_titles_keys)) {
          $indexes_array[] = 1;
          $found_key_titles[] = $col_title;
        }
        else {
          $indexes_array[] = 0;
        }
      }

      $new_table->field_weights[0]['value'] = serialize($weights_array);
      $new_table->field_dataset[0]['value'] = $dataset_nid;
      $new_table->field_indexes[0]['value'] = serialize($indexes_array);

      // 3.7 node save, get table nid
      //drupal_set_message('<pre>' . print_r($new_table, true) . '</pre>');
      node_save($new_table);
      $table_nid = $new_table->nid;

      if ($table_nid) {
        $created_table_nids[] = $table_nid;
        $created_table_titles[] = $new_table->title;
      }
    } // create table
    for ($record_index = 0; $record_index < $table['rows_qty']; $record_index++) {
      // 4.1 type,
      $new_record = new stdClass();
      $new_record->type = 'record';
      $new_record->uid = $uid;
      // 4.2 data
      for ($col_index = 0; $col_index <= $table['titles_cols_qty'] - 1; $col_index++) {
        if (isset($table['value'][$record_index][$col_index])) {
          $new_record->{"field_" .  $table['col_type'][$col_index]}[$col_index]['value'] = $table['value'][$record_index][$col_index];
        }
        else {
          $new_record->{"field_" .  $table['col_type'][$col_index]}[$col_index]['value'] = '<not set>';
        }
      }
      //drupal_set_message('<pre>' . print_r($new_record, true) . '</pre>');
      $new_record->field_table[0]['value'] = $table_nid;
      node_save($new_record);
      tow_import_progress($hash, $stage);
    } // end of records

    unset($new_table);
    unset($table_nid);
  }

  tow_import_progress($hash, $stage, 'finish');

  if ($warning_messages) {
    foreach  ($warning_messages as $value) {
      $warning .= $value . '; ';
    }
    $warning = substr($warning, 0, -2);
    // TODO:  make a list of warnings to be displayed
    // $warning = theme_item_list($warning_messages, NULL, 'ul');
    tow_import_progress($hash, 4, 'message', $warning);
  }
  else
    tow_import_progress($hash, 4, 'fail');
  //tow_import_progress($hash, 4, $warning);

  if ($found_key_titles) {
    foreach  ($found_key_titles as $value) {
      $found_keys .= $value . '; ';
    }
    $found_keys = substr($found_keys, 0, -2);
    // TODO:  make a list of warnings to be displayed
    // $warning = theme_item_list($warning_messages, NULL, 'ul');
    tow_import_progress($hash, 5, 'message', 'we found ' . count($found_key_titles) . ' key fields (' . $found_keys . ')');
  }
  elseif (($progress_indicator < 0) || ($import_action == 'append')) {
    tow_import_progress($hash, 5, 'fail');
  }
  else {
    tow_import_progress($hash, 5, 'message', "we found no key fields");
  }

  if ($import_action == 'create') {
    if ($created_table_titles) {
      foreach  ($created_table_titles as $value) {
        $created_tables .= $value . '; ';
        $created_tables_spaced .= $value . ' ';
      }
      foreach  ($created_table_nids as $value) {
        $created_nids_spaced .= $value . ' ';
      }
      $created_tables = substr($created_tables, 0, -2);
      // TODO:  make a list of warnings to be displayed
      // $warning = theme_item_list($warning_messages, NULL, 'ul');
      tow_import_progress($hash, 6, 'message', $created_tables . ' created./ ' . $created_tables_spaced . '/ ' . $created_nids_spaced);
    }
    else {
      tow_import_progress($hash, 6, 'message', "no tables created");
    }
  }
  else {
    if (($progress_indicator >= 0)) {
      tow_import_progress($hash, 6, 'message', $table_node_to_be_updated->title . ' updated./ ' . $table_node_to_be_updated->title . '/ ' . $table_node_to_be_updated->nid);
    }
    else
    tow_import_progress($hash, 6, 'message', "No data was appended.");
  }
}

/**
 * Detects the type of input value.
 * @param string $str_value
 * @return array('type' => 'bool', 'int', float', 'null', 'date' or 'text',
 * 				 'value' => formatted value)	
 */
function _tow_import_detect_type($str_value)
{
  $value = strtolower(trim($str_value));

  // boolean
  if ( ($value == 'true') ||
  ($value == 'false') ||
  ($value == 'yes') ||
  ($value == 'no') )
  {
    $res['type'] = "bool";
    if ($value == 'true' || $value == 'yes') {
      $res['value'] = 'true';
    }
    else {
      $res['value'] = 'false';
    }
  }
  // numeric: integer or float
  elseif  (is_numeric($value)) {
    if (($res['value'] = (integer)$value) == $value) {
      $res['type'] = "int";
    }
    else
    {
      $res['type'] = "float";
      $res['value'] = (float) $value;
    }
  }
  // date
  elseif ($timestamp=strtotime($value)) {
	if (abs($timestamp - time()) <= 86400) {//24*60*60
		$res['value'] = date("H:i:s",$timestamp);
		$res['type'] = "time";
	} elseif (date('His',$timestamp) == '000000') {
		$res['value'] = date("Y-m-d",$timestamp);
		$res['type'] = "date";
	} else {
		$res['value'] = date("Y-m-d H:i:s",$timestamp);
		$res['type'] = "datetime";
	}
  }
  elseif ($value === "null") {
    $res['type'] = "null";
    $res['value'] = null;
  }
  else {
    $res['type'] = "text";
    $res['value'] = $str_value;
  }
  return $res;
}

/**
 * 
 * Determines the 'greatest common' type of two input types
 * @param string $type1, $type2
 * @return string ('bool', 'int', float', 'null', 'date', 'time', 'datetime' or 'text') or 0 if no common type for 
 * input values exists   
 */
function _tow_common_type($type1, $type2) {
  $in_type1 = strtolower(trim($type1));
  $in_type2 = strtolower(trim($type2));
  if ($in_type1 === $in_type2) {
    return $in_type1;
  };
  
  $common_types = array('bool'=> array('int'=> 'text',
  								   'float'=> 'text',
                                   'null'=> 'bool',
  									'date'=> 'text',
  									'time'=> 'text',
  									'datetime'=> 'text',
  									'text'=> 'text',),
                       'int'=> array('bool'=> 'text',
  								   'float'=> 'float',
                                   'null'=> 'int',
  									'date'=> 'text',
  									'time'=> 'text',
  									'datetime'=> 'text',
  									'text'=> 'text',),
                      'float'=> array('bool'=> 'text',
                                   'int'=> 'float',
                                   'null'=> 'float',
  									'date'=> 'text',
  									'time'=> 'text',
  									'datetime'=> 'text',
  									'text'=> 'text',),
                      'null'=> array('bool'=> 'bool',
                                   'int'=> 'int',
  								   'float'=> 'float',
  									'date'=> 'date',
  									'time'=> 'time',
  									'datetime'=> 'datetime',
  									'text'=> 'text',),
                      'date'=> array('bool'=> 'text',
                                   'int'=> 'text',
  								   'float'=> 'text',
                                   'null'=> 'date',
  									'time'=> 'datetime',
  									'datetime'=> 'datetime',
  									'text'=> 'text',),
                      'time'=> array('bool'=> 'text',
                                   'int'=> 'text',
  								   'float'=> 'text',
                                   'null'=> 'date',
  									'date'=> 'datetime',
  									'datetime'=> 'datetime',
  									'text'=> 'text',),
                      'datetime'=> array('bool'=> 'text',
                                   'int'=> 'text',
  								   'float'=> 'text',
                                   'null'=> 'date',
  									'time'=> 'datetime',
  									'date'=> 'datetime',
  									'text'=> 'text',),
                      'text'=> array('bool'=> 'text',
                                   'int'=> 'text',
  								   'float'=> 'text',
                                   'null'=> 'text',
  									'date'=> 'text',
  									'time'=> 'text',
  									'datetime'=> 'text',),
                  );
  if (isset($common_types[$in_type1][$in_type2])) {
    return $common_types[$in_type1][$in_type2];
  }
  else {
    return 0;
  }
}  

