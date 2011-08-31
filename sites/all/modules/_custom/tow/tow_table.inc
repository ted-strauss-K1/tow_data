<?php 
// $Id: tow_table.inc,v 1.0.0.0  2011/02/28 15:32:21 tow Exp $

/**
 * @file
 */

/**
 * Checking $_GET, construct array of display options to be passed to tow_table()
 */
function tow_table_display_options($node) {
  
  $display_options = array();
  
  foreach(tow_possible_fields() as $field_type)
    $count_fields += count($node->{'field_title_' . $field_type});
  
  if (is_numeric($_GET['order']) && isset($_GET['sort']))
    if ($_GET['order'] >= 0 && $_GET['order'] < $count_fields)
      if ($_GET['sort'] == 'asc' || $_GET['sort'] == 'desc')
        $display_options['sort'] = array(
          'sort_index' => $_GET['order'],
          'sort_order' => $_GET['sort'],
        );  
  
  if (is_numeric($_GET['filter']) && is_numeric($_GET['op']) && isset($_GET['value']))
    if ($_GET['filter'] >= 0 && $_GET['op'] >= 0)
      if ($_GET['filter'] < $count_fields)
        if ($_GET['op'] < tow_table_filter_operators())
          $display_options['filter'] = array(
            'filter_column_index' => $_GET['filter'],
            'filter_operator_key' => $_GET['op'],
            'filter_value' => $_GET['value']
          );
          
  return $display_options;
}

/**
 * Menu callback
 */
function tow_table_callback($nid) {

  $node = node_load($nid);

  if (!$node || $node->type != 'table') {
    drupal_not_found();
    die();
  }
  
  global $dataset;
  global $table;
  global $user;
  
  $dataset = node_load($node->field_dataset['0']['value']);
  $dataset = (object)array('nid' => $dataset->nid, 'title' => $dataset->title, 'uid' => $dataset->uid);
  $table = $node;
  
  $output = $node->teaser;
  if ($user->uid == $dataset->uid || user_access('edit any table content'))
    $output .= "<div>" . l('Edit', 'node/' . $node->nid . '/edit', array('query' => 'destination=table/' . $node->nid)) . "</div>";
  
  $output .= drupal_get_form('tow_table_filter');
  
  $display_options = tow_table_display_options($node);
  
  /*
  $display_options['sort'] = array ('sort_index' => 2, 'sort_order' => 'ASC');
  $display_options['filter'] = array ('filter_column_index' => 3, 'filter_operator_key' => 0, 'filter_value' => 2004);
  $display_options['limits'] = array ('first_elem_no' => 0, 'total_elements' => 10);
  $display_options['paged_view'] = true;
  */
  $display_options['paged_view'] = TRUE;
  $display_options['limits']['total_elements'] = 10;                   
  $display_options['click_sortable'] = TRUE;

  $output .= tow_table($node, $display_options);
  
  if ($user->uid == $dataset->uid)
    $output .= _tow_table_actions_list($node->nid, 'edit');

  return $output;
}

/**
 * Get table data to be placed into "table" view.
 * @param $node node to be displayed
 * 
 * $sort_index 		column index or 'nid'	// Default value: $sort_index = 'nid';
 * $sort_order		"ASC" or "DESC"	      // Default value: $sort_order = "ASC";
 * 
 * $filter_column_index column index 		// Default value: $filter_column_index = 0,  
 * $filter_operator_key filtering operator 	// Default value: $filter_operator_key = 0,
 * $filter_value = value to be used during comparison // Default value: $filter_value = 0
 
 * $first_elem_no 	first element to be displayed	// Default value: $first_elem_no = 0;
 * $elements_per_page	no of elements to display 	// Default value: $total_elements = 10;
 * 
 * 
 * 
 * @return array of table rows. Each row is array of cells. 
 */
