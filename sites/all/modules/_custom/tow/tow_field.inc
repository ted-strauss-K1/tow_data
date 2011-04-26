<?php
// $Id: tow_field.inc,v 1.0.0.0 2011/02/28 23:42:15 tow Exp $
/**
 * @file
 * 
 */

/**
 * @return confirmation form for field deletion
 */
function tow_field_delete_confirm($form_state, $table_nid, $type, $delta, $index) {

  global $user;
  global $table;
  global $dataset;
  
  $table = node_load($table_nid);
  if ($user->uid != $node->uid && !user_access('edit any table content')) {
    drupal_access_denied();
    die();
  }
  if (!$table->{'field_title_' . $type}[$delta]['value']) {
    drupal_not_found();
    die();
  }
  $dataset = node_load($table->field_dataset[0]['nid']);
     
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $type
  );
  $form['delta'] = array(
    '#type' => 'value',
    '#value' => $delta
  );
  $form['field_index'] = array(
    '#type' => 'value',
    '#value' => $index
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete field %field?',
      array('%field' => $table->{'field_title_' . $type}[$delta]['value'])
    ),
    'node/'. $table_nid,
    t('Deletion of field may cause loss of data'),
    t('Confirm'),
    t('Cancel')
  ); 

}

/**
 * Submission callback
 */
function tow_field_delete_confirm_submit($form, &$form_state) {
  
  global $table;

  $type = $form_state['values']['type'];
  $delta = $form_state['values']['delta'];
  $field_index = $form_state['values']['field_index'];
  
  db_query("
    DELETE FROM {content_field_%s} WHERE nid IN
    (
      SELECT nid FROM content_type_record WHERE field_table_nid = %d
    )
  AND delta = %d
 ", $type, $table->nid, $delta);
  
  unset($table->{'field_title_' . $type}[$delta]);
  if ($type = 'enum')
    unset($table->field_enum_options[$delta]);
  
  $weights = unserialize($table->field_weights[0]['value']);
  $current_weight = $weights[$field_index];  
  unset($weights[$field_index]);
  foreach ($weights as $i => $weight)
    if($weight > $current_weight)
      $weights[$i] --;
  $table->field_weights[0]['value'] = serialize(array_values($weights));
  
  $indexes = unserialize($table->field_indexes[0]['value']);
  unset($indexes[$field_index]);
  $table->field_indexes[0]['value'] = serialize($indexes);
  
  node_save($table);
    
  drupal_set_message('The field has been deleted');
  $form_state['redirect'] = 'node/' . $table->nid;
}

/**
 * Interface for field editing
 */
function tow_field_edit_form($form_state, $table_nid, $type, $delta, $index) {
  
  global $user;
  global $table;
  global $dataset;
  
  $table = node_load($table_nid);
  if ($user->uid != $node->uid && !user_access('edit any table content')) {
    drupal_access_denied();
    die();
  }
  if (!$table->{'field_title_' . $type}[$delta]['value']) {
    drupal_not_found();
    die();
  }
  $dataset = node_load($table->field_dataset[0]['nid']);

  $form['current_type'] = array(
    '#type' => 'value',
    '#value' => $type
  );  
  $form['delta'] = array(
    '#type' => 'value',
    '#value' => $delta
  );  
  $form['field_index'] = array(
    '#type' => 'value',
    '#value' => $index
  );
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Field title'),
    '#default_value' => $table->{'field_title_' . $type}[$delta]['value'],
  );
  
  $possible_fields = tow_possible_fields();
  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => $possible_fields,
    '#default_value' => array_search($type, $possible_fields)
  );
  
  $index_array = unserialize($table->field_indexes[0]['value']);
  $form['index'] = array(
    '#type' => 'checkbox',
    '#title' => t('Index'),
    '#default_value' => $index_array[$index]
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  
  $form['#redirect'] = 'node/' . $table_nid;
  
  return $form;
}

/**
 * Submission callback
 */
