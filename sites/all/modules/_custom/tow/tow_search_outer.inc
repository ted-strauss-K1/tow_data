<?php

function tow_search_outer_filter_block() {
  
  $block['title'] = NULL;
  $block['content'] = drupal_get_form('tow_search_outer_filter_form');
  
  return $block;
}

function tow_search_outer_filter_taxonomy() {
  
  $terms = array(t('Any'));
  
  $res = db_query("SELECT tid, name FROM {term_data} WHERE vid = 2 ORDER BY name ASC");
  while ($term = db_fetch_array($res))
    $terms[$term['tid']] = $term['name'];
  return $terms;

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
 * Creates form elements for the search_datasets page
 */
function tow_search_datasets_altering_form(&$form) {
  $form['subjects'] = array(
		'#type' => 'select',
		'#options' => tow_search_outer_filter_taxonomy(),
		'#title' => t('Subjects')
	  );
	
  $form['subjects']['#default_value'] = _tow_search_outer_default_subject(array_keys($form['subjects']['#options']), $filters); 

	$form['size'] = array(
		'#type' => 'select',
		'#options' => array(
		  t('Any'),
		  t('More than 100 rows'),
		  t('More than 500 rows'),
		  t('More than 1000 rows'),
		  t('More than 5000 rows')
		),
		'#title' => t('Size of data set')
	  );
	  
	$form['saved_searches'] = array(
		'#type' => 'select',
		'#options' => array(
		  t('Any'),
		  t('At least 1'),
		  t('At least 5'),
		  t('At least 10')
		),
		'#title' => t('Saved searches')
	  );
	  
	$form['post_date'] = array(
		'#type' => 'select',
		'#options' => array(
		  t('Any'),
		  t('One day ago'),
		  t('One week ago'),
		  t('One month ago'),
		  t('6 months ago')
		),
		'#title' => t('Post date (up to)'),
	  );
	  
	$form['access_status'] = array(
		'#type' => 'select',
		'#options' => array(
		  t('Any'),
		  t('Open'),
		  t('By request'),
		  t('Private')
		),
		'#title' => t('Access status'),
	  );
	  
	$form['popularity'] = array(
		'#type' => 'select',
		'#options' => array(
		  t('Any'),
		  t('Low'),
		  t('Moderate'),
		  t('High')
		),
		'#title' => t('popularity (# views)'),
	  );
  
	$form['#submit'] = array('tow_search_outer_filter_form_submit');
}

/**
 * Composes an URL upon submission of search-datasets form
 * URL is understandable by apachesolr and contains all the search parameters
 */
function tow_search_outer_filter_form_submit($form, &$form_state) {
  // x('tow_search_outer_filter_form_submit called');
  // x($form_state);
  $fv = $form_state['values'];
  
  $path = 'search/apachesolr_search/' . $fv['processed_keys'];
  $query = 'filters=';
  
  $filters = array();
  
  // search in datasets only
  $query .= 'type:dataset';
  
  // subjects
  // each consequent search option should be separated from previous by whitespace
  if ($form_state['values']['subjects']) {
    $query .= ' ' . 'tid:' . $form_state['values']['subjects'];
  }
    
  if ($query == 'filters=')
    $query = NULL;
  //x($path);
  //x($query);
  drupal_goto($path, $query);
}