function tow_table($node, $display_options = array()) {

  $paged_view = isset($display_options['paged_view']);
  
  if (($paged_view) && (isset($display_options['limits']['total_elements'])))
    $total_elements = $display_options['limits']['total_elements'];
  else 
    $total_elements = 10; 
  
  $headers = $rows = array();

  // Creating query
  $user_table = tow_construct_table_query($node, $display_options);

  $headers = $user_table['titles']['values'];
  
  if ($display_options['click_sortable']) {
    array_unshift($headers, "");
  }

  $query =  $user_table['query'];
  $args = $user_table['arguments'];

  if ($paged_view) {
    $result = pager_query($query, $total_elements, 0, NULL, $args);
  } else {
    $result = db_query($query, $args);
  }
  
  while ($rs = db_fetch_array($result)){
    $row = array();
    foreach ($rs as $key => $value) {
      if ($user_table['titles']['types'][$key] == 'bool') {
        $row[] = array('data' => _tow_format_bool($value));
      }elseif ($user_table['titles']['types'][$key] == 'date') {
        $row[] = array('data' => substr($value, 0, 10));
      }elseif ($user_table['titles']['types'][$key] == 'timestamp') {
        $row[] = array('data' => date('d M Y H:i:s', $value));  
      }elseif ($user_table['titles']['types'][$key] == 'enum') {
        $title_enum = $user_table['titles']['values'][$key];
        foreach($node->field_title_enum as $enum_delta => $enum_value) {
          if ($enum_value['value'] == $title_enum) {
            unset($node->field_title_enum[$enum_delta]);
            break;
          }
        }
        $enum_options = unserialize($node->field_enum_options[$enum_delta]['value']);
        $row[] = array('data' => $enum_options[$value]); 
      }else {
        $row[] = array('data' => $value);
      }
    }
	if (!$display_options['click_sortable']) {
       array_shift($row); 
     }
    $rows[] = array('data' => $row);
  }
  
  if ($display_options['click_sortable']) {
    $output =  theme('sortable_table', $headers, $rows, $node->nid, $node->uid, array('id' => 'main-table'));
  }else {
    $output =  theme('table', $headers, $rows);
  }
    
  if  ($paged_view) {
    $output .= theme('pager', NULL, $total_elements);
    global $bc_attachment;
    $total = db_result(db_query("SELECT COUNT(*) FROM {content_type_record} WHERE field_table_value = %d", $node->nid));
    $bc_attachment = ' (' . $total . t(' rows; displaying ');
    $start = $_GET['page'] * $total_elements  + 1;
    $bc_attachment .= $start . ' - ' . ($start + count($rows) -1) . ')';  
  }

  return $output;
}

/**
 * Part of hook_form_alter implementation.
 */
