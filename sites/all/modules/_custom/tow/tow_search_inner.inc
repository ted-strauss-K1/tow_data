<?php
/**
 * Menu callback
 */
function tow_search_within_dataset_callback($nid) {
  x('tow_search_within_dataset_callback called');
  x($_GET);
  // x($_POST);
  $output = '';
  
  // 0 Check URL 
  // 0.1 if nid belongs to dataset 
  $node = node_load($nid);
  if (!$node || $node->type != 'dataset') {
    drupal_not_found();
    die();
  }
  
/*  global $dataset;
  global $table;
  global $user;
  */
  //$dataset = node_load($nid);
  //$dataset = (object)array('nid' => $dataset->nid, 'title' => $dataset->title, 'uid' => $dataset->uid);
  //$table = $node;
    
  /*$output = $node->teaser;
  if ($user->uid == $dataset->uid || user_access('edit any table content'))
    $output .= "<div>" . l('Edit', 'node/' . $node->nid . '/edit', array('query' => 'destination=table/' . $node->nid)) . "</div>";
  */

  // 0.2 Check search parameters
  foreach ($_GET as $param => $value) {
    
  }  
  
  
  //http://43_tow_local/search_dataset/1?scan_tables=3,4&disp_tables=3,4&disp_fields=t3-char-2,t3-enum-3,t3-char-4,t3-int-5,t4-char-1&save=&ssname=&ovwr=
  $search_parameters = array (
    'scan_tables' => explode($_GET['scan_tables'], ','),
  	'disp_tables' => explode($_GET['disp_tables'], ','),
  	'disp_fields' => explode($_GET['disp_fields'], ','),
    'save_search' => $_GET['save'],
    'ss_name' => $_GET['ssname'],
    'overwrite' => $_GET['ovwr'],
    
  );
  
  // 1 Add search form
  // 1.1 Add empty form
 // $output .= drupal_get_form('tow_search_form', $nid);
  
  // 1.2 Fill in the form with search parameters
  
  
  
  //$search_options = tow_search_within_dataset_options();
  
  /*
  $display_options['sort'] = array ('sort_index' => 2, 'sort_order' => 'ASC');
  $display_options['filter'] = array ('filter_column_index' => 3, 'filter_operator_key' => 0, 'filter_value' => 2004);
  $display_options['limits'] = array ('first_elem_no' => 0, 'total_elements' => 10);
  $display_options['paged_view'] = true;
  
  $display_options['paged_view'] = TRUE;
  $display_options['limits']['total_elements'] = 10;                   
  $display_options['click_sortable'] = TRUE;
  */
  //$output .= tow_search_within_dataset($node, $search_options);
  
  /*if ($user->uid == $dataset->uid)
    $output .= _tow_table_actions_list($node->nid, 'edit');
  */
  return $output;
}
/*
/**
 * Build form for simple search.
 
function tow_search_within_dataset_simple() {
  $form['dataset_nid'] = array(
    '#type' => 'value',
    '#value' => $dataset_nid,
  );
  $form['simple_search'] = array(
    '#tree' => TRUE,
    'textfield' => array(
      '#type' => 'textfield',
      '#title' => t('Simple search'),
	  '#default_value' => $form['basic']['inline']['keys']['#default_value'],
  ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Go'),
	  //'#submit' => array('tow_search_within_dataset_simple_submit'),
  //'file' => 'tow_search_inner.inc',
    )
  );
  return $form;
}

/**
 * Build form for advanced search.
 
function tow_search_within_dataset_advanced() {
 
}
*/

/**
 * Creates form elements for the search_within_dataset page
 */