function tow_field_edit_form_submit($form, &$form_state) {
  global $table;
  
  // Update index array of the table
  $index_array = unserialize($table->field_indexes[0]['value']);
  $index_array[$form_state['values']['field_index']] = $form_state['values']['index'];
  $table->field_indexes[0]['value'] = serialize($index_array);
  
  // Update types of the table fields
  $field_types = tow_possible_fields(); 
  
$table_nid = $form_state['values']['table_nid'];
$field_title = $form_state['values']['title'];
$field_type_old = $form_state['values']['current_type'];
$field_type_new = $field_types[$form_state['values']['type']];
$field_type_old_delta = $form_state['values']['delta'];

$field_type_new_delta = 2;

// 1 Switch title 
// 1.1 Take title from old type table (not needed, already selected)
/*    $title_res = db_query("SELECT field_title_%s_value FROM {content_field_title_%s} WHERE nid = %d AND delta = %d",
    $field_type_old, $field_type_old, $table_nid, $field_type_old_delta);
  */  

/*  db_query("DELETE FROM {content_field_title_%s} WHERE vid = %d AND delta = %d",
    $field_type_old, $table_nid, $field_type_old_delta); */

/*  db_query("INSERT INTO {content_field_title_%s} (vid, nid, delta, content_field_title_%s) 
  	VALUES (%d, %d, %d, %s)", $field_type_new, $field_type_new, $table_nid, $table_nid, 
    $field_type_new_delta, $field_type_old_delta);
*/  
// 2 Move data
  // 2.0 Check data types
  $warn = FALSE;
  switch ($field_type_new) {
    case 'text':
      $conversion_procedure = '$new_value = $old_value;';
      break;
    case 'enum':
      $warn = TRUE;
      // $err_field_names[] = "count_fields";
      $warn_msgs[] = t("No type may be converted to enum.");
      $conversion_procedure = '$new_value = NULL;';
      break;
    case 'char':
      $conversion_procedure = '$new_value = $old_value;';
      if ($field_type_old == 'text') {
        $warn = TRUE;
        $warn_msgs[] = t("Long text may be cropped while converting to char.");
        $conversion_procedure = '$new_value = substr($old_value, 0, 255);';
      } 
      break;
    case 'code':
      if ($field_type_old == 'date' || $field_type_old == 'datetime' || 
        $field_type_old == 'time' || $field_type_old == 'timestamp') {
        $warn = TRUE;
        $warn_msgs[] = t("Neither time nor date may be converted to code.");
        $conversion_procedure = '$new_value = NULL;';
      }
      else {
        $warn = TRUE;
        $warn_msgs[] = t("Only first three symbols will survive when converting type to code.");
        $conversion_procedure = '$new_value = substr($old_value, 0, 3);';
      }
      break;
    case 'bool':
      if ($field_type_old == 'date' || $field_type_old == 'datetime' || 
        $field_type_old == 'time' || $field_type_old == 'timestamp') {
        $warn = TRUE;
        $warn_msgs[] = t("Neither time nor date may be converted to bool.");
        $conversion_procedure = '$new_value = NULL;';
        }
      else {
        $warn = TRUE;
        $warn_msgs[] = t("Only values '1', '0', 'true', 'false', 'yes', 'no' will be converted to bool.");
        $conversion_procedure = <<<CONV
		switch (strtolower($old_value)) {
          case '1':
          case 'true':
      	  case 'yes':
      		$new_value = TRUE;
      		break;
          case '0':
          case 'false':
      	  case 'no':
      	    $new_value = FALSE;
      		break;
      	  default:
      	  $new_value = NULL;
      	}
CONV;
      }
      break;
    default:
      switch ($field_type_old) {
        case 'char':
        case 'code':
        case 'text':
        case 'enum':
          if ($field_type_new == 'int' || $field_type_new == 'float') {
            $warn = TRUE;
            $warn_msgs[] = t(ucfirst($field_type_old) . " may not always be converted to decimal." );
            $conversion_procedure = '$new_value = (' . $field_type_new . ')$old_value;';
          }
          else {
            $warn = TRUE;
            $warn_msgs[] = t(ucfirst($field_type_old) . " may not be converted neither to date nor to time.");
            $conversion_procedure = '$new_value = NULL;';
          }
          break;
        case 'bool':
          if ($field_type_new == 'int' || $field_type_new == 'float') {
            $conversion_procedure = '$new_value = (' . $field_type_new . ')$old_value;';
          }
          else {
            $warn = TRUE;
            $warn_msgs[] = t("Bool may not be converted neither to date nor to time.");
            $conversion_procedure = '$new_value = NULL;';
          }
          break;
        case 'date':
        case 'time':
          $warn = TRUE;
          $warn_msgs[] = t(ucfirst($field_type_old) . " may not be converted to other type.");
          $conversion_procedure = '$new_value = NULL;';
          break;
        case 'int':
          if ($field_type_new == 'float') {
            $conversion_procedure = '$new_value = (float)$old_value;';
          }
          else {
            $warn = TRUE;
            $warn_msgs[] = t("Int may not be converted neither to date nor to time.");
            $conversion_procedure = '$new_value = NULL;';
            }
          break;
        case 'float':
          if ($field_type_new == 'int') {
            $warn = TRUE;
            $warn_msgs[] = t("Float may not always be converted to integer.");
            $conversion_procedure = '$new_value = (int)$old_value;';
          }
          else {
            $warn = TRUE;
            $warn_msgs[] = t("Float may not be converted neither to date nor to time.");
            $conversion_procedure = '$new_value = NULL;';
          }
          break;
        case 'datetime':
          if ($field_type_new == 'timestamp') {
            $conversion_procedure = '$new_value = $old_value;';
          }
          else {
            $warn = TRUE;
            $warn_msgs[] = t("Datetime may be converted only to timestamp.");
            $conversion_procedure = '$new_value = NULL;';
          }
          break;    
        case 'timestamp':
          if ($field_type_new == 'datetime') {
            $conversion_procedure = '$new_value = $old_value;';
          }
          else {
            $warn = TRUE;
            $warn_msgs[] = t("Datetime may be converted only to datetime.");
            $conversion_procedure = '$new_value = NULL;';
          }
          break;  
      }
  }
  
  //_tow_show_conversion_warning($warn, $err_field_names, $warn_msgs);   
  
  $table_node = node_load($table_nid);
  
  $weights_array = unserialize($table_node->field_weights[0]['value']);

  $index = 0;
  foreach ($field_types as $field_type) {
    $delta[$field_type] = -1;
    if (!is_array($table_node->{'field_title_' . $field_type}))
      continue;
    foreach($table_node->{'field_title_' . $field_type} as $title_value) {
      if ($title_value['value']) {
        $delta[$field_type]++;
        if (($field_type == $field_type_old) && ($delta[$field_type] == $field_type_old_delta)) {
         /* $aux_array[$weights_array[$index]]['title'] = $title_value['value'];
          $aux_array[$weights_array[$index]]['delta'] = $delta[$field_type];
          $aux_array[$weights_array[$index]]['type'] = $field_type;
         */ $pos = $index;
        }
        else {
          $aux_array[$weights_array[$index]]['title'] = $title_value['value'];
          $aux_array[$weights_array[$index]]['delta'] = $delta[$field_type];
          $aux_array[$weights_array[$index]]['type'] = $field_type;
        };        
        $index++;
        
      }
    }
  }

  if ($aux_array)
    ksort($aux_array);
  
  $delta[$field_type_new]++;
  $aux_array[$weights_array[$pos]]['title'] = $field_title;
  $aux_array[$weights_array[$pos]]['delta'] = $delta[$field_type_new];
  $aux_array[$weights_array[$pos]]['type'] = $field_type_new;
  
  
    
  foreach ($field_types as $type_key => $type) {
  //  $delta = 0;
    //$enum_delta = 0;
    foreach ($aux_array as $index => $field) {
      if ($field['type'] == $type) {
        //$form_state['values']['field_title_' . $type][$delta]['value'] = $field['title'];
        //$form_state['values']['field_title_' . $type][$delta]['_weight'] = $delta;
//        if ($type == 'enum') {
//        $form_state['values']['field_enum_options'][$delta]['value'] = serialize(explode("\r\n", $field['enum_options']));
//          $form_state['values']['field_enum_options'][$delta]['_weight'] = $delta;
//          $enum_delta++;
//        }
        //$delta++;
        $new_weights[] = $index;
//        $index++;
      }
    }
  /*  for ($i = $form_state['values']['count_fields']; $i > $delta; $i--) {
      unset($form_state['values']['field_title_' . $type][$i]);
    }
    if ($type == 'enum') {
      for ($i = $form_state['values']['count_fields']; $i > $enum_delta; $i--) {
        unset($form_state['values']['field_enum_options'][$i]);
      }
    }
  */}




  // 1.1 Compute new delta
  //$field_type_new_delta = max(keys($table_node->{"field_title_" . $field_type_new})) + 1;
  // 1.2 Record title into new type table  
  $table_node->{"field_title_" . $field_type_new}[$delta[$field_type_new]]['value'] = 
    $table_node->{"field_title_" . $field_type_old}[$field_type_old_delta]['value'];
  // 1.3 Delete title from old type table
  unset($table_node->{"field_title_" . $field_type_old}[$field_type_old_delta]);
  $table_node->field_weights[0]['value'] = serialize($new_weights);
  
  node_save($table_node);
/*
  $res_old_records = db_query("SELECT * FROM {content_field_%s} WHERE `nid` IN 
    (SELECT `nid` FROM {content_type_record} WHERE `field_table_nid` = %d) AND `delta` = %d", 
    $field_type_old, $field_type_old, $table_nid, $field_type_old_delta); 
  
  while ($res = db_fetch_array($res_old_records)){
    $old_value = $res["field_" . $field_type_old . "_value"];
    $new_value = 
    
    
    db_query("INSERT INTO {content_field_%s} (vid, nid, delta, content_field_%s) 
  	VALUES (%d, %d, %d, %s)", $field_type_new, $field_type_new, $res['nid'], $res['vid'], 
    $field_type_new_delta, $new_value);
    
  }
    */
  // 2.0.1 Show warning if data may be lost
  // 2.0.2 Define if data conversion is required (or we just need to erase old data after user confirmation) 
  // 2.1 Read data from old type data table (if requied)
  // 2.2 Delete data from old type data table
  // 3. Convert data into new type
  // 4. Record new data into data table. 
  
  node_save($table);
}