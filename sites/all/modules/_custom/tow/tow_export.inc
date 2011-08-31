<?php
// $Id: tow_export.inc,v 1.0.0.0 2011/02/28 23:42:15 tow Exp $

/**
 * @file
 * Functions for export
 */

define('TOW_EXPORT_FILE_LIFETIME', 10);

######### FORM ########

function tow_export_possible_formats($value, $format_id = false) {
  $formats = array(
    'PDF'     => array('extension' => 'pdf', 'writer_type' => 'PDF',          'prompt' => 'PDF'),
    'CSV'     => array('extension' => 'csv', 'writer_type' => 'CSV',          'prompt' => 'CSV'),
    'TXT/tab' => array('extension' => 'txt', 'writer_type' => 'CSV',          'prompt' => 'Text (tab-delimited)'),
    'XLS/mul' => array('extension' => 'xls', 'writer_type' => 'Excel5',       'prompt' => 'Excel (multiple worksheets)'),
    'XLS/sgl' => array('extension' => 'xls', 'writer_type' => 'Excel5',       'prompt' => 'Excel (all in one sheet)'),
    'XML'     => array('extension' => 'xls', 'writer_type' => 'Excel2003XML', 'prompt' => 'XML'),
    'SQL'     => array('extension' => 'sql', 'writer_type' => '',             'prompt' => 'SQL'),
    'HTML'    => array('extension' => 'htm', 'writer_type' => 'HTML',         'prompt' => 'HTML'),
  
  );
  
  
  if (in_array($value, array_keys($formats['PDF']))) {
    if ($format_id) {
      return ($formats[$format_id][$value]);
    }  
    else {
      $result = array();
      foreach ($formats as $format_key => $format) {
        $result[$format_key] = $formats[$format_key][$value];
      }
      return $result;
    }
  } 
  else { 
    return false;
  }
}

function tow_export_form($form_state, $nid) {
  
  global $dataset;
  $dataset = node_load($nid);
  if ($dataset->type !== 'dataset') {
    drupal_not_found();
    die();
  }
  
  drupal_add_js(drupal_get_path('module', 'tow') . '/js/export.js');
  
  drupal_set_title($dataset->title);
  
    // way
  $form['to'] = array(
    '#type' => 'item',
    '#tree' => TRUE,
    '#title' => t('Export data'),
  );
  $form['to']['way'] = array(
    '#type' => 'radios',
    '#default_value' => 0,
    '#options' => array(t('Entire data set'), t('Selected tables')),
  );
  $form['to']['table'] = array(
    '#type' => 'select',
    '#options' => tow_table_list($nid),
    '#multiple' => TRUE
  );
  $form['guide'] = array('#type' => 'item', '#value' => l('Learn about exporting data', 'export_learn'));
  
  $form['format'] = array(
    '#type' => 'radios',
    '#options' => tow_export_possible_formats('prompt'),
    '#default_value' => 'CSV',
    '#title' => t('Choose format of exported file')
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export')
  );
  
  return $form;
}