function tow_table_form_alter(&$form, &$form_state) {
  
  ## DEFAULT
  // unset unneeded elements
  unset($form['menu']);
  unset($form['revision_information']);
  unset($form['comment_settings']);
  
  // hide native cck fields
  foreach (tow_possible_fields() as $field_type)
    $form['field_title_' . $field_type]['#access'] = FALSE;
  $form['field_enum_options']['#access'] = FALSE;
  $form['field_dataset']['#access'] = FALSE;
  
  $form['#content_extra_fields']['body_field']['weight'] = 0;
  
  // add tabular form containing
  $form['fields'] = array(
    '#theme' => 'tabular_form',
    '#title' => 'Fields',
    '#tree' => true,
    '#weight' => -1,
  );
  
  $form['fields']['header'] = array(
    '#type' => 'value',
    '#value' => array('Title', 'Type'),
  );
  
  // add array of delta
  $form['delta'] = array(
    '#tree' => TRUE
  );
  
  // get array of fields ordered by weights
  $weights_array = unserialize($form['#node']->field_weights[0]['value']);
  // register common validation function
  $form['#validate'][] = 'tow_table_validate_common';
  ## TABLE IS BEING EDITED
  if ($weights_array) {   
    
    global $dataset;
    global $table;
  
    $dataset = node_load($form['#node']->field_dataset['0']['value']);
    $dataset = (object)array('nid' => $dataset->nid, 'title' => $dataset->title);
    $table = (object)array('nid' => $form['#node']->nid, 'title' => $form['#node']->title);
    
    $index = 0;
    foreach (tow_possible_fields() as $field_type) {
      $delta = 0;
      if (!is_array($form['#node']->{'field_title_' . $field_type}))
        continue;
      foreach($form['#node']->{'field_title_' . $field_type} as $title_value)
        if ($title_value['value']) {
          $aux_array[$weights_array[$index]]['value'] = $title_value['value'];
          $aux_array[$weights_array[$index]]['type'] = $field_type;
          $aux_array[$weights_array[$index]]['delta'] = $delta;
          $index++;
          $delta++;
        }
    }
    
    ksort($aux_array);
    
    $form['fields']['header']['#value'][] = t('Index');
    $indexes_array = unserialize($form['#node']->field_indexes[0]['value']);
    
    // nest arrays of delta and array of fields
    foreach($aux_array as $key => $value) {
      
      $form['delta'][$key] = array(
        '#type' => 'value',
        '#value' => $value['delta']
      );
           
      $form['fields']['data'][$key]['title'] = array(
        '#type' => 'textfield',
        '#default_value' => $value['value'],
        '#size' => 5,
      );
      $form['fields']['data'][$key]['type'] = array(
        '#type' => 'item',
        '#value' => $value['type'],
      );
      
      $form['fields']['data'][$key]['index'] = array(
        '#type' => 'checkbox',
        '#default_value' => $indexes_array[$key],
      );
      
      if ($value['type'] == 'enum') {
        foreach($form['#node']->field_title_enum as $enum_delta => $enum_value)
          if ($enum_value['value'] == $value['value'])
            break;
        $enum_options = unserialize($form['#node']->field_enum_options[$enum_delta]['value']);
        if ($enum_options) {  
          $enum_options_string = implode(' | ', $enum_options);
          $form['fields']['data'][$key]['type']['#value'] .= ' : ' . $enum_options_string;
        }  
      }
    }// TODO: interface for field deleting

    
    // add interface for field adding
    $form['field_add'] = array(
      '#type' => 'fieldset',
      '#title' => 'Add field',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#tree' => TRUE,
      '#weight' => -0.5  
    );
    $form['field_add']['title'] = array(
      '#type' => 'textfield',
      '#description' => t('Enter title'),
      '#size' => 5,
      '#maxlength' => 20,
    );
    $form['field_add']['type'] = array(
      '#type' => 'select',
      '#description' => t('Select type'),
      '#options' => tow_possible_fields(),
    );
    $form['field_add']['enum_options'] = array(
      '#type' => 'textarea',
      '#description' => t('If you choose enum, enter enum options here. Each option should be in single string.'),
    );
    $form['field_add']['description'] = array(
      '#type' => 'textarea',
      '#description' => t('Write a short description to help people understand this field (optional).'),
    );
    $form['field_add']['index'] = array(
      '#type' => 'checkbox',
      '#title' => t('Index')
    );
    
    // register additional validation function for table edit case  
    $form['#validate'][] = 'tow_table_validate_edit';
    
    // add interface for actions: add record, truncate, browse    
    $form['action_links'] = array(
      '#type' => 'item',
      '#value' => _tow_table_actions_list($form['#node']->nid, 'browse'),
      '#weight' => -0.3
    );
    
    // TODO: user friendly interface for weights changing
    
    // register submit callbacks
    $form['#submit'][] = 'tow_table_submit_fields';
    $form['#submit'][] = 'tow_table_submit_weights';
    $form['#submit'][] = 'tow_table_submit_field_add';
    $form['#submit'][] = 'tow_table_submit_indexes';
    
  }  
  
  ## TABLE IS BEING CREATED
  else{
    
    if ($_SESSION['table_definition']) {
      $form['title']['#default_value'] = $_SESSION['table_definition']['table_name'];
      $form['field_dataset'][0]['#default_value']['value'] = $_SESSION['table_definition']['dataset'];
      $count_fields = $_SESSION['table_definition']['count_fields'];
      unset($_SESSION['table_definition']);
    }
    else{
      $count_fields = 1;
      if (is_numeric($_GET['dataset']))
        $form['field_dataset'][0]['#default_value']['value'] = $_GET['dataset'];
        //array_unshift($form['#validate'], 'tow_table_dont_validate_cancel');
        $form['#validate'][] =  'tow_table_dont_validate_cancel';
    }
    
    global $dataset;
    $dataset = node_load($form['field_dataset'][0]['#default_value']['value']);
    $dataset = (object)array('nid' => $dataset->nid, 'title' => $dataset->title);
    
    $form['buttons']['submit']['#value'] = t('Create table');
    
    $form['fields']['header']['#value'][] = 'Enum options';
    $form['fields']['header']['#value'][] = 'Description';
    $form['fields']['header']['#value'][] = 'Index';
    
    for ($i = 0; $i < $count_fields; $i++) {
    
      $form['fields']['data'][$i]['title'] = array(
        '#type' => 'textfield',
        '#size' => 8,
      );    
      $form['fields']['data'][$i]['type'] = array( 
        '#type' => 'select',
        '#options' => tow_possible_fields(),
      );
      $form['fields']['data'][$i]['enum_options'] = array(
        '#type' => 'textarea',
        '#rows' => 2,
      );
      $form['fields']['data'][$i]['description'] = array(
        '#type' => 'textarea',
        '#rows' => 2,
      );
      $form['fields']['data'][$i]['index'] = array(
        '#type' => 'checkbox',
      );
    }
    
    $field_types = tow_possible_fields();
    
    for($i = 2; $i < $count_fields; $i++) {
      foreach ($field_types as $field_type) {
        $form['field_title_' . $field_type][$i] = $form['field_title_' . $field_type][0];
        $form['field_title_' . $field_type][$i]['_weight']['#delta'] = $i;
        $form['field_title_' . $field_type][$i]['#delta'] = $i;
        $form['field_title_' . $field_type][$i]['#weight'] = $i;
      }
      $form['field_enum_options'][$i] = $form['field_enum_options'][0];
      $form['field_enum_options'][$i]['_weight']['#delta'] = $i;
      $form['field_enum_options'][$i]['#delta'] = $i;
      $form['field_enum_options'][$i]['#weight'] = $i;
    }
    
    $form['count_fields'] = array(
      '#type' => 'value',
      '#value' => $count_fields,
    );
    
    $form['#submit'][] = 'tow_table_submit_create';
  }  
      

}

