<?php
/**
 * Re-implementation of search_view(). Modification of apachesolr_search_view().
 */
function tow_outer_search_view($type = 'apachesolr_search') {
 
  $type = 'apachesolr_search';
  $keys = trim(tow_get_keys());

  // We did not redirect, so run the search if needed.
  $results = '';
  $filters = '';
//  if (!isset($_GET['filters'])) {
//	$_GET['filters'] = 'type:dataset';
// }
  $filters = trim($_GET['filters']);
  // Only perform search if there is non-whitespace search term or filters:
  if ($keys || $filters) {
	// Log the search keys:
	$log = $keys;
	if ($filters) {
	  $log .= 'filters='. $filters;
	}
	watchdog('search', '%keys (@type).', array('%keys' => $log, '@type' => t('Search')), WATCHDOG_NOTICE, l(t('results'), 'search/'. $type .'/'. $keys));

	// Collect the search results:
	$results = search_data($keys, 'tow');


	if ($results) {
	  $results = theme('box', t('Search results'), $results);
	}
	else {
	  $results = theme('box', t('Your search yielded no results'), variable_get('apachesolr_search_noresults', apachesolr_search_noresults()));
	}
  }
  return $results;
} 

/**
 *   Modification of search_get_keys()
 */
function tow_get_keys() {
  static $return;
  if (!isset($return)) {
	// Extract keys as remainder of path
	// Note: support old GET format of searches for existing links.
	$path = explode('/', $_GET['q'], 2);
	$keys = empty($_REQUEST['keys']) ? '' : $_REQUEST['keys'];
	$return = count($path) == 2 ? $path[1] : $keys;
  }
  return $return;
}

/**
 * Home page search block
 */
function tow_search_outer_filter_block() {
  
  $block = array();
  
  $block['title'] = 'Filters';
  $block['content'] = drupal_get_form('tow_search_outer_filter_form');
  
  return $block;
 
}

/**
 * Creates form elements for home page search block
 */
function tow_search_outer_filter_form($form_state) {

	$filters = $_GET['filters'];
	
	$nopost = (count($form_state['post']) == 0);

	$form['keywords'] = array(
		'#type' => 'textfield',
		'#title' => t('keywords'),
	); 
	if ($nopost && $keys = trim(tow_get_keys())) 
		$form['keywords']['#value'] = $keys;

	
	$form['size'] = array(
		'#type' => 'select',
		'#options' => array(
			'0' => t('any'),
			'10' => t('10+'),
			'100' => t('100+'),
			'500' => t('500+'),
			'1000' => t('1000+'),
			'5000' => t('5000+'),
		),
		'#title' => t('# rows'),  
	);
		
	if ($nopost && preg_match('/(?<=sis_tow_amount_records:\[)\d{1,}/', $filters, $n)) 
		$form['size']['#value'] = $n[0];
	  
	
	$form['saved_searches'] = array(
		'#type' => 'select',
		'#options' => array(
			'0' => t('any'),
			'1' => t('1+'),
			'5' => t('5+'),
			'10' => t('10+'),
			'20' => t('20+'),
			'50' => t('50+'),
		),
		'#title' => t('# searches'),
	);
	if ($nopost && preg_match('/(?<=sis_tow_amount_saved_searches:\[)\d{1,}/', $filters, $n))
		$form['saved_searches']['#value'] = $n[0];
		
	$form['access_status'] = array(
		'#type' => 'select',
		'#options' => array(
			'-1' => t('any'),
			'0' => t('open'),
			'1' => t('by request'),
		),
		'#title' => t('access'),
	);
	if ($nopost && preg_match('/(?<=sis_cck_field_access_type:)\d{1}/', $filters, $n))
		$form['access_status']['#value'] = $n[0];
		
	$form['forum'] = array(
		'#type' => 'select',
		'#options' => array(
			t('any'),
			t('active'),
		),
		'#title' => t('forum'),
		'#disabled' => TRUE,
	  );

	  $form['docs'] = array(
		'#type' => 'checkbox',
		'#title' => t('docs'),
		'#disabled' => TRUE,
	  );

	$form['categories'] = array(
		'#type' => 'select',
		'#options' => tow_search_outer_filter_taxonomy(),
		'#title' => t('categories'),
	);
	$matches_qty = preg_match_all('/(?<=tid:)\d{1,}/', $filters, $n);
	$category = '';
	foreach ($n[0] as $tid) {
		if (in_array($tid, array_flip(tow_search_outer_filter_taxonomy()))) {
			$category = $tid;
			break;
		}
	}
	if ($nopost && $matches_qty)
		$form['categories']['#value'] = $category;

	$form['submit'] = array(
		'#type' => 'submit',
		'#title' => t('Search'),
		'#value' => t('Search'),
	); 
	  
	if (apachesolr_has_searched()) {

		$response = apachesolr_static_response_cache();
		if (empty($response)) {
		  return;
		}
		$query = apachesolr_current_query();

		$vid1_block = apachesolr_search_taxonomy_facet_block($response, $query,'im_vid_1');
		$form['tags'] = array(
			'#type' => 'item',
			'#title' => 'tags',
			'#value' => $vid1_block['content'],
		);
		drupal_add_js(drupal_get_path('module', 'tow') . '/js/outer_search.js');
	}
	return $form;
}

/**
 * Validation function
 */
function tow_search_outer_filter_form_validate($form, &$form_state) {

}

/**
 * Composes an URL upon submission of form at homepage dataset search block
 * URL is understandable by apachesolr and contains all the search parameters
 */
function tow_search_outer_filter_form_submit($form, &$form_state) {

	$form_state['rebuild'] = TRUE;

	$fv = $form_state['values'];

	$path = 'newest/' . $fv['keywords'];
	$query = 'filters=';

	// subjects
	// each consequent search option should be separated from previous by whitespace
	if ($form_state['values']['size']) {
		$query .= ' ' . 'sis_tow_amount_records:[' . $form_state['values']['size'] . ' TO *]';
	}

	if ($form_state['values']['saved_searches']) {
		$query .= ' ' . 'sis_tow_amount_saved_searches:[' . $form_state['values']['saved_searches'] . ' TO *]';
	}	

	if ($form_state['values']['access_status'] !== '-1') {
		$query .= ' ' . 'sis_cck_field_access_type:' . $form_state['values']['access_status'];
	}

	if ($form_state['values']['categories']) {
		$query .= ' ' . 'tid:' . $form_state['values']['categories'];
	}

	if ($query == 'filters=') $query = NULL;

	drupal_goto($path, $query);
}

/**
 * Return array of 'Dataset category' terms with keys being tid's
 */
function tow_search_outer_filter_taxonomy() {
 
  $terms = array('0' => t('Any'));
  
  $res = db_query("SELECT tid, name FROM {term_data} WHERE vid = 2 ORDER BY name ASC");
  while ($term = db_fetch_array($res))
	$terms[$term['tid']] = $term['name'];
  return $terms;
}

/**
 * Block of facets for amount of tables (test facet block)
 */
function tow_sis_tow_amount_tables_block() {
  
	if (apachesolr_has_searched()) {

		$response = apachesolr_static_response_cache();
		if (empty($response)) {
		  return;
		}
		$query = apachesolr_current_query();
		$delta = 'sis_tow_amount_tables';
		return apachesolr_facet_block($response, $query, 'sis_tow_amount_tables', $delta, $delta, t('Filter by Amount of tables'), 'Amount of tables');
	}  
}