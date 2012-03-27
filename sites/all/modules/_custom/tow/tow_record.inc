<?php
// $Id: tow_record.inc,v 1.0.0.0  2011/02/28 15:32:21 tow Exp $

/**
 * @file
 */

/**
 * @param unknown_type $form
 * @param unknown_type $form_state
 */
function tow_record_form_alter(&$form, &$form_state) {

  global $dataset;
  global $table;
  global $record;
  $record = TRUE;
  
  // remove title, menu options, revision information, comment settings, publishing options
  unset($form['title']);
  unset($form['menu']);
  unset($form['revision_information']);
  unset($form['comment_settings']);
  //unset($form['author']); //TODO: hide this field but place user id like field_table?
  unset($form['options']);
  unset($form['buttons']['preview']);

  // hide native cck fields
  foreach (tow_possible_fields() as $field_type)
    $form['field_' . $field_type]['#access'] = FALSE;

  // get table node
  if ($_GET['table'])
    $table = node_load($_GET['table']);
  else
    $table = node_load($form['#node']->field_table[0]['value']);
  if (!$table)
    return;

  $dataset = node_load($table->field_dataset[0]['value']);
  $dataset = (object)array('nid' => $dataset->nid, 'title' => $dataset->title);

  $form['table_nid'] = array(
    '#type' => 'value',
    '#value' => $table->nid
  );
  
  $form['field_table']['#access'] = FALSE;
    
  // add tabular form containing cck fields ordered by weight
  $form['fields'] = array(
    '#theme' => 'tabular_form',
    '#title' => 'Fields',
    '#tree' => true,
  );
  
  $form['fields']['header'] = array(
    '#type' => 'value',
    '#value' => array('Title', 'Type', 'Value'),
  );
  
  $form['delta'] = array(
    '#tree' => TRUE
  );

  $weights_array = unserialize($table->field_weights[0]['value']);
  if ($weights_array) {   
    $index = 0;
    foreach (tow_possible_fields() as $field_type) {
      $delta = 0;
      if (!is_array($table->{'field_title_' . $field_type}))
        continue;
      foreach($table->{'field_title_' . $field_type} as $title_value) {
        if ($title_value['value']) {
          $aux_array[$weights_array[$index]]['title'] = $title_value['value'];
          $aux_array[$weights_array[$index]]['type'] = $field_type;
          $aux_array[$weights_array[$index]]['value'] = $form['field_' . $field_type][$delta]['#default_value']['value'];
          $aux_array[$weights_array[$index]]['delta'] = $delta;
          $index++;
          $delta++;
        }
      }
    }
    ksort($aux_array);
  
    foreach($aux_array as $key => $value) {
      $form['fields']['data'][$key]['title'] = array(
        '#type' => 'item',
        '#value' => $value['title'],
      );
      $form['fields']['data'][$key]['type'] = array(
        '#type' => 'item',
        '#value' => $value['type'],
      );
      
      switch ($value['type']) {//TODO: control size of inputs depending on type?
        case 'text':
          $input_type = 'textarea';
          break;
        case 'bool':
          $value['value'] = $value['value'] === NULL ? '-1' : $value['value'];
          $input_type = 'select';
          break;
        case 'enum':
          $input_type = 'select';
          break;
        case 'timestamp':
          $value['value'] = 'NOW()';
          $input_type = 'textfield';
        default :
          $input_type = 'textfield';
      }
           
      $form['fields']['data'][$key]['value'] = array(
        '#type' => $input_type,
        '#default_value' => $value['value'],
      );
      
      if ($value['type'] == 'enum') {
        $form['fields']['data'][$key]['value']['#options'] = array('-1' => '-----'); 
        $enum_options = unserialize($table->field_enum_options[$value['delta']]['value']); 
        if (is_array($enum_options))
          foreach($enum_options as $option)
            $form['fields']['data'][$key]['value']['#options'][] = $option;
      }

      
      if ($value['type'] == 'bool') {
        $form['fields']['data'][$key]['value']['#options'] = array('-1' => '-----', '1' => 'true', '0' => 'false'); 
      }
			
      $form['delta'][$key] = array(
        '#type' => 'value',
        '#value' => $value['delta']
      );
      
      if (!$form['field_' . $value['type']][$value['delta']]) {
        $form['field_' . $value['type']][$value['delta']] = $form['field_' . $value['type']][$value['delta'] -1];
        $form['field_' . $value['type']][$value['delta']]['_weight']['#delta'] = $value['delta'];
        $form['field_' . $value['type']][$value['delta']]['#delta'] = $value['delta'];
        $form['field_' . $value['type']][$value['delta']]['#weight'] = $value['delta'];
      }
      
    }
  }

  // specify form redirection
  $args = arg();
  if ($args[2] == 'edit') // for node/[record-nid]/edit
    $form_state['redirect'] = 'table/' . $table->nid;
  else
    $form['#redirect'] = 'table/' . $table->nid; // for node/record/add 
  
  // register validate callbacks
  $form['#validate'][] = 'tow_record_validate_edit';
  
  // register submit callbacks
  $form['#submit'][] = 'tow_record_submit_table';
  $form['#submit'][] = 'tow_record_submit_values';

}