## CALLBACKS FOR EDITING

/**
 * Submit callback.
 * Set corresponding cck fields (e.g. $form_state['values'][]field_title_int) field values to be equal to values entered by user 
 */
function tow_table_submit_fields($form, &$form_state) {
  
  $weights = $form_state['values']['field_weights'][0]['value'];
  if (!$weights) // nothing in field_weight (node is being saved first time)
    return;
  $weights = unserialize($weights);
  if (!$weights) // empty array in field_weight
    return;
  
  if (!is_array($form_state['values']['fields']['data']))
    return;
  
  foreach ($form_state['values']['fields']['data'] as $key => $value) {
    $delta = $form_state['values']['delta'][$key];
    $type = $form['fields']['data'][$key]['type']['#value'];
    $form_state['values']['field_title_' . $type][$delta]['value'] = $value['title'];
  }
  
}

/**
 * Submit callback
 * If table is being saved first time set weights to be empty array.
 * If there is a field to add add one element to weights array. (Rebuilding weights)
 */
function tow_table_submit_weights($form, &$form_state) {  
  if (!$form_state['values']['field_weights'][0]['value']) {
    $weights = array();
    $form_state['values']['field_weights'][0]['value'] = serialize($weights);
  }

  if ($form_state['values']['field_add']['title']) {
    $weights = unserialize($form_state['values']['field_weights'][0]['value']);
    if ($weights) {
      $field_types = tow_possible_fields();
      $type = $field_types[$form_state['values']['field_add']['type']];
      $pos = 0;
      foreach ($field_types as $type_value) {
        if (!is_array($form_state['values']['field_title_' . $type_value]))
          continue;
        foreach ($form_state['values']['field_title_' . $type_value] as $field) {
          if (is_array($field) && $field['value']) {
            $pos++;
          }
        }
        if ($type == $type_value) {
          break;
        }
      }
      $weights_left = array_slice($weights, 0, $pos);
      $insert = array(max($weights) + 1);
      $weights_right = array_slice($weights, $pos);
      $weights = array_merge($weights_left, $insert, $weights_right);  
      
    }else{
      $weights[] = 0;
    }
    $form_state['values']['field_weights'][0]['value'] = serialize($weights);
  }
  
}

