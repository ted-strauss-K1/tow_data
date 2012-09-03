<?php

/**
 * @file
 * 
 * Functions for export
 */



/****************************************************************************************************
 * Forms
 ****************************************************************************************************/

/**
 * Export form.
 */
function tow_export_form($form_state, $nid) {
  
  // Get dataset or die.
  global $_tow_dataset;
  $_tow_dataset = node_load($nid);
  if ($_tow_dataset->type !== 'dataset') {
    drupal_not_found();
    die();
  }
  
  // Add export javascript.
  drupal_add_js(drupal_get_path('module', 'tow') . '/js/export.js');
  
  // Set title.
  drupal_set_title($_tow_dataset->title);
  
  // Way.
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
  
  // Guide.
  $form['guide'] = array('#type' => 'item', '#value' => l('Learn about exporting data', 'export_learn'));
  
  // Format.
  $form['format'] = array(
    '#type' => 'radios',
    '#options' => tow_export_possible_formats('prompt'),
    '#default_value' => 'CSV',
    '#title' => t('Choose format of exported file')
  );
  
  // Submit.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export')
  );
  
  return $form;
}

/**
 * Export dataset to file specified.
 */
function tow_export_form_submit($form, &$form_state) {
  
  // PHPExcel.
  require_once 'sites/all/libraries/PHPExcelExtension/PHPExcel.php';
  
  // PHPExcel_Cell_AdvancedValueBinder.
  require_once 'sites/all/libraries/PHPExcelExtension/PHPExcel/Cell/AdvancedValueBinder.php';
  
  // PHPExcel_IOFactory.
  require_once 'sites/all/libraries/PHPExcelExtension/PHPExcel/IOFactory.php';
  
  // Set value binder.
  PHPExcel_Cell::setValueBinder(new PHPExcel_Cell_AdvancedValueBinder());

  // PHPExcel_Shared_String.
  require_once 'sites/all/libraries/PHPExcelExtension/PHPExcel/Shared/String.php';
  PHPExcel_Shared_String::setDecimalSeparator('.');
  PHPExcel_Shared_String::setThousandsSeparator('');
  
  global $_tow_dataset;
  
  // Input parameters.
  $dataset_nid =  $_tow_dataset->nid;
  $export_selected_tables_only = (bool) $form_state['values']['to']['way'];
  $export_format = $form_state['values']['format']; 
  
  // Set export settings.
  switch ($export_format) {
    case 'PDF':
    case 'CSV': 
    case 'TXT/tab': 
    case 'XLS/sgl': 
    case 'XML': 
    case 'SQL': 
    case 'HTML':
      $export_settings['create_multiple_sheets'] = FALSE;
      break;

    case 'XLS/mul':
      $export_settings['create_multiple_sheets'] = TRUE;
  }
  
  // Prepare table nids to be exported.
  if ($export_selected_tables_only) { 
    
    // Remember only specified nids.
    $table_nids =  $form_state['values']['to']['table'];
  }
  else {

    // Get nids of all tables in the dataset.
    $table_nids_res = db_query("

      SELECT 
        t.nid AS nid 

        FROM 
          {content_type_table} t 

        WHERE 
          t.field_dataset_value = %d 

    ", $dataset_nid);
    $table_nids = array();
    while ($rs = db_fetch_array($table_nids_res)) {
      $table_nids[] = strtolower($rs['nid']);
    }
  }

  // Read table data.
  $export_data = array();
  $export_data['table_qty'] = 0;
  
  // Read table node.
  foreach ($table_nids as $table_index => $value) {
    $table_node = node_load($value);
    $export_data['table_titles'][] = $table_node->title;
    
    // Construct sql-request for the desired data.
    // Set options for the query.
    $query_options['click_sortable'] = FALSE;
    
    // Construct SQL query.
    $user_table = tow_table_construct_table_query($table_node, $query_options);
    
    // Receive data.
    // Column titles.
    $export_data['column_titles'][] = $user_table['titles']['values'];

    // Column types, required only for export into sql format.
    if ($export_format == 'SQL') {
      $export_data['column_types'][] = $user_table['titles']['types'];
    }
    
    // Data in records.
    $result = db_query($user_table['query'], $user_table['arguments']);
    
    // Remember key fields for SQL.
    if ($export_format == 'SQL') {
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
    
    // Convert received data into actual data to be put into the file.
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
          foreach ($table_node->field_title_enum as $enum_delta => $enum_value) {
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
      }
      
      // Remove record nid value from the row array.
      array_shift($row);
      $rows[] = $row;
    }
    
    unset($result);
    $export_data['cell_data'][] = $rows;
    $export_data['table_qty']++;
  }

  // Format table titles and column titles for SQL.
  if ($export_format == 'SQL') {
    foreach ($export_data['table_titles'] as &$table_title) {
      $table_title = preg_replace('/[^A-Za-z0-9]/', '_', $table_title);  
    }
    
    // Table loop.
    foreach ($export_data['column_titles'] as &$column_titles) {
      
      // Column loop.
      foreach ($column_titles as &$column_title) {
         $column_title = preg_replace('/[^A-Za-z0-9]/', '_', $column_title);  
      }
    }
    
    // Table loop.
    foreach ($export_data['key_column_titles'] as &$column_titles) {
      
      // Column loop.
      foreach ($column_titles as &$column_title) {
        $column_title = preg_replace('/[^A-Za-z0-9]/', '_', $column_title);  
      }
    }
  }
  
  // Record data in the file.
  // Set file properties.
  if (count($table_nids) == 1) {
    
    // If only one table exported, set filename to its title.
    $filename = $export_data['table_titles'][0];
    $filename =  strtr($filename, ' :?*$%/<>|\\"', '____________');
    $temp_file = $_SERVER['DOCUMENT_ROOT'] . base_path() . file_directory_path() . '/export/' . $filename;
  }
  else {
    
    // Otherwise, set filename to that of dataset.
    $filename = $_tow_dataset->title;
    $filename =  strtr($filename, ' :?*$%/<>|\\"', '____________');
    $temp_file = $_SERVER['DOCUMENT_ROOT'] . base_path() . file_directory_path() . '/export/' . $filename;
  }
  
  $export_file_extension = tow_export_possible_formats('extension', $export_format);
  
  // Create writer object of required type.
  // Create new PHPExcel object.
  if ($export_format != 'SQL') {
    $obj_php_excel = new PHPExcel();
  }
  else {
    $SQL_requests = "SET SQL_MODE=\"NO_AUTO_VALUE_ON_ZERO\";\n\n";
  }
  
  // Put all the data into the writer object.
  if ($export_format != 'SQL') {
    $current_cell = array('row' => 1, 'column' => 0);
    foreach ($export_data['table_titles'] as $table_index => $table_title) {

      // Set active worksheet title.
      if ($export_settings['create_multiple_sheets']) {
        
        // Set current sheet title to $export_data['table_titles'][$table_index].
        $obj_php_excel->getActiveSheet()->setTitle($export_data['table_titles'][$table_index]);
      }
      else {
        
        // Set current sheet title to $_tow_dataset->title.
        $obj_php_excel->getActiveSheet()->setTitle($_tow_dataset->title);
        
        // Append table title to the previously recorded data.
        $obj_php_excel->getActiveSheet()->getCellByColumnAndRow(0, $current_cell['row']++)->setValue($export_data['table_titles'][$table_index]);
      }
      
      // Record column titles into the worksheet.
      foreach ($export_data['column_titles'][$table_index] as $column_title) {
        $obj_php_excel->getActiveSheet()->getCellByColumnAndRow($current_cell['column']++, $current_cell['row'])->setValue($column_title);  
      }
      
      // Reset counters.
      $current_cell['column'] = 0;
      $current_cell['row']++;
      
      // Record data on the sheet.
      foreach ($export_data['cell_data'][$table_index] as $row) {
        foreach ($row as $cell) {
           $obj_php_excel->getActiveSheet()->getCellByColumnAndRow($current_cell['column']++, $current_cell['row'])->setValue($cell); 
        }
        
        // Update counters.
        $current_cell['column'] = 0;
        $current_cell['row']++;
      }
      
      // Add another sheet if needed.
      if ($export_settings['create_multiple_sheets'] && ($table_index < (count($export_data['table_titles'])-1))) {
        
        // Create new one.
        $obj_php_excel->createSheet();
        
        // Make created sheet active.
        $obj_php_excel->setActiveSheetIndex($obj_php_excel->getActiveSheetIndex(0) + 1);
        
        // Reset row counter to zero.
        $current_cell['row'] = 1;
      }
    }
  }
  else { 
    
    // Import to SQL
    foreach ($export_data['table_titles'] as $table_index => $table_title) {
      
      // Create new table.
      // Table title.
      $SQL_request = "CREATE TABLE IF NOT EXISTS " . $table_title . " (\n";

      foreach ($export_data['column_titles'][$table_index] as $column_index => $column_title) {
        
        // Column titles.
        $SQL_request .= "  " . $column_title . " ";
        
        // Column types.
        $SQL_request .= _tow_export_get_sql_type($export_data['column_types'][$table_index][$column_index]) . "\n";
      }
      
      foreach ($export_data['key_column_titles'][$table_index] as $column_index => $column_title) {
        
        // Keys 
        $SQL_request .= "  KEY " . $column_title . " (" . $column_title . "), ";
      }
      
      $SQL_request = substr($SQL_request, 0, -2);
      $SQL_request .= "\n) ENGINE=MyISAM DEFAULT CHARSET=utf8;\n";
      $SQL_requests .= $SQL_request . "\n";
      
      // Insert values into the table.
      $SQL_request = "INSERT INTO " . $table_title . " (";
      
      // Column titles.
      foreach ($export_data['column_titles'][$table_index] as $column_title) {
        $SQL_request .= "" . $column_title . ", ";
      } 
      $SQL_request = substr($SQL_request, 0, -2);
      $SQL_request .= ") VALUES\n";
      
      // Values.
      foreach ($export_data['cell_data'][$table_index] as $row) {
        $SQL_request .= "(";
        foreach ($row as $column_index => $cell) {
          if ($cell) {
            $SQL_request .= _tow_export_format_sql_value($cell, $export_data['column_types'][$table_index][$column_index]) . ", ";
          }
          else {
            $SQL_request .= "NULL" . ", ";
          }
        }
        $SQL_request = substr($SQL_request, 0, -2);
        $SQL_request .= "),\n"; 
      }
      $SQL_request = substr($SQL_request, 0, -2);
      $SQL_request .= ";\n";
      $SQL_requests .= $SQL_request . "\n";
    } 
  }
  
  // Save data into the file.
  // Set required writer type.
  if ($export_format != 'SQL') {
    $writer_type = tow_export_possible_formats('writer_type', $export_format);
    
    // Create writer.
    $obj_writer = PHPExcel_IOFactory::createWriter($obj_php_excel, $writer_type);
    // Set writertype-dependent parameters.
  }
  switch ($export_format) {
    case 'CSV': 
      $obj_writer->setDelimiter(';');
      $obj_writer->setEnclosure('');
      $obj_writer->setLineEnding("\r\n");
      break;
    case 'TXT/tab': 
      $obj_writer->setDelimiter("\t");
      $obj_writer->setEnclosure('');
      $obj_writer->setLineEnding("\r\n");
  }
  
  // Use writer to save file.
  switch ($export_format) {
    case 'PDF':
    case 'CSV': 
    case 'TXT/tab': 
    case 'XLS/mul':
    case 'XLS/sgl': 
    case 'XML': 
    case 'HTML':
      $obj_writer->save($temp_file);   
      break;
    case 'SQL':
      file_put_contents($temp_file, $SQL_requests);
  }  
  
  // Send file to the user.
  $file_headers = array(
    'Content-Type: octet/stream',
    'Content-Disposition: attachment; filename="' . $filename . '.' . $export_file_extension . '"',
    'Content-Length: ' . filesize($temp_file),
  );   
  file_transfer($temp_file, $file_headers);
  
  // Delete temporary file.
  unlink($temp_file);
}



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Returns the name of the format according to export type
 * or a set of possible formats of the type.
 */
function tow_export_possible_formats($value, $format_id = FALSE) {
  
  // Possible formats.
  $formats = array(
    'PDF' => array(
      'extension' => 'pdf', 
      'writer_type' => 'PDF', 
      'prompt' => 'PDF'
    ),
    'CSV' => array(
      'extension' => 'csv', 
      'writer_type' => 'CSV', 
      'prompt' => 'CSV'
    ),
    'TXT/tab' => array(
      'extension' => 'txt', 
      'writer_type' => 'CSV', 
      'prompt' => 'Text (tab-delimited)'
    ),
    'XLS/mul' => array(
      'extension' => 'xls', 
      'writer_type' => 'Excel5', 
      'prompt' => 'Excel (multiple worksheets)'
    ),
    'XLS/sgl' => array(
      'extension' => 'xls', 
      'writer_type' => 'Excel5', 
      'prompt' => 'Excel (all in one sheet)'
    ),
    'XML' => array(
      'extension' => 'xls', 
      'writer_type' => 'Excel2003XML', 
      'prompt' => 'XML'
    ),
    'SQL' => array(
      'extension' => 'sql', 
      'writer_type' => '', 
      'prompt' => 'SQL'
    ),
    'HTML' => array(
      'extension' => 'htm', 
      'writer_type' => 'HTML', 
      'prompt' => 'HTML'
    ),
  );
  
  // Get result.
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
    return FALSE;
  }
}



/****************************************************************************************************
 * Helper functions
 ****************************************************************************************************/

/**
 * Returns value suitable for SQL INSERT request.
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

/**
 * Returns SQL type corresponding to the $tow_type.
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
