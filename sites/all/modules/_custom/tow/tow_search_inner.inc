<?php
/**
 * Creates form elements for the search_within_dataset page
 */
function tow_search_within_dataset_altering_form(&$form, &$form_state, $dataset_nid) {
	//x('tow_search_alter_form called');
	//x($form);
	//x($dataset_nid);
  
  global $dataset;
  $dataset = node_load($dataset_nid);
  
  if ($dataset->type != 'dataset') {
    drupal_access_denied();
    die();
  }
  
  drupal_add_js(drupal_get_path('module', 'tow') . '/js/search.js');
  
  drupal_set_title($dataset->title);
  
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
      '#options' => tow_table_list($dataset_nid),
      '#multiple' => TRUE
    )
  );
  
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
      '#value' => $form_state['values']['filters']['existing_fields']
    );
  }
  else {
    $form['filters']['existing_fields'] = array(
      '#type' => 'value',
      '#value' => tow_search_get_fields($dataset_nid)
    );
    $form['filters']['existing_operators'] = array(
      '#type' => 'value',
      '#value' => tow_search_get_operators()
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
      'method' => 'append'
    ),
  );
  
  $form['filters']['filter-items-wrapper'] = array(
    '#prefix' => '<div id="filter-items-wrapper">',
    '#suffix' => '</div>',
    'filter-items' => _tow_search_get_filter_element($form['filters']['existing_fields']['#value'], $form['filters']['existing_operators']['#value'])  
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
      '#title' => t('Fields')
    )
  );
  
  $form['agsp'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Averaging, grouping, sorting and processing the results'),
    'result_type' => array(
      '#type' => 'select',
      '#options' => array('Individual results'),
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

function _tow_search_get_filter_element($fields) {

  $element = array(
    '#tree' => TRUE,
    '#prefix' => '<div>',
    '#suffix' => '</div>',
    'remove' => array(
      '#value' => t('Remove'),
      '#prefix' => '<div class="remove-filter">',
      '#suffix' => '</div>'
    ),
    'field' => array(
      '#type' => 'select',
      //'#attributes' => array('style' => 'display:none'),
      '#options' => $fields,
      '#title' => t('Field'),
    ),
    'operator' => array(
      '#type' => 'select',
      '#options' => tow_search_get_operators(),
      '#title' => t('operator'),
    ),
    'value' => array(
      '#type' => 'textfield',
      '#title' => t('Value'),
      '#prefix' => '<div class="value-field">',
      '#suffix' => '</div>',
    )
  );
  
  return $element;
}

function tow_search_get_fields($nid) {
  
  $possible_fields = tow_possible_fields();
  $res = db_query("SELECT nid FROM {content_type_table} WHERE field_dataset_value = %d", $nid);
  
  while ($node = db_fetch_object($res)) {
    $node = node_load($node->nid);
    $field_id = 0;
    foreach ($possible_fields as $field) {
      foreach ($node->{'field_title_' . $field} as $value) {
        if ($value['value'])
          $fields['t' . $node->nid . '-' . $field_id++] = $value['value'];
      }
    }
  }
  return $fields;
}

function tow_search_get_operators() {
  return array('>', '<', '=');
}

function _tow_search_outer_default_subject($tids, $filters) {
  if ($filters) {
    $start = strpos($filters, 'tid:');
    if ($start !== FALSE) {
      $end = strpos($filters, ' ', $start + 4);
      
      if ($end === FALSE) {
        $tid = substr($filters, $start + 4);
      }else{
        $tid = substr($filters, $start + 4, $end - $start - 4);
      }
    }
    if (in_array($tid, $tids)) {
      return $tid;
    }
  }
  return 0;  
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

function tow_search_within_dataset_submit($form, &$form_state) {
 x('success 2');
}