function tow_export_form_submit($form, &$form_state) {
  
  /** PHPExcel */
  require_once 'sites/all/libraries/PHPExcelExtension/PHPExcel.php';
  /** PHPExcel_Cell_AdvancedValueBinder */
  require_once 'sites/all/libraries/PHPExcelExtension/PHPExcel/Cell/AdvancedValueBinder.php';
  /** PHPExcel_IOFactory */
  require_once 'sites/all/libraries/PHPExcelExtension/PHPExcel/IOFactory.php';
  // Set value binder
  PHPExcel_Cell::setValueBinder( new PHPExcel_Cell_AdvancedValueBinder() );

  require_once 'sites/all/libraries/PHPExcelExtension/PHPExcel/Shared/String.php';
  PHPExcel_Shared_String::setDecimalSeparator('.');
  PHPExcel_Shared_String::setThousandsSeparator('');
  
  
  global $dataset;
  
  // Input parameters
  $dataset_nid =  $dataset->nid;
  $export_selected_tables_only = (bool) $form_state['values']['to']['way'];
  $export_format = $form_state['values']['format']; 
  
  // 0.1 Set export settings
  switch ($export_format) {
    case 'PDF':
    case 'CSV': 
    case 'TXT/tab': 
    case 'XLS/sgl': 
    case 'XML': 
    case 'SQL': 
    case 'HTML':
      $export_settings['create_multiple_sheets'] = false;
      break;
    case 'XLS/mul':
      $export_settings['create_multiple_sheets'] = true;
  }
  // 0.2 Prepare table nids to be exported
  if ($export_selected_tables_only) { 
    // remember only specified nids
    $table_nids =  $form_state['values']['to']['table'];
  }
  else {
    // get nids of all tables in the dataset   
    $table_nids_res = db_query("
    SELECT `nid`
    FROM {content_type_table} `t` 
    WHERE `t`.field_dataset_value = %d", $dataset_nid);
    $table_nids = array();
    while ($rs = db_fetch_array($table_nids_res)) {
      $table_nids[] = strtolower($rs['nid']);
    }
  }
  
  // 1 Read table data
  $export_data = array();
  $export_data['table_qty'] = 0;
  // 1.0 Read table node 
  
  foreach($table_nids as $table_index => $value) {
    $table_node = node_load($value);
    
    $export_data['table_titles'][] = $table_node->title;
    
    // 1.1 Construct sql-request for the desired data
    // 1.1.1 Set options for the query
    $query_options['click_sortable'] = false;     
    // TODO: set ascending sort order
    // 1.1.2 Construct SQL query   
    $user_table = tow_construct_table_query($table_node, $query_options);
    // 1.1.3 Receive data
    
    // Column titles
    $export_data['column_titles'][] = $user_table['titles']['values'];

    // Column types, required only for export into sql format 
    if ($export_format == 'SQL') {
      $export_data['column_types'][] = $user_table['titles']['types'];
    }
    // Data in records
    $result = db_query($user_table['query'], $user_table['arguments']);
    
    // 1.1.4 Remember key fields for SQL
    if ($export_format == 'SQL') {
      //$export_data['key_column_titles'][$table_index] = array();
      $indexes = unserialize($table_node->field_indexes[0]['value']);
      $key_column_titles = array();
      foreach ($user_table['titles']['values'] as $col_index => $column_title) {
        if ($indexes[$col_index]) {
          $key_column_titles[] = $column_title;
        }
      }   
      $export_data['key_column_titles'][] = $key_column_titles;
    }
    
    $rows = array();
    // 1.2 Convert received data into actual data to be put into the file 
    while ($rs = db_fetch_array($result)) {
      $row = array();
      foreach ($rs as $key => $value) {
        if ($user_table['titles']['types'][$key] == 'bool') {
          $row[] = _tow_format_bool($value);
        } 
        elseif ($user_table['titles']['types'][$key] == 'date') {
          $row[] = substr($value, 0, 10);
        } 
        elseif ($user_table['titles']['types'][$key] == 'timestamp') {
          $row[] = date('d M Y H:i:S', $value);  
        } 
        elseif ($user_table['titles']['types'][$key] == 'enum') {
          $title_enum = $user_table['titles']['values'][$key];
          foreach($table_node->field_title_enum as $enum_delta => $enum_value) {
            if ($enum_value['value'] == $title_enum) {
              unset($table_node->field_title_enum[$enum_delta]);
              break;
            }
          }
          $enum_options = unserialize($table_node->field_enum_options[$enum_delta]['value']);
          $row[] = $enum_options[$value]; 
        }
        else {
          $row[] = $value;
        }
      } // cell loop 
     // remove record nid value from the row array
      array_shift($row);
      
      $rows[] = $row;
    } // row loop
    unset($result);
    $export_data['cell_data'][] = $rows;
    $export_data['table_qty']++;
  } // table loop

  // 1.3 Format table titles and column titles for SQL
  if ($export_format == 'SQL') {
    foreach ($export_data['table_titles'] as &$table_title) {
      $table_title = preg_replace('/[^A-Za-z0-9]/', '_', $table_title);  
    }
     foreach ($export_data['column_titles'] as &$column_titles) {
       foreach ($column_titles as &$column_title) {
         $column_title = preg_replace('/[^A-Za-z0-9]/', '_', $column_title);  
       } // column loop
     } // table loop
     foreach ($export_data['key_column_titles'] as &$column_titles) {
       foreach ($column_titles as &$column_title) {
         $column_title = preg_replace('/[^A-Za-z0-9]/', '_', $column_title);  
       } // column loop
     } // table loop
  }
  
  // 2. Record data in the file
  // 2.0 Set file properties
  if (count($table_nids) == 1) {
    // If only one table exported, set filename to its title
    $filename = $export_data['table_titles'][0]; // temporary file name
    $filename =  strtr($filename, ' :?*$%/<>|\\"', '____________');
    $temp_file = $_SERVER['DOCUMENT_ROOT'] . base_path() . file_directory_path() . '/export/' . $filename;
  }
  else {
    // otherwise, set filename to that of dataset
    $filename = $dataset->title; // temporary file name
    $filename =  strtr($filename, ' :?*$%/<>|\\"', '____________');
    $temp_file = $_SERVER['DOCUMENT_ROOT'] . base_path() . file_directory_path() . '/export/' . $filename;
  }
  
  $export_file_extension = tow_export_possible_formats('extension', $export_format);
  
  // 2.1 Create writer object of required type
  // Create new PHPExcel object
  if ($export_format != 'SQL') {
    $objPHPExcel = new PHPExcel();
  }
  else {
    $SQL_requests = "SET SQL_MODE=\"NO_AUTO_VALUE_ON_ZERO\";\n\n";
  }
  // 2.2 Put all the data into the writer object
  if ($export_format != 'SQL') {
    $current_cell = array('row' => 1, 'column' => 0);
    foreach ($export_data['table_titles'] as $table_index => $table_title) {
      // 2.2.a.1 set active worksheet title
      if ($export_settings['create_multiple_sheets']) {
        // set current sheet title to $export_data['table_titles'][$table_index]
        $objPHPExcel->getActiveSheet()->setTitle($export_data['table_titles'][$table_index]);
      }
      else 
      {
        // set current sheet title to $dataset->title;
        $objPHPExcel->getActiveSheet()->setTitle($dataset->title);
        // append table title to the previously recorded data  
        $objPHPExcel->getActiveSheet()->getCellByColumnAndRow(0, $current_cell['row']++)->setValue($export_data['table_titles'][$table_index]);
      }
      
      // 2.2.a.3 record column titles into the worksheet
      foreach($export_data['column_titles'][$table_index] as $column_title) {
        $objPHPExcel->getActiveSheet()->getCellByColumnAndRow($current_cell['column']++, $current_cell['row'])->setValue($column_title);  
      } // column titles loop
      // reset counters 
      $current_cell['column'] = 0;
      $current_cell['row']++;
      
      // 2.2.a.4 record data on the sheet
      foreach ($export_data['cell_data'][$table_index] as $row) {
        foreach ($row as $cell) {
           $objPHPExcel->getActiveSheet()->getCellByColumnAndRow($current_cell['column']++, $current_cell['row'])->setValue($cell); 
        } // cell loop
        // update counters
        $current_cell['column'] = 0;
        $current_cell['row']++;
      } // rows loop
      // 2.2.a.5 Add another sheet if needed
      if ($export_settings['create_multiple_sheets'] && ($table_index < (count($export_data['table_titles'])-1))) {
        // create new one
        $objPHPExcel->createSheet();
        // Make created sheet active
        $objPHPExcel->setActiveSheetIndex($objPHPExcel->getActiveSheetIndex(0) + 1);
        // reset row counter to zero
        $current_cell['row'] = 1;
      }
    } // table loop
  }
  else { // Import to SQL 
    //$current_cell = array('row' => 1, 'column' => 0);
    foreach ($export_data['table_titles'] as $table_index => $table_title) {
      // 2.2.b.1 Create new table
      // Table title
      $SQL_request = "CREATE TABLE IF NOT EXISTS `" . $table_title . "` (\n";

      foreach($export_data['column_titles'][$table_index] as $column_index => $column_title) {
        // Column titles 
        $SQL_request .= "  `" . $column_title . "` ";
        // Column types 
        $SQL_request .= _tow_export_get_sql_type($export_data['column_types'][$table_index][$column_index]) . "\n";
      } // column titles loop
      
      foreach ($export_data['key_column_titles'][$table_index] as $column_index => $column_title) {
        // Keys 
        $SQL_request .= "  KEY `" . $column_title . "` (`" . $column_title . "`),\n";
      }
      
      $SQL_request = substr($SQL_request, 0, -2);
      $SQL_request .= "\n) ENGINE=MyISAM DEFAULT CHARSET=utf8;\n";
      $SQL_requests .= $SQL_request . "\n";
      
      // 2.2.b.2 Insert values into the table
      $SQL_request = "INSERT INTO `" . $table_title . "` (";
      // Column titles
      foreach($export_data['column_titles'][$table_index] as $column_title) {
        $SQL_request .= "`" . $column_title . "`, ";
      } 
      $SQL_request = substr($SQL_request, 0, -2);
      $SQL_request .= ") VALUES\n";
      // values       
      foreach ($export_data['cell_data'][$table_index] as $row) {
        $SQL_request .= "(";
        foreach ($row as $column_index => $cell) {
          if ($cell) {
            $SQL_request .= _tow_export_format_sql_value($cell, $export_data['column_types'][$table_index][$column_index]) . ", ";
          }
          else {
            $SQL_request .= "NULL" . ", ";
          }
        } // cell loop
        $SQL_request = substr($SQL_request, 0, -2);
        $SQL_request .= "),\n"; 
      } // rows loop
      $SQL_request = substr($SQL_request, 0, -2);
      $SQL_request .= ";\n";
      $SQL_requests .= $SQL_request . "\n";
    } // table loop
  }
  // 2.3 Save data into the file
  // 2.3.1 set required writer type
  if ($export_format != 'SQL') {
    $writer_type = tow_export_possible_formats('writer_type', $export_format);
    // 2.3.2 create writer 
    $objWriter = PHPExcel_IOFactory::createWriter($objPHPExcel, $writer_type);
    // 2.3.3 Set writertype-dependent parameters
  }
    switch ($export_format) {
    case 'CSV': 
      $objWriter->setDelimiter(';');
      $objWriter->setEnclosure('');
      $objWriter->setLineEnding("\r\n");
      break;
    case 'TXT/tab': 
      $objWriter->setDelimiter("\t");
      $objWriter->setEnclosure('');
      $objWriter->setLineEnding("\r\n");
  }
  
  // 2.3.4 use writer to save file
   switch ($export_format) {
     case 'PDF':
     case 'CSV': 
     case 'TXT/tab': 
     case 'XLS/mul':
     case 'XLS/sgl': 
     case 'XML': 
     case 'HTML':
       $objWriter->save($temp_file);   
       break;
     case 'SQL':
       file_put_contents($temp_file, $SQL_requests);
   }  
  
  // 3. Send file to the user
  $file_headers = array(
    'Content-Type: octet/stream',
    'Content-Disposition: attachment; filename="' . $filename . '.' . $export_file_extension . '"',
    'Content-Length: ' . filesize($temp_file),
  );
   
  file_transfer($temp_file, $file_headers);
  // 4. Delete temporary file
  unlink($temp_file);

}

/**
 * @return SQL type corresponding to the $tow_type 
 */
function _tow_export_get_sql_type($tow_type) {
  switch ($tow_type) {
    case 'int':
      $res = 'int(11) DEFAULT NULL, ';
      break;
    case 'char':
      $res = 'varchar(255) DEFAULT NULL, ';
      break;
    case 'float':
      $res = 'decimal(10,2) DEFAULT NULL, ';
      break;
    case 'bool':
      $res = 'varchar(6) DEFAULT NULL, ';
      break;
    case 'text':
      $res = 'longtext, ';
      break;
    case 'enum':
      $res = 'varchar(255) DEFAULT NULL, ';
      break;
    case 'date':
      $res = 'varchar(20) DEFAULT NULL, ';
      break;
    case 'datetime':
      $res = 'datetime DEFAULT NULL, ';
      break;
    case 'time':
      $res = 'time DEFAULT NULL, ';
      break;
    case 'timestamp':
      $res = 'int(11) DEFAULT NULL, ';
      break;
    case 'code':
      $res = 'varchar(3) DEFAULT NULL, ';
      break;
    default:
      $res = '';
  }
 return $res;
}

/**
 * @return value suitable for SQL INSERT request 
 */
function _tow_export_format_sql_value($value, $tow_type) {
  switch ($tow_type) {
    case 'int':
    case 'float':
      $res = $value;
      break;
    case 'char':
    case 'bool':
    case 'text':
    case 'enum':
    case 'date':
    case 'datetime':
    case 'time':
    case 'timestamp':
    case 'code':
      $res = '\'' . $value . '\'';
  }
 return $res;
}