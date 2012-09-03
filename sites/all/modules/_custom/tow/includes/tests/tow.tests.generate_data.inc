<?php

/**
 * @file
 * 
 * Data generation testing.
 */



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Page callback.
 */
function tow_tests_generate_data() {
  return drupal_get_form('tow_tests_generate_data_form');
}

/**
 * Form.
 */
function tow_tests_generate_data_form() {
  
  $form['user_qty'] = array(
    '#type' => 'textfield',
    '#description' => t('Enter no of new users'),
    '#size' => 5,
    '#maxlength' => 20,
    '#default_value' => 0,
  );

  $form['dataset_qty'] = array(
    '#type' => 'textfield',
    '#description' => t('Enter no of datasets for each user'),
    '#size' => 5,
    '#maxlength' => 20,
    '#default_value' => 1,
  );
  
  $form['table_qty'] = array(
    '#type' => 'textfield',
    '#description' => t('Enter no of tables in each dataset'),
    '#size' => 5,
    '#maxlength' => 20,
    '#default_value' => 5,
  );
  
  $form['max_columns'] = array(
    '#type' => 'textfield',
    '#description' => t('Enter the maximum no of fields in the table'),
    '#size' => 5,
    '#maxlength' => 20,
    '#default_value' => 8,
  );

  $form['record_qty'] = array(
    '#type' => 'textfield',
    '#description' => t('Enter no of records in each table'),
    '#size' => 5,
    '#maxlength' => 20,
    '#default_value' => 5,
  );

  $options = array();
  $aux = array_diff(scandir(drupal_get_path('module', 'tow') . '/docs'), array('.', '..', '.svn'));
  foreach ($aux as $fn) {
    $options[$fn] = $fn;
  }
  $form['source'] = array(
    '#type' => 'select',
    '#description' => t('Choose the file to generate data from'),
    '#options' => $options,
    '#default_value' => 0,
  );
  
  $form['desc'] = array(
    '#type' => 'item',
    '#value' => t('Confirm?'),
  );

  $form['truncate'] = array(
    '#type' => 'submit',
    '#value' => t('Yes'),
  );

  return $form;
}

/**
 * Submission callback.
 */
