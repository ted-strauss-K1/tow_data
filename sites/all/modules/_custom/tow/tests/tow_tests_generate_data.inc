<?php

//include_once('./' . drupal_get_path('module', 'tow') . '/tests/tow_tests_truncate_cck.inc');

function tow_tests_generate_data() {
  return drupal_get_form('tow_tests_generate_data_form');  
}

function tow_tests_generate_data_form(){
   $form['user_qty'] = array(
      '#type' => 'textfield',
      '#description' => t('Enter no of new users'),
      '#size' => 5,
      '#maxlength' => 20,
  	  '#default_value' => 2,
  );
  $form['dataset_qty'] = array(
      '#type' => 'textfield',
      '#description' => t('Enter no of datasets for each user'),
      '#size' => 5,
      '#maxlength' => 20,
  	  '#default_value' => 2,
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
      '#description' => t('Enter the maximum no of columns in the table'),
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
 
  $form['desc'] = array(
    '#type' => 'item',
    '#value' => 'Confirm?'
  );
  $form['truncate'] = array(
    '#type' => 'submit',
    '#value' => 'Yes',
  );
  return $form;
}

function _tow_tests_get_random_string($str_len){
  global $data;
  global $data_index;
  $res = substr($data, $data_index++, $str_len);
  $pattern = '/[\n\r ]/';
  $replacement = '';
  $res = preg_replace($pattern, $replacement, $res);
  return $res;
} 

function tow_tests_generate_data_form_submit($form, &$form_state){
  $time_start = time();
  
  // Reading text file with data for filling the database 
  global $data;
  global $data_index;
  $data = file_get_contents('./' . drupal_get_path('module', 'tow') . '/tests/data.txt');
  $data_index = mt_rand(0, 10000);

  // User defined parameters
  $qty = array( 'users' => $form_state['values']['user_qty'], 
  	'datasets' => $form_state['values']['dataset_qty'],
  	'tables' => $form_state['values']['table_qty'],
    'records' => $form_state['values']['record_qty']);
  $max_columns = $form_state['values']['max_columns'];;
  
  // Other parameters
  $max_terms = 5; 
  $max_requesting_users = 10;
  $max_leng = array( 'dataset_title' => 10, 'table_title' => 10,
  	'body' => 32, 'teaser' => 15, 'column_title' => 9, 'user_name' => 7, 'user_pass' => 5,
  	'enum_option' => 6, 'char_field_data' => 15, 'taxonomy_term' => 8);
  $time_value_options = array(15, 453, 457, 457, 3245, 2357, 467, 7445, 457, 346);
  $date_value_options = array(105, 453, 45, 4357, 35, 2357, 467, 77445, 4857, 346);
  $timedate_value_options = array(915, 453, 4567, 457, 345, 257, 4657, 7745, 457, 3446);
  
  // Get possible types for columns
  $types = tow_possible_fields();
  foreach ($types as $value){
    $deltas[$value] = 0;
  }
  $types_qty = count($types);
  
  // Get Vocabulary ID
  $vid_res = db_query("SELECT `vid` FROM {vocabulary} WHERE `name`='Dataset categories'");
  $vid = db_fetch_array($vid_res);
  $vid = $vid['vid']; 
  
  // Get list of taxonomy terms
  $term_res = db_query("SELECT `tid`, `name`, `vid`, `description`, `weight` FROM {term_data} WHERE `vid`=%d", $vid);
  $term_qty = 0;
  while ($term = db_fetch_array($term_res)){
    $terms[] = $term;
    $term_qty++; 
  }
  
  // Creating users
  for ($uid = 1; $uid <= $qty['users']; $uid++) {
    $user_acc = new stdClass();
    $user_param['name'] = _tow_tests_get_random_string($max_leng['user_name']);
    $user_param['mail'] = $user_param['name'] . "@milo.com";
    $user_param['status'] = 1;
    $user_param['pass'] = md5(_tow_tests_get_random_string($max_leng['user_pass']));
    $user_param['created'] = time() - mt_rand(10, 2*pow(10, 6));
    user_save($user_acc, $user_param);
  }
  
  
  // 1. Get all user names from database 
  $user_res = db_query("SELECT `uid`, `name` FROM {users} WHERE `status`=1");
  $user_qty = 0;
  while ($user = db_fetch_array($user_res)){
    $users[] = $user;
    $user_qty++; 
  }
  
  drupal_set_message($qty['users'] . " new users created, " . $user_qty . " users in total.");
  
  // 2. For each user create datasets
  for($user_index = 0; $user_index < $user_qty; $user_index++) {
    $uid = $users[$user_index]['uid'];
    $user_name = $users[$user_index]['name'];
  // 2.1 Set Dataset type, title, uid, body, teaser
    for($did = 1; $did <= $qty['datasets']; $did++) {
      $new_dataset = new stdClass();
      $new_dataset->type = 'dataset';
      $new_dataset->title = _tow_tests_get_random_string($max_leng['dataset_title']);  
      $new_dataset->uid = $uid;
      $new_dataset->name = $user_name;
      $new_dataset->body = _tow_tests_get_random_string($max_leng['body']);
      $new_dataset->teaser = _tow_tests_get_random_string($max_leng['teaser']);
      $new_dataset->field_access_type[0]['value'] = rand(0, 2);
      
      // Generate terms for dataset  
      $dataset_term_qty = mt_rand(1, $max_terms);
      
      for ($term_index = 1; $term_index <= $dataset_term_qty; $term_index++ ) {
        $term_id = mt_rand(0, $term_qty);

        // check if new taxonomy term should be created
        if ($term_id == $term_qty) {
          // create if needed
          unset($term);
          $term = array(
    		'vid' => $vid, // Voacabulary ID
    		'name' => _tow_tests_get_random_string($max_leng['taxonomy_term']), // Term Name
          );
          // add new term to the Vocabulary
          taxonomy_save_term($term);
          // add new term to the term list 
          $terms[$term_id]['tid'] = $term['tid'];
          $terms[$term_id]['name'] = $term['name'];
          $term_qty++;
        }  
//      add term to the dataset  
        $new_dataset->taxonomy[$terms[$term_id]['tid']] = new stdClass();
        foreach ($terms[$term_id] as $key => $value) {
           $new_dataset->taxonomy[$terms[$term_id]['tid']]->{$key} = $value;
        }
      }
  // 2.2 node_save, get dataset nid 
      node_save($new_dataset);
      $dataset_nid = $new_dataset->nid;

      // Generate access requests for the dataset
      $requesting_users_qty = rand(0, min(array($max_requesting_users, floor($user_qty/2))));
      if ($requesting_users_qty) {
        // $ruids - array of uids, that have already requested access
        $ruids = array($uid); 
        for ($requesting_user_index = 1; $requesting_user_index <= $requesting_users_qty; $requesting_user_index++){
          // generate new $ruid, it should be unique, not in $ruids yet
          do {
            $ruid = mt_rand(0, $user_qty-1);
          } while (in_array($ruid, $ruids));
          $ruids[] = $ruid;
          // generate status for the request
          $status = mt_rand(0, 2) - 1;
          db_query("INSERT INTO {tow_access_requests} (nid, uid, status, request_timestamp) VALUES (%d, %d, %d, %d)", 
            $dataset_nid, $users[$requesting_user_index]['uid'], $status, time() - mt_rand(1, 2*pow(10, 6)));
        }  // for 
      } // if 
      
  // 3. For each dataset create tables
      for ($tid = 1; $tid <= $qty['tables']; $tid++) {
        // 3.1 Set table type, title,  uid, body, teaser
        $new_table = new stdClass();
        $new_table->type = 'table';
        $new_table->title = _tow_tests_get_random_string($max_leng['table_title']);  
        $new_table->uid = $uid;
        $new_table->body = _tow_tests_get_random_string($max_leng['body']);
        $new_table->teaser = _tow_tests_get_random_string($max_leng['teaser']);
        
      // 3.2 generate columns, their types and titles
        $columns_qty = mt_rand(3, $max_columns);
        $column_type = array();
        
        for ($col_index = 0; $col_index <= $columns_qty-1; $col_index++) {
          //3.3 types of columns
          $type_id = mt_rand(0, $types_qty-1);
          $column_type[$col_index]['type_id'] = $type_id ;
          $field_type = "field_title_" . $types[$type_id];
          $column_type[$col_index]['type'] = $types[$type_id];
          // 3.4 column titles
          $new_table->{$field_type}[]['value'] = _tow_tests_get_random_string($max_leng['column_title']);
           // 3.5 enum options if needed
          if ($field_type == "field_title_enum") {
            $new_table->{field_enum_options}[]['value']= 
              serialize(array(_tow_tests_get_random_string($max_leng['enum_option'] ),
                              _tow_tests_get_random_string($max_leng['enum_option'] ), 
                              _tow_tests_get_random_string($max_leng['enum_option'] )));
          }
        }
          // TODO: random weights 
        //   3.6 field_dataset_nid, weights generate
        for ($col_index = 0; $col_index <= $columns_qty-1; $col_index++) {
          $weights_array[$col_index] = $col_index;
        }
        $new_table->field_weights[0]['value'] = serialize($weights_array);
        $new_table->field_dataset[0]['nid'] = $dataset_nid;
        // 3.7 node save, get table nid
        node_save($new_table);
        $table_nid = $new_table->nid;

// 4. For each table create records
        for ($rid = 1; $rid <= $qty['records']; $rid++) {
// 4.1 type, 
          $new_record = new stdClass();
          $new_record->type = 'record'; 
          $new_record->uid = $uid;
   // 4.2 data        
           for ($col_index = 0; $col_index <= $columns_qty-1; $col_index++) {
             switch ($column_type[$col_index]['type']) {
               case 'int':
                 $value = mt_rand(0, 1000);
                 break;
               case 'float':
                 $value = mt_rand(0, 1000000)/mt_rand(1, 1000000);
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
                 $value = $time_value_options[mt_rand(0, 9)];
                 break;
               case 'date':
                 $value = $date_value_options[mt_rand(0, 9)];
                 break;
               case 'datetime':
                 $value = $datetime_value_options[mt_rand(0, 9)];
                 break;
               case 'timestamp':
                 $value = $datetime_value_options[mt_rand(0, 9)];
                 break;
               case 'code':
                 $value = substr($data, $data_index++, 3);
                 break;  
             }
             $new_record->{"field_" . $column_type[$col_index]['type']}[]['value'] = $value;
           }
           $new_record->field_table[0]['nid'] = $table_nid;
          node_save($new_record);
        } // end of records 
      } // end of tables
      
      // reset counter if needed
      if ($data_index > 1500000) {
        $data_index = 1;
      }
    } // end of datasets
  }// end of users
  drupal_set_message($qty['datasets'] . " datasets for each user, " . $qty['tables'] . " tables in each dataset, " .
    $qty['records'] . " records in each table created.");
  drupal_set_message($qty['datasets']*$qty['tables']*$qty['records']*$user_qty . " nodes in total.");

  $elapsed_time = time() - $time_start;
  drupal_set_message("Elapsed time: " . $elapsed_time . " seconds.");
  //drupal_goto('<front>');
}