function tow_search_within_dataset_altering_form(&$form, &$form_state, $dataset_nid) {

  global $dataset;
  $dataset = node_load($dataset_nid);
  if ($dataset->type != 'dataset') {
    drupal_access_denied();
    die();
  }

  drupal_add_js(drupal_get_path('module', 'tow') . '/js/search.js');
 
  drupal_set_title($dataset->title);

  $form['dataset_nid'] = array(
    '#type' => 'value',
    '#value' => $dataset_nid,
  ); 
  
  $form['simple_search'] = array(
    '#tree' => TRUE,
    'textfield' => array(
      '#type' => 'textfield',
      '#title' => t('Simple search'),
	  '#default_value' => $form['basic']['inline']['keys']['#default_value'],
  ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Go'),
	  '#submit' => array('tow_search_within_dataset_submit_simple'),
  //'file' => 'tow_search_inner.inc',
  )
  );

  $form['advanced_search'] = array(
    '#tree' => TRUE,
    '#title' => t('Advanced search form'),
    'title' => array(
      '#type' => 'item',
      '#title' => t('Advanced search form'),
  )
  );

  $form['clear'] = array(
    '#type' => 'item',
    '#value' => t('Clear form'),
    '#prefix' => '<div class="clear-form">',
    '#suffix' => '</div>'
    );
    $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
	'#submit' => array('tow_search_within_dataset_submit'),
    //'file' => 'tow_search_inner.inc',
    );

    $form['saved_searches'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Saved searches'),
    'select' => array(
      '#type' => 'select',
      '#options' => tow_search_get_saved_searches($dataset->nid),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Load results'),
	  '#submit' => 'tow_search_within_dataset_load_saved_search',
    )
    );

    $form['tables'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Which tables are you interested in?'),
    'select' => array(
      '#type' => 'select',
      '#options' => tow_table_list_with_links($dataset_nid),
      '#multiple' => TRUE
    )
    );

    // FILTERS
    $form['filters'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Constrain & filter the data; search for terms and values'),
    'items' => array(
      '#prefix' => '<div id="filter-items">',
      '#suffix' => '</div>',
    ),
    '#prefix' => '<div class="clear-block" id="filter-wrapper">',
    '#suffix' => '</div>',
    );

    if (is_array($form_state['values']['filters']['items'])) {
      $form['filters']['existing_fields'] = array(
      '#type' => 'value',
      '#value' => $form_state['values']['filters']['existing_fields'],
      //'#attributes' => array('class' => 'filters-inactive'),
      );
      $form['filters']['existing_enum_options'] = array(
      '#type' => 'value',
      '#value' => $form_state['values']['filters']['existing_enum_options'],
      //'#attributes' => array('class' => 'filters-inactive'),
      );
    }
    else {
      $form['filters']['existing_fields'] = array(
      '#type' => 'value',
      '#value' => tow_search_get_fields($dataset_nid), 
      //'#attributes' => array('class' => 'filters-inactive'),
      );
      $form['filters']['existing_enum_options'] = array(
      '#type' => 'value',
      '#value' => tow_search_get_enum_options($dataset_nid), 
      //'#attributes' => array('class' => 'filters-inactive'),
      );
      $form['filters']['existing_operators'] = array(
      '#type' => 'value',
      '#value' => tow_search_get_operators(),
      );

    }

    $form['filters']['add_more'] = array(
    '#type' => 'button',
    '#value' => t('Add more'),
    '#weight' => 100,
    '#ahah' => array(
      'path' => ahah_helper_path(array(
        'filters', 'filter-items-wrapper', 'filter-items'
        )),
      'wrapper' => 'filter-items-wrapper',
      'effect' => 'fade',
      'event' => 'click',
      'method' => 'append',
        ),
        );

        $form['filters']['filter-items-wrapper'] = array(
    '#prefix' => '<div id="filter-items-wrapper">',
    '#suffix' => '</div>',
        //'filter-items' => _tow_search_get_filter_element($form['filters']['existing_fields']['#value'], $form['filters']['existing_operators']['#value'])
    'filter-items' => _tow_search_get_filter_element($form['filters']['existing_fields']['#value'], $form['filters']['existing_enum_options']['#value']),
        );

        $form['fields'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Select which fields to display in the results'),
    'select_tables' => array(
      '#type' => 'checkboxes',
      '#options' => $form['tables']['select']['#options'],
      '#title' => t('Tables')
        ),
    'select_fields' => array(
      '#type' => 'checkboxes',
      '#options' => $form['filters']['existing_fields']['#value'],
      '#title' => t('Fields'),
        //'#disabled' => true,
        )
        );

        $form['agsp'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Averaging, grouping, sorting and processing the results'),
    'result_type' => array(
      '#type' => 'select',
      '#options' => array('Individual results', 'Group by'),
      '#title' => t('Result type')
        ),
    'sort_by' => array(
      '#type' => 'select',
      '#options' => $form['filters']['existing_fields']['#value'],
      '#title' => t('Sort by')
        ),
    'filter_null' => array(
      '#type' => 'checkbox',
      '#title' => t('Filter out rows that have one or more null records')
        ),
    'limit' => array(
      '#type' => 'select',
      '#prefix' => '<span class="display-amount">' . t('Display') . '</span>',
      '#suffix' => '<span>' . t('results per page') . '</span>',
      '#options' => array(25, 50, 100, 250) 
        ),
        );

        $form['save_search'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Save this search'),
    'save' => array(
      '#type' => 'checkbox',
      '#title' => t('Save this search')
        ),
    'textfield' => array(
      '#type' => 'textfield',
      '#description' => t('Name this search something descriptive'),
        ),
    'overwrite' => array(
      '#type' => 'checkbox',
      '#title' => t('Overwrite an existing saved search with this name (only allowed if you created original)')
        ),
    'note' => array(
      '#type' => 'item',
      '#value' => t('Note: the administrator of this data set can remove your saved search if they decided to')
        )
        );

        $form['clear2'] = $form['clear'];
        $form['submit2'] = $form['submit'];

        // Turn off apachesolr form features, if they are set
        if (isset($form['basic'])) {
          unset($form['basic']);
        }
        if (isset($form['#submit'])) {
          unset($form['#submit']);
        }
        if (isset($form['#validate'])) {
          unset($form['#validate']);
        }
        if (isset($form['#action'])) {
          unset($form['#action']);
        }
        // x($form);
        return $form;

}