/**
 * Submit callback.
 * If user added feld, place this field title into corresponding cck field.
 */
function tow_table_submit_field_add($form, &$form_state) {
  $title = $form_state['values']['field_add']['title'];
  if ($title) {
    $possible_fields = tow_possible_fields();
    $type = $possible_fields[$form_state['values']['field_add']['type']];
    $key = count($form_state['values']['field_title_' . $type]) - 2;
    $form_state['values']['field_title_' . $type][$key]['value'] = $title;
    if ($type == 'enum') {
      $enum_options = serialize(explode("\r\n", $form_state['values']['field_add']['enum_options']));
      $form_state['values']['field_enum_options'][$key]['value'] = $enum_options;
    }    
  // Saving description
    $description = trim($form_state['values']['field_add']['description']);
    _tow_field_description_save($form['#node']->nid, count($form_state['values']['fields']['data']), $description);
  }
}

/**
 * Submit callback
 * Save serialized array of indexes
 */
function tow_table_submit_indexes($form, &$form_state) {
  foreach ($form_state['values']['fields']['data'] as $field)
    $indexes[] = $field['index'];
  if ($form_state['values']['field_add']['title'])
    $indexes[] = $form_state['values']['field_add']['index'];
  $form_state['values']['field_indexes'][0]['value'] = serialize($indexes);
}

## CALLBACK FOR CREATION

/**
 * Submit callback.
 * Place fields user submitted into cck fields.
 * Save array of weights. 
 */
function tow_table_submit_create($form, &$form_state) {
  $weights = $indexes = array();
  $field_types = tow_possible_fields();
  foreach ($field_types as $type_key => $type) {
    $delta = 0;
    $enum_delta = 0;
    foreach ($form_state['values']['fields']['data'] as $index => $field) {
      if ($field['type'] == $type_key) {
        $form_state['values']['field_title_' . $type][$delta]['value'] = $field['title'];
        $form_state['values']['field_title_' . $type][$delta]['_weight'] = $delta;
        if ($type == 'enum') {
          $form_state['values']['field_enum_options'][$delta]['value'] = serialize(explode("\r\n", $field['enum_options']));
          $form_state['values']['field_enum_options'][$delta]['_weight'] = $delta;
          $enum_delta++;
        }
        $delta++;
        $weights[] = $index;
      }
    }
    for ($i = $form_state['values']['count_fields']; $i > $delta; $i--) {
      unset($form_state['values']['field_title_' . $type][$i]);
    }
    if ($type == 'enum') {
      for ($i = $form_state['values']['count_fields']; $i > $enum_delta; $i--) {
        unset($form_state['values']['field_enum_options'][$i]);
      }
    }
  }
  
  global $descriptions;
  foreach ($form_state['values']['fields']['data'] as $field) {
    $indexes[] = $field['index'];
    // Preparing descriptions to be saved after node creation
    $descriptions[] = trim($field['description']);
  }
  
  $form_state['values']['field_weights'][0]['value'] = serialize($weights);
  $form_state['values']['field_indexes'][0]['value'] = serialize($indexes);
}

/**
 * @param $nid
 * @return html list of links: add record, truncate, browse OR edit table 
 */
function _tow_table_actions_list($nid, $last_link) {
  $items = array(
    l('Add record', 'node/add/record', array('html' => TRUE, 'query' => 'table=' . $nid)),
    l('Delete all', 'table/' . $nid . '/truncate'),
  );
  
  if ($last_link == 'edit')
    $items[] = l('Edit table', 'node/' . $nid . '/edit');
  elseif ($last_link == 'browse')
    $items[] = l('Browse table', 'table/' . $nid);
    
  return theme_item_list($items);
}

/**
 * When "cancel" button is clicked, redirect to node dataset page without error messages.
 */