/**
 * Submit callback.
 * Set nid of table.
 */
function tow_record_submit_table($form, &$form_state) {
  $form_state['values']['field_table'][0]['value'] = $form_state['values']['table_nid'];
}

/**
 * Submit callback.
 * Set corresponding cck fields (e.g. $form_state['values'][]field_int) field values to be equal to values entered by user 
 */
function tow_record_submit_values($form, &$form_state) {
  if (!is_array($form_state['values']['fields']['data']))
    return;
  foreach($form_state['values']['fields']['data'] as $key => $value) {
    $type = $form['fields']['data'][$key]['type']['#value'];
    if ($type == 'bool')
      $value['value'] = _tow_bool_to_int($value['value']);
    if ($type == 'timestamp')
      $value['value'] = time();
    $delta = $form_state['values']['delta'][$key]; 
    $form_state['values']['field_' . $type][$delta]['_weight'] = $delta;
    $form_state['values']['field_' . $type][$delta]['value'] = _tow_insert_fake($value['value']);
  }
}

/**
 * Formats node content to be displayed on the record view page, which indexed by apachesolr
 */
function tow_record_format_for_search_indexing(&$node) {
  //x($node->field_bool);
  $table_nid = $node->field_table[0]['value'];
   
  // Adding enum field value 
  unset($node->content['field_enum']);
  foreach ($node->field_enum as $delta => $value) {
    $enum_options = unserialize(db_result(db_query("SELECT `field_enum_options_value` 
                                                    FROM {content_field_enum_options}
                                                    WHERE `nid` = %d 
                                                      AND `vid` = %d
                                                      AND `delta` = %d", 
                                                      $table_nid, $table_nid, $delta)));
	$node->content[] = array('#value' => $enum_options[$value['value']] . ' ');
  }
  
  // Formating boolean field values
  unset($node->content['field_bool']);
  foreach ($node->field_bool as $delta => $value) {
    if (isset($value['value'])) {
      $node->content[] = array('#value' => _tow_format_bool($value['value']) . ' ');
    }
  }
  
  // Substitute table_nid by table title
  unset($node->content['field_table']);
  $node->content[] = array('#value' => db_result(db_query("SELECT `title`
                                                   FROM {node}
                                                   WHERE `nid` = %d 
                                                   AND `vid` = %d", 
                                                   $table_nid, $table_nid)));
}

/*
 * Substitutes NULL placeholders by NULLs in node CCK fields
 */
function tow_record_set_empty(&$node) {
	$types = content_types($node->type);
	foreach ($types['fields'] as $field) {
		foreach ($node->$field['field_name'] as &$instance) {
			$instance['value'] = _tow_remove_fake($instance['value']);
		}
	}
}