function tow_tests_generate_data_form_submit($form, &$form_state) {
  global $_tow_data;
  global $_tow_spaces;
  global $_tow_spaces_count;
  
  $time_start = time();
  
  // Reading text file with data for filling the database.
  $_tow_data = file_get_contents(drupal_get_path('module', 'tow') . '/docs/' . $form_state['values']['source']);
  
  $pos = 0;
  do {
    $_tow_spaces[]=$pos;
  } while ($pos = strpos($_tow_data, ' ', $pos + 1));
  $_tow_spaces[] = strlen($_tow_data);
  $_tow_spaces_count = count($_tow_spaces);

  // User defined parameters.
  $qty = array(
    'users' => $form_state['values']['user_qty'], 
    'datasets' => $form_state['values']['dataset_qty'],
    'tables' => $form_state['values']['table_qty'],
    'records' => $form_state['values']['record_qty'],
  );
  $max_columns = $form_state['values']['max_columns'];
  
  // Other parameters.
  $max_terms = 5; 
  $max_requesting_users = 10;
  $min_records = 5; 
  
  // Here the length means number of words.
  $max_leng = array(
    'dataset_title' => 4, 
    'table_title' => 4,
    'body' => 500, 
    'teaser' => 10, 
    'column_title' => 2, 
    'user_name' => 2, 
    'user_pass' => 1,
    'enum_option' => 2, 
    'char_field_data' => 5, 
    'taxonomy_term' => 2
  );
    
  // Get possible types for columns.
  $types = tow_possible_fields();
  foreach ($types as $value) {
    $deltas[$value] = 0;
  }
  $types_qty = count($types);
  
  // Get Vocabulary ID.
  $vid_res = db_query("SELECT vid FROM {vocabulary} WHERE name = '%s'", 'Dataset categories');
  $vid_category = db_fetch_array($vid_res);
  $vid_category = $vid_category['vid']; 
  $vid_res = db_query("SELECT vid FROM {vocabulary} WHERE name = '%s'", 'Dataset tags');
  $vid_tag = db_fetch_array($vid_res);
  $vid_tag = $vid_tag['vid']; 
  
  // Get list of taxonomy terms.
  $term_res_category = db_query("SELECT tid, name, vid, description, weight FROM {term_data} WHERE vid = %d", $vid_category);
  $term_res_tag = db_query("SELECT tid, name, vid, description, weight FROM {term_data} WHERE vid = %d", $vid_tag);
  $term_qty_category = 0;
  $term_qty_tag = 0;
  $terms_category = array();
  $terms_tag = array();
  while ($term = db_fetch_array($term_res_category)) {
    $terms_category[] = $term;
    $term_qty_category++; 
  }
  while ($term = db_fetch_array($term_res_category)) {
    $terms_tag[] = $term;
    $term_qty_tag++; 
  }
  
  // Creating users.
  for ($uid = 1; $uid <= $qty['users']; $uid++) {
    $user_acc = new stdClass();
    $user_param['name'] = _tow_tests_get_random_string($max_leng['user_name']);
    $user_param['mail'] = $user_param['name'] . "@milo.com";
    $user_param['status'] = 1;
    $user_param['pass'] = md5(_tow_tests_get_random_string($max_leng['user_pass']));
    $user_param['created'] = time() - mt_rand(10, 2 * pow(10, 6));
    user_save($user_acc, $user_param);
  }
  
  // Get all user names from the database.
  $user_res = db_query("SELECT uid, name FROM {users} WHERE status = %d", 1);
  $user_qty = 0;
  while ($user = db_fetch_array($user_res)) {
    $users[] = $user;
    $user_qty++;
  }
  
  drupal_set_message($qty['users'] . " new users created, " . $user_qty . " users are used in total.");
  
  // For each user create datasets.
  for ($user_index = 0; $user_index < $user_qty; $user_index++) {
    
    $uid = $users[$user_index]['uid'];
    $user_name = $users[$user_index]['name'];
  
    for ($did = 1; $did <= $qty['datasets']; $did++) {
      
      // Set Dataset type, title, uid, body, teaser.
      $new_dataset = new stdClass();
      $new_dataset->type = 'dataset';
      $new_dataset->title = _tow_tests_get_random_string($max_leng['dataset_title']);  
      $new_dataset->uid = $uid;
      $new_dataset->name = $user_name;
      $new_dataset->body = _tow_tests_get_random_string($max_leng['body']);
      $new_dataset->field_access_type[0]['value'] = rand(0, 2);
      $new_dataset->created = mt_rand(strtotime('2011-01-01 00:00:00'), strtotime('2011-12-31 24:59:59'));
      $new_dataset->changed = $new_dataset->created + mt_rand(0, 1036800);
      
      // Generate terms for dataset.
      $term_id = mt_rand(0, $term_qty_category - 1);
      $new_dataset->taxonomy[$terms_category[$term_id]['tid']] = new stdClass();
      foreach ($terms_category[$term_id] as $key => $value) {
        $new_dataset->taxonomy[$terms_category[$term_id]['tid']]->{$key} = $value;
      }
      $dataset_term_qty = mt_rand(1, $max_terms);
      for ($term_index = 1; $term_index <= $dataset_term_qty; $term_index++ ) {
        $term_id = mt_rand(0, $term_qty_tag);

        // Check if a new taxonomy term should be created.
        if ($term_id == $term_qty_tag) {
          
          // Create if needed.
          unset($term);
          $term = array(
            'vid' => $vid_tag,
            'name' => _tow_tests_get_random_string($max_leng['taxonomy_term']),
            'description' => _tow_tests_get_random_string($max_leng['teaser']),
          );
          
          // Add the new term to the Vocabulary.
          taxonomy_save_term($term);
          
          // Add the new term to the term list.
          $terms_tag[$term_id]['tid'] = $term['tid'];
          $terms_tag[$term_id]['name'] = $term['name'];
          $term_qty_tag++;
        }
        
        // Add the term to the dataset.
        $new_dataset->taxonomy[$terms_tag[$term_id]['tid']] = new stdClass();
        foreach ($terms_tag[$term_id] as $key => $value) {
           $new_dataset->taxonomy[$terms_tag[$term_id]['tid']]->{$key} = $value;
        }
      }

      // Save node, get dataset nid.
      try {
        node_save($new_dataset);
      }
      catch (Exception $e) {
        watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
      }
      $dataset_nid = $new_dataset->nid;

      // Generate access requests for the dataset.
      $requesting_users_qty = rand(0, min(array($max_requesting_users, floor($user_qty / 2))));
      if ($requesting_users_qty) {
      
        // $ruids is an array of uids, that have already requested access.
        $ruids = array($uid); 
        for ($requesting_user_index = 1; $requesting_user_index <= $requesting_users_qty; $requesting_user_index++) {
          
          // generate new $ruid, it should be unique, not in $ruids yet
          do {
            $ru_index = mt_rand(0, $user_qty-1);
          } while (in_array($users[$ru_index]['uid'], $ruids));
          $ruids[] = $users[$ru_index]['uid'];
          
          // Generate status for the request.
          $status = mt_rand(0, 2) - 1;
          db_query("INSERT INTO {tow_access_requests} (nid, uid, status, request_timestamp) VALUES ('%d', '%d', '%d', '%d')", 
            $dataset_nid, $users[$ru_index]['uid'], $status, time() - mt_rand(1, 2 * pow(10, 6)));
        }
      }
      
      // For each dataset create tables.
      for ($tid = 1; $tid <= $qty['tables']; $tid++) {
        
        // Set table type, title,  uid, body, teaser.
        $new_table = new stdClass();
        $new_table->type = 'table';
        $new_table->title = _tow_tests_get_random_string($max_leng['table_title']);  
        $new_table->uid = $uid;
        $new_table->body = _tow_tests_get_random_string($max_leng['body']);
        $new_table->teaser = _tow_tests_get_random_string($max_leng['teaser']);
        
        // Generate columns, their types and titles.
        $columns_qty = mt_rand(3, $max_columns);
        $column_type = array();
        $indexes_array = array();
         
        for ($col_index = 0; $col_index <= $columns_qty - 1; $col_index++) {
          
          // Types of columns.
          $type_id = mt_rand(0, $types_qty - 1);
          $column_type[$col_index]['type_id'] = $type_id ;
          $field_type = "field_title_" . $types[$type_id];
          $column_type[$col_index]['type'] = $types[$type_id];
          
          // Column titles.
          $new_table->{$field_type}[]['value'] = _tow_tests_get_random_string($max_leng['column_title']);
          
          // Column unit.
          $new_table->field_units[]['value'] = _tow_tests_get_random_string(1);
       
          // Enum options if needed.
          if ($field_type == "field_title_enum") {
            $new_table->{field_enum_options}[]['value']= serialize(array(
              _tow_tests_get_random_string($max_leng['enum_option']),
              _tow_tests_get_random_string($max_leng['enum_option']), 
              _tow_tests_get_random_string($max_leng['enum_option'])
            ));
          }
          $indexes_array[] = mt_rand(0, 1);
        }

        // Weights generation.
        for ($col_index = 0; $col_index <= $columns_qty-1; $col_index++) {
          $weights_array[$col_index] = $col_index;
        }
        $new_table->field_weights[0]['value'] = serialize($weights_array);
        $new_table->field_dataset[0]['value'] = $dataset_nid;
        $new_table->field_indexes[0]['value'] = serialize($indexes_array);
        
        // Save node, get table nid.
        try {
          node_save($new_table);
        }
        catch (Exception $e) {
          watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
        }
        $table_nid = $new_table->nid;

        // For each table create records.
        $qty_records = ($table_nid) ? mt_rand($min_records, $qty['records']) : 0;
        for ($rid = 1; $rid <= $qty_records; $rid++) {

          // Type. 
          $new_record = new stdClass();
          $new_record->type = 'record'; 
          $new_record->uid = $uid;
   
          // Data.
          for ($col_index = 0; $col_index <= $columns_qty - 1; $col_index++) {
            switch ($column_type[$col_index]['type']) {
              case 'int':
                $value = mt_rand(0, 1000);
                break;
              case 'float':
                $value = mt_rand(0, 1000000) / mt_rand(1, 1000000);
                break;
              case 'char':
              case 'text':
                $value = _tow_tests_get_random_string(mt_rand(5, $max_leng['char_field_data']));
                break;
              case 'enum':
                $value = mt_rand(0, 2);
                break;
              case 'bool':
                $value = mt_rand(0, 1);
                break;
              case 'time':
                $value =  date('H:i:s', mt_rand(0, 84600));
                break;
              case 'date':
                $value = date('Y-m-d', mt_rand(1000000, 3000000));
                break;
              case 'datetime':
                $value =  date('Y-m-d H:i:s', mt_rand(1000000, 3000000));
                break;
              case 'timestamp':
                $value = time() - mt_rand(0, pow(10, 6));
                break;
              case 'code':
                $value = substr($_tow_data, $data_index++, 3);
               break;  
            }
            $new_record->{"field_" . $column_type[$col_index]['type']}[]['value'] = $value;
          }
          $new_record->field_table[0]['value'] = $table_nid;
          try {
            node_save($new_record);
          }
          catch (Exception $e) {
            watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
          }
        }
      }
      
      // Reset counter if needed.
      if ($data_index > 1500000) {
        $data_index = 1;
      }
    }
  }
  
  // Messages.
  $message = $qty['datasets'] . " datasets for each user, " . $qty['tables'] . " tables in each dataset, " . $qty['records'] . " records in each table created.";
  drupal_set_message(filter_xss($message));
  $message = $qty['datasets'] * $qty['tables'] * $qty['records'] * $user_qty . " nodes in total.";
  drupal_set_message(filter_xss($message));

  $elapsed_time = time() - $time_start;
  $message = "Elapsed time: " . $elapsed_time . " seconds.";
  drupal_set_message(filter_xss($message));
}