function tow_table_dont_validate_cancel($form, &$form_state) {
  if ($form_state['clicked_button']['#value'] == 'Cancel') {
    unset($_SESSION['messages']['error']);
    drupal_goto('node/' . $form_state['values']['field_dataset'][0]['value']);
  }
}

/**
 * Add table structure themed as table to the node
 */
function tow_table_structure(&$node) {
  
  global $dataset;
  global $table;
  global $bc_attachment;
  global $user;
  
  $bc_attachment = ' (' . t('table structure') . ')';
  
  $dataset = node_load($node->field_dataset['0']['value']);
  $dataset = (object)array('nid' => $dataset->nid, 'title' => $dataset->title);
  $table = (object)array('nid' => $node->nid, 'title' => $node->title);
  
  if ($user->uid == $node->uid || user_access('edit any table content')) {
    $node->structure['clone'] = drupal_get_form('tow_table_clone_table_form');
    $node->structure['delete'] = drupal_get_form('tow_table_delete_table_form');
    $node->structure['edit_description'] = l('edit description', 'node/' . $node->nid . '/edit');
  }
  
  $headers = array(t('Action'), t('Field'), t('Type'), t('Index'), t('% Blanks'));
  
  $weights_array = unserialize($node->field_weights[0]['value']);
  $index_array = unserialize($node->field_indexes[0]['value']);
  
  // if there are any fields fill rows and sort them by weights
  if (!empty($weights_array)) {
    $index = 0;
    foreach (tow_possible_fields() as $field_type) {
      $delta = 0;
      if (!is_array($node->{'field_title_' . $field_type}))
        continue;
      foreach($node->{'field_title_' . $field_type} as $title_value)
        if ($title_value['value']) {
          $rows[$weights_array[$index]]['action'] = l('delete', 'field_delete/' . $table->nid . '/' . $field_type . '/' . $delta . '/' . $weights_array[$index]) . ' '
            . l('edit', 'field_edit/' . $table->nid . '/' . $field_type . '/' . $delta . '/' . $weights_array[$index]);
          $rows[$weights_array[$index]]['value'] = $title_value['value'];
          $rows[$weights_array[$index]]['type'] = $field_type;
          $rows[$weights_array[$index]]['index'] = $index_array[$weights_array[$index]] ? t('Yes') : t('No');
          $rows[$weights_array[$index]]['blanks'] = tow_table_structure_blanks($node->nid, $delta, $field_type);
          if ($field_type == 'enum') {
            $enum_options = unserialize($node->field_enum_options[$delta]['value']);
            if ($enum_options) {  
              $enum_options_string = implode(' | ', $enum_options);
              $rows[$weights_array[$index]]['type'] .= ' : ' . $enum_options_string;
            }
          }
          $index++;
          $delta++;
        }
      }
    
      ksort($rows);
      
      // Adding info link for each row
      $index = 0;
      foreach ($rows as $key => $value) {
        $rows[$key]['action'] = l('info', 'field/' . $table->nid . '/' . $index++) . ' ' . $rows[$key]['action']; 
      }
      
      $node->structure['structure'] = theme('table', $headers, $rows);  
  }else{
    $node->structure['structure'] = t('There are no rows');
    $count_fields = 0;
  }
  
  $count_rows = db_result(db_query("SELECT COUNT(*) FROM {content_type_record} WHERE field_table_value = %d", $table->nid));
  if ($count_rows == 1)
    $count_rows .= ' ' . t('row');
  else
    $count_rows .= ' ' . t('rows');
    
  $count_fields = count($rows);
  if ($count_fields == 1)
    $count_fields .= ' ' . t('field');
  else
    $count_fields .= ' ' . t('fields');
  
  $node->structure['table_stats'] = t('This table has %rows and %fields', array('%rows' => $count_rows, '%fields' => $count_fields));
  global $table_statistics;
  if (!$table_statistics)
    $table_statistics = 0;
  $node->structure['table_stats'] .= "<br>" . t('Views') . ': ' . $table_statistics;
  
  if ($user->uid == $node->uid || user_access('edit any table content')) {
    foreach($rows as $row)
      $existing_fields[] = $row['value'];
    $node->structure['field_add'] = drupal_get_form('tow_table_field_add_form', $node, $existing_fields);
  }
}