/**
 * Returns list of saved searches, connected with dataset
 */
function tow_search_get_saved_searches($dataset_nid) {
  $saved_search_res = db_query("SELECT `ctss`.`nid`, `n`.`title`
                                     FROM content_type_saved_search `ctss`
                                     LEFT JOIN node `n`
                                       ON `n`.`nid` = `ctss`.`nid`
                                     WHERE `ctss`.`field_dataset_nid_value` = %d" , $dataset_nid);
  // TODO: pass saved search nids (?)
  while ($saved_search = db_fetch_array($saved_search_res)) {
    $saved_searches[] = $saved_search['title'];
  }
  if ($saved_searches) {
    return $saved_searches;
  }
  else
  return array();
}

/**
 * Returns filter block elements
 */
function _tow_search_get_filter_element($fields, $enum_options) {

  // All elements that may be displayed in constraint area are described here. Furher hiding or
  // displaying is managed by js.
  $element = array(
    '#tree' => TRUE,
    '#prefix' => '<div>',
    '#suffix' => '</div>',
    'remove' => array(
      '#value' => t('Remove'),
      '#prefix' => '<div class="remove-filter">',
      '#suffix' => '</div>',
  	  '#weight' => 0,	  
      ),
    'field' => array(
      '#type' => 'select',
      //'#attributes' => array('style' => 'display:none'),
      '#options' => $fields,
      '#title' => t('Field'),
      '#weight' => 1,
      ),
    'operator' => array(
      '#type' => 'select',
      '#options' => tow_search_get_operators(),
      '#title' => t('operator'),
      '#weight' => 2,
      ),
    'value_1' => array(
      '#type' => 'textfield',
      '#title' => t('Value 1'),
      '#prefix' => '<div class="gen-value-field-1">',
      '#suffix' => '</div>',
      '#weight' => 3,
      ),
    'value_2' => array(
      '#type' => 'textfield',
      '#title' => t('Value 2'),
      '#prefix' => '<div class="gen-value-field-2">',
      '#suffix' => '</div>',
      '#weight' => 4,
      ),
    'date-within' => array(
  		'#type' => 'select',
      	'#options' => tow_search_get_date_within_operators(),
      	'#title' => t('Date within'),
        '#weight' => 5,
      ),
    'date_value_1' => array(
  		'#type' => 'textfield',
  		'#title' => 'Date value',
      	'#weight' => 6,
      ),
    'date_value_2' => array(
  		'#type' => 'textfield',
  		'#title' => 'Second date value',
      	'#weight' => 8,
      ),
    'enum_list' => array(
  		'#type' => 'select',
      	'#options' => $enum_options,
      	'#title' => t('Enum options'),
        '#multiple' => true,
        '#weight' => 10,
      ),

      );
      // nice date element
      yuicalendar_build_form($element, 'date_value_1', time(), 7, NULL, TRUE);
      yuicalendar_build_form($element, 'date_value_2', time(), 9, NULL, TRUE);
      return $element;
}

/**
 * Returns field titles of all the tables from the dataset $nid
 */
function tow_search_get_fields($nid) {

  $possible_fields = tow_possible_fields();
  $res = db_query("SELECT nid FROM {content_type_table} WHERE field_dataset_value = %d", $nid);

  while ($node = db_fetch_object($res)) {
    $node = node_load($node->nid);
    $table_fields = _tow_get_table_fields($node);

    $field_id = 0;
    foreach($table_fields as $field) {
      $value = 't' . $node->nid . '-' . $field['type']. '-' . $field_id++;
      $fields[$value] = $field['title'];
    }
  }
  return $fields;
}

/**
 * Returns comparison operators, that may be used in constraints
 */
function tow_search_get_operators() {
  $operators = array(	'text-equal'    => 'equals',
  				'text-contains' => 'contains',
  				'text-notcontains'  => 'doesn\'t contain',
  				'text-length-equal' => 'length =',
  				'text-length-greater' => 'length >',
                'text-length-less'     => 'length <',

                'int-equal'    => 'equals', 
  				'int-greater' => 'greater than',
                'int-less'     => 'less than',
                'int-between' => 'between',
  				'int-notequals'  => 'not equals',

                'date-equal'    => 'equals', 
  				'date-less'     => 'before',
                'date-greater' => 'after',
                'date-between' => 'between',
  				'date-within'  => 'within',

  				'enum-equal'    => 'equals', 
  				'enum-notequals'  => 'not equals',
                'enum-anyof'     => 'any of',
  );
  return $operators;
}

/**
 * Returns some more operators for date-within filtering operator
 */
function tow_search_get_date_within_operators() {
  $operators = array(	'3-days'    => '3 days',
  				'3-weeks' => '3 weeks',
  				'3-months'  => '3 months',
  				'1-year' => '1 year',
  				'2-years' => '2 years',
                '5-years'     => '5 years',

  );
  return $operators;
}

/**
 * Returns all enum options for all the tables from the dataset $nid
 */
function tow_search_get_enum_options($nid) {

  $all_enum_options = array();

  $res = db_query("SELECT `cfeo`.`nid` as `table_nid`, `cfeo`.`field_enum_options_value` as `enum_opt`
  					FROM {content_field_enum_options} `cfeo`
  					LEFT JOIN {content_type_table} `ctt`
  					ON `cfeo`.`nid` = `ctt`.`nid`
  					WHERE `ctt`.`field_dataset_value` = %d", $nid);

  while ($enum_options = db_fetch_array($res)) {
    if ($enum_options['enum_opt']) {
      $table_enum_options = unserialize($enum_options['enum_opt']);
      foreach ($table_enum_options as $old_key => $option) {
        $new_key = 't' . $enum_options['table_nid'] . '-' . $old_key;
        $table_enum_options[$new_key] = $option;
        unset($table_enum_options[$old_key]);
      }

      $all_enum_options = array_merge($all_enum_options, $table_enum_options);
    }
  }
  return $all_enum_options;
}

/**
 * Returns list of dataset tables together with links between these tables
 */
function tow_table_list_with_links($dataset_nid) {
  // 0. Get flat list of tables of a dataset
  $table_list = tow_table_list($dataset_nid);
  // 1. Get titles of indexed fields
  $table_index_fields = array();
  foreach ($table_list as $table_nid => $table_title) {

    $table_node = node_load($table_nid);

    $table_fields = _tow_get_table_fields($table_node);

    $key_column_titles = array();
    foreach ($table_fields as $col_index => $field) {
      if ($field['index']) {
        $key_column_titles[$col_index] = $field['title'];
      }
    }
    $table_index_fields[$table_nid] = $key_column_titles;
  }

  // 2.0
  $linked_tables = array();
  // 2.1 Compare index fields of the tables and determine linked tables
  foreach ($table_index_fields as $table_nid => $index_fields) {
    foreach ($index_fields as $field_index => $field_title) {
      foreach ($table_index_fields as $table_nid_2 => $index_fields_2) {
        // go to comparing field titles only for different tables
        if ($table_nid != $table_nid_2) {
          foreach ($index_fields_2 as $field_index_2 => $field_title_2) {
            if ($field_title ==  $field_title_2) {
              // titles match
              $linked_tables[$table_nid][$table_nid_2] = $table_list[$table_nid_2];
              // stop checking this table
              break;
            } // if

          } // 2nd field loop
        }  // if
      } // 2nd table loop
    } // 1st field loop
  } // 1st table loop

  // 3. Format list of tables for display
  $linked_tables_title_list = array();
  foreach ($table_list as $table_nid => $table_title) {
    // display string contains table title itself
    $linked_table_titles = $table_title;
    // and titles of linked tables, if there are any linked tables
    if (array_key_exists($table_nid, $linked_tables)) {
      $linked_table_titles .= ' (linked to ';
      foreach ($linked_tables[$table_nid] as $linked_table_nid => $linked_table_title) {
        $linked_table_titles .= $linked_table_title . ', ';
      }
      $linked_table_titles = substr($linked_table_titles, 0, -2);
      $linked_table_titles .= ')';
    }
    $linked_tables_title_list[$table_nid] = $linked_table_titles;
  }

  return $linked_tables_title_list;
}

/**
 * Redirects to the page with results. Final URL contains search string and
 * additional filter: search in records only
 */
function tow_search_within_dataset_submit_simple($form, &$form_state) {
  //x('submit simple search called');

  $args = implode($_GET);

  // Submit may be called from two types of URL:
  // search/apachesolr_search/test?filters=type:record,sis_cck_field_dataset_nid:[dataset-nid] and
  // search_dataset/[dataset-nid]
  // The following lines extract dataset-nid from both URLs

  $matches_qty = preg_match('/(?<=dataset_nid:|search_dataset\/)\d{1,}$/', $args, $dataset_nids);
  //x($dataset_nids);
  if ($matches_qty) {
    $dataset_nid = $dataset_nids[0];
  }
  else {
    // TODO: GOTO 404 page
  }

  // tell apachesolr to perform search
  $path = 'search/apachesolr_search/' . $form_state['clicked_button']['#post']['simple_search']['textfield'];

  // search in records only
  $query = 'filters=type:record';

  // limit search to the specified dataset only, separate search option by space
  $query .= ' ' . 'sis_tow_dataset_nid:' . $dataset_nid;
  drupal_goto($path, $query);

}

/**
 * Composes URL with all the search parameters and redirects user to it
 */
function tow_search_within_dataset_submit($form, &$form_state) {
  x($form_state);
  // query example
  /*$query .= 'filter=' . $form_state['values']['field'] . '&op=' . $form_state['values']['operator'] . '&value=' . $form_state['values']['value'];
  drupal_goto('table/' . $table->nid, $query);
  */
  
  $query = '';
  // dataset nid
  //$query .= 'dataset=' . $form_state['values']['dataset_nid'];
  
  // scanned tables
  $query .= 'scan_tables=';
  foreach ($form_state['values']['tables']['select'] as $value) {
       $query .= $value . ','; 
  }
   $query = substr($query, 0, -1);
  
  // tables to be displayed
  $query .= '&disp_tables=';
  $selected_tables = array_keys($form_state['values']['fields']['select_tables'], true);
  foreach ($selected_tables as $table_nid) {
     $query .= $table_nid . ','; 
  }
  $query = substr($query, 0, -1);
    
  // fields to be displayed
  $query .= '&disp_fields=';
  $selected_tables = array_keys($form_state['values']['fields']['select_fields'], true);
  foreach ($selected_tables as $table_nid) {
     $query .= $table_nid . ','; 
  }
  $query = substr($query, 0, -1);
  
  // save search?
  if ($form_state['values']['save_search']['save']) {
    $query .= '&save=' . $form_state['values']['save_search']['save'];  
    $query .= '&ssname=' . $form_state['values']['save_search']['textfield'];
    // TODO: do not add if false 
    $query .= '&ovwr=' . $form_state['values']['save_search']['overwrite'];
  
  }
  
  
  
  
  drupal_goto('search_dataset/' . $form_state['values']['dataset_nid'], $query);
  
  //x('tow_search_submit called');
  //x($form_state['values']['dataset_nid']);
  // Save search 
  /*if ($form_state['values']['save_search']['save']) {
    $new_ss = new stdClass();
    // common node parameters 
    $new_ss->type = 'saved_search';
    // TODO:
    //$new_ss->uid = 
    $new_ss->title = $form_state['values']['save_search']['textfield'];
    // nid field, if saved search with the same name exists and we are overwriting it
    // TODO:
    if (in_array($form_state['values']['save_search']['textfield'], $form_state['values']['saved_searches']) &&
      $form_state['values']['save_search']['overwrite']) {
        // TODO: $new_ss->nid = 
//        $new_ss->title = $form_state['values']['save_search']['textfield'];
      }
   
    // searched tables field
    foreach($form_state['values']['tables']['select'] as $value) {
      $new_ss->field_scanned_tables[]['value'] = $value; 
    }   
    // dataset field 
    $new_ss->field_dataset_nid[0]['value'] = $form_state['values']['dataset_nid'];
    // displayed tables
    $selected_tables = array_keys($form_state['values']['fields']['select_tables'], 1);
    foreach ($selected_tables as $table_nid) {
      $new_ss->field_displayed_tables[]['value'] = $table_nid;   
    }  
    // displayed fields
    $selected_fields = array_keys($form_state['values']['fields']['select_fields'], 1);
    $new_ss->field_displayed_fields[0]['value'] = serialize($selected_fields); 

    // search constraints 
    $new_ss->field_search_constraints[0]['value'] = serialize($form_state['values']['filters']['filter-items-wrapper']['filter-items']);
    
  }
  
  
  //x($form_state);
  // TODO: add number of results
  // 
  node_save($new_ss);*/
}