/**
 * @return percentage of blank cells of field
 */
function tow_table_structure_blanks($table_nid, $delta, $type) {
  $blanks = round(db_result(db_query("
    SELECT 1 - (
      SELECT COUNT(*)
      FROM {content_field_%s} x INNER JOIN {content_type_record} r 
      ON x.nid = r.nid AND x.field_%s_value IS NOT NULL
      WHERE r.field_table_value = %d
      AND x.delta = %d
    ) / (
      SELECT COUNT(*) FROM `content_type_record` WHERE field_table_value = %d
    )
    ", $type, $type, $table_nid, $delta, $table_nid)) * 100);
  if ($blanks != 0)
    $blanks .= '%';
  return $blanks;
}

/**
 * @return form with 'Clone table' button
 */
function tow_table_clone_table_form() {
  $form['clone'] = array(
    '#type' => 'submit',
    '#value' => t('Clone table')
  );
  return $form;
}

/**
 * Submission callback
 * Save node of table type similar to original.
 */
function tow_table_clone_table_form_submit($form, &$form_state) {
  global $dataset;
  global $table;
  $node = node_load($table->nid);
  unset($node->nid, $node->vid, $node->created, $node->changed, $node->comment, $node->last_comment_timestamp, $node->last_comment_name, $node->comment_count);
  
  $title = $node->title;
  $len = $i = strlen($title) - 1;
  while (is_numeric($title[$i])) {
    $number .= $title[$i];
    $i--;
  }
  
  if (isset($number)) {
    $number = strrev($number);
    $word = substr($title, 0, 1 + $len - strlen($number));
  }
  else {
    $word = $title;
  }
  
  $highest_db_title = db_result(db_query("
    SELECT title FROM {node} n INNER JOIN {content_type_table} ctt
    ON n.nid = ctt.nid AND ctt.field_dataset_value = %d
    WHERE n.title LIKE '%s%' ORDER BY title DESC LIMIT 1
    ", $dataset->nid, $word));
  
  if ($highest_db_title == $title) {
    $number++;
    if ($number < 10)
      $number = '0' . $number;
    $node->title = $word . $number;
  }else {
    $node->title = _numbered_title($highest_db_title);
  }
  
  node_save($node);
  global $user;
  $nid = db_result(db_query("SELECT nid FROM {node} WHERE uid = %d ORDER BY nid DESC LIMIT 1", $user->uid));
  drupal_set_message(t('Table %title has been created', array('%title' => $node->title)));
  $form_state['redirect'] = 'node/' . $nid;
  //drupal_set_message(t('Table') . ' ' . l($node->title, 'node/' . $nid) . ' ' . t('has been created'));
}

/**
 * @return form with 'Delete table' button
 */
function tow_table_delete_table_form() {
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete table')
  );
  return $form;
}

/**
 * Submission callback
 * Delete table node and redirect to dataset
 */
function tow_table_delete_table_form_submit($form, &$form_state) {
  global $table;
  global $dataset;
  node_delete($table->nid);  
  $form_state['redirect'] = 'dataset/' . $dataset->nid;
}

/**
 * If @param $title has no digits at the end, e.g. 'mytable', @return "mytable01"
 * If it has digits at the end, e.g. 'table05', @return 'table06'
 */
function _numbered_title($title) {
  
  $len = $i = strlen($title) - 1;
  while (is_numeric($title[$i])) {
    $number .= $title[$i];
    $i--;
  }
  
  if (isset($number)) {
    $number = strrev($number);
    $word = substr($title, 0, 1 + $len - strlen($number));
    $number++;
    if ($number < 10)
      $number = '0' . $number;
  }
  else {
    $word = $title;
    $number = '01';
  }
      
  return $word . $number;
}

/**
 * User interface for field adding 
 */
function tow_table_field_add_form($form_state, $node, $existing_fields) {

  $form['existing_fields'] = array(
    '#type' => 'value', 
    '#value' => $existing_fields
  );
  $form['node'] = array(
    '#type' => 'value',
    '#value' => $node
  );
  $form['field_add'] = array(
    '#type' => 'fieldset',
    '#title' => 'Add field',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#weight' => -0.5  
  );
  $form['field_add']['title'] = array(
    '#type' => 'textfield',
    '#description' => t('Enter title'),
    '#size' => 5,
    '#maxlength' => 20,
  );
  $form['field_add']['type'] = array(
    '#type' => 'select',
    '#description' => t('Select type'),
    '#options' => tow_possible_fields(),
  );
  $form['field_add']['enum_options'] = array(
    '#type' => 'textarea',
    '#description' => t('If you choose enum, enter enum options here. Each option should be in single string.'),
  );
  $form['field_add']['description'] = array(
    '#type' => 'textarea',
    '#description' => t('Write a short description to help people understand this field (optional).'),
  );
  $form['field_add']['index'] = array(
    '#type' => 'checkbox',
    '#title' => t('Index')
  );
  $form['field_add']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('add field')
  );
  
  $form['#validate'][] = 'tow_table_field_add_form_validate';
  
  return $form;
}

/**
 * Submission callback
 * Rebuild array of weights
 * Add field to table node
 */
function tow_table_field_add_form_submit($form, &$form_state) {
  $node = $form_state['values']['node'];
  $title = $form_state['values']['field_add']['title'];
  $possible_fields = tow_possible_fields();
  $type = $possible_fields[$form_state['values']['field_add']['type']];

  $weights = unserialize($node->field_weights[0]['value']);
  if ($weights) {
    $pos = 0;
    foreach ($possible_fields as $type_value) {
      foreach ($node->{'field_title_' . $type_value} as $field) {
        if ($field['value']) {
          $pos++;
        }
      }
      if ($type == $type_value) {
        break;
      }
    }
    $weights_left = array_slice($weights, 0, $pos);
    $insert = array(max($weights) + 1);
    $weights_right = array_slice($weights, $pos);
    $weights = array_merge($weights_left, $insert, $weights_right);
  }else {
    $weights[] = 0;
  }
  $node->field_weights[0]['value'] = serialize($weights);
  
  if ($node->{'field_title_' . $type}[0]['value'])
    $delta = count($node->{'field_title_' . $type});
  else
    $delta = 0;
  
  $node->{'field_title_' . $type}[$delta] = array('value' => $title);
  
  if ($type == 'enum') {
    $enum_options = serialize(explode("\r\n", $form_state['values']['field_add']['enum_options']));
    $node->field_enum_options[$delta]['value'] = $enum_options;
  }
  
  $indexes = unserialize($node->field_indexes[0]['value']); 
  $indexes[] = $form_state['values']['field_add']['index'];
  $node->field_indexes[0]['value'] = serialize($indexes);
  
  node_save($node);
  // Saving description
  $description = trim($form_state['values']['field_add']['description']);
  _tow_field_description_save($node->nid, count($indexes)-1, $description);
  drupal_set_message('Field has been added');
}

function _tow_get_table_fields($table_node){
  $aux_array = array();
  
  // 1.1 Weights responsible for the column order
  $weights_array = unserialize($table_node->field_weights[0]['value']);
  $indexes = unserialize($table_node->field_indexes[0]['value']);
  
  // 1.2 Extracting all data needed for query construction into auxiliary array
  $index = 0;
  foreach (tow_possible_fields() as $field_type) {
    $delta = 0;
    if (!is_array($table_node->{'field_title_' . $field_type}))
      continue;
    foreach($table_node->{'field_title_' . $field_type} as $title_value) {
      if ($title_value['value']) {
        $aux_array[$weights_array[$index]]['title'] = $title_value['value'];
        $aux_array[$weights_array[$index]]['delta'] = $delta;
        $aux_array[$weights_array[$index]]['type'] = $field_type;
        $aux_array[$weights_array[$index]]['index'] = $indexes[$index];

        $index++;
        $delta++;
      }
    }
  }

  if ($aux_array)
    ksort($aux_array);
 return $aux_array;
}