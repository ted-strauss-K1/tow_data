<?php
/**
 * Re-implementation of search_view(). Modification of apachesolr_search_view().
 */
function tow_outer_search_view($type = 'apachesolr_search') {
	if ($_GET['q'] == 'home') {
		$_GET['q'] = 'newest';
	}
	$type = 'apachesolr_search';
	$keys = trim(tow_get_keys());

	// We did not redirect, so run the search if needed.
	$results = '';

	$filters = tow_get_search_outer_filters();
	$filters = $filters['query'];
	$_GET['filters'] = $filters;
	
	
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

	return $results;
} 

/**
 * Proxy for outer search using AJAX.
 */
function tow_search_outer_ajax() {
	$_GET['q'] = str_replace('search_outer_ajax/', '', $_GET['q']);
	$results = tow_outer_search_view();
	$form = drupal_get_form('tow_search_outer_filter_form');
	$number_block = tow_search_outer_number_of_results_block();
	$tabs_block = tow_search_outer_tabs_block();
	return drupal_json(array(
		'results' => $results,
		'form' => $form, 
		'number' => $number_block['content'], 
		'tabs' => $tabs_block['content'], 
		
	));
} 

/**
 *	 Modification of search_get_keys()
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
 *	 Returns tab for homepage (featured, newest, popular, debated)
 */
function tow_get_homepage_tab() {
	static $tab;
	if (!isset($tab)) {
	// Extract keys as remainder of path
	// Note: support old GET format of searches for existing links.
	$path = explode('/', $_GET['q'], 2);
	$tab = $path[0];
	if (!in_array($tab, array('featured', 'newest', 'popular', 'debated', 'search_dataset')))
		$tab = tow_get_search_outer_default_tab();
	}
	return $tab;
}


/**
 *	 Returns default tab for home page
 */
function tow_get_search_outer_default_tab() {
	return 'newest';
}


/**
 *	 Returns values for oter search filters retrieved from $_GET array
 */
function tow_get_search_outer_filters() {
	static $values;
	if (!isset($values)) {
		$filters = $_GET['filters'];	
		$values = array();
		$values['query'] = '';
		if (preg_match('/(?<=sis_tow_amount_records:\[)\d{1,}/', $filters, $n)) {
			$values['size'] = $n[0];
			$values['query'] .= 'sis_tow_amount_records:[' . $n[0] . ' TO *] ';
		} else {
			$values['size'] = FALSE;
		}

		if (preg_match('/(?<=sis_tow_amount_saved_searches:\[)\d{1,}/', $filters, $n)) {
			$values['saved_searches'] = $n[0];
			$values['query'] .= 'sis_tow_amount_saved_searches:[' . $n[0] . ' TO *] ';
		} else {
			$values['saved_searches'] = FALSE;
		}
		
		if (preg_match('/(?<=sis_cck_field_access_type:)\d{1}/', $filters, $n)) {
			$values['access_status'] = $n[0];
			if ($values['access_status'] == 2)
				$values['access_status'] = FALSE;
			if ($values['access_status']) 
				$values['query'] .= 'sis_cck_field_access_type:' . $n[0] . ' ';
		} else {
			$values['access_status'] = FALSE;			
		}
		
		$matches_qty = preg_match_all('/(?<=tid:)\d{1,}/', $filters, $n);

		$values['categories'] = '';
		$values['tags_query'] = '';
		$values['tags'] = array();

		$outer_tids = array_flip(tow_search_outer_filter_taxonomy());
		foreach ($n[0] as $tid) {
			if (in_array($tid, $outer_tids)) {
				$values['categories'] = $tid;
			} else {
				$values['tags_query'] .= ' ' . 'tid:' . $tid;
				$values['tags'][] = $tid;
			}
			$values['query'] .= 'tid:' . $n[0] . ' ';
		}

	}
	return $values;
}



/**
 * Block displaying number of results found
 */

function tow_search_outer_number_of_results_block() {
	$block = array();
	$block['title'] = NULL;
	if (apachesolr_has_searched()) {
		$response = apachesolr_static_response_cache();
		$total = $response->response->numFound;
		
		if ($number_of_results == 1) {
			$content =	$total . ' ' . t('dataset');
		} else {
			$content =	$total . ' ' . t('datasets');
		}
		
		$keys = trim(tow_get_keys());
		$values = tow_get_search_outer_filters();
		
		$filters = '';
		
		if ($keys)
			$filters[] = 'keywords: ' . $keys;
			
		if ($value = $values['size'])
			$filters[] = $value . '+ rows';
			
		if ($value = $values['saved_searches'])
			$filters[] = $value . '+ searches';
			
		if ($values['access_status'] !== FALSE)
			$filters[] = 'access: ' . tow_get_access_status($values['access_status']);
			
		if ($value = $values['categories']) 
			$filters[] = 'category: ' . taxonomy_get_term($value)->name;
			
		foreach ($values['tags'] as $tid) {
			$filters[] = 'tag: ' . taxonomy_get_term($tid)->name;
		}
			
		$block['content'] = $content . theme_item_list($filters);

	} else {
		$block['content'] = ' ';
	}
	return $block;	
}

/**
 * Block displaying tabs at home page
 */

function tow_search_outer_tabs_block() {
	$tab = tow_get_homepage_tab();
	if (!$tab) 
		$tab = tow_get_search_outer_default_tab();
	
	$content = '';
	$disabled = array('attributes' => array('disabled' => 'true'));

	$content .= tow_tab_link($tab, 'Featured', 'featured', TRUE);
	$content .= ' | ' . tow_tab_link($tab, 'Newest', 'newest');
	$content .= ' | ' . tow_tab_link($tab, 'Popular', 'popular');
	$content .= ' | ' . tow_tab_link($tab, 'Debated', 'debated', TRUE);
	$content .= ' | ' . l('Create new dataset','node/add/dataset');

	$block = array();
	$block['title'] = NULL;
	$block['content'] =	$content;

	return $block;
}

/**
 * Block displaying search results at home page
 */

function tow_search_outer_results_block() {
	if (!$_GET['q'])
		$_GET['q'] = tow_get_search_outer_default_tab();
	tow_set_search_outer_default_tab(); 
	$block['title'] = NULL;
	$block['content'] =	tow_outer_search_view();
	drupal_add_js(drupal_get_path('module', 'tow') . '/js/jquery.ba-hashchange.min.js');
	drupal_add_js(drupal_get_path('module', 'tow') . '/js/outer_search.js');

	return $block;
}

/**
 * Returns link to homepage tab
 */

function tow_tab_link($current_tab, $title, $tab, $disable = FALSE) {
	if ($disable)
		return $title;
	elseif ($current_tab == $tab)
		return '<b>' . $title . '</b>';
	else 
		return l($title,$tab, array('attributes' => array('class' => 'tow-search-outer-tab')));
		//return l($title,'<front>', array('fragment' => $tab, 'attributes' => array('class' => 'tow-search-outer-tab')));
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
	$values = tow_get_search_outer_filters();
	
	$nopost = (count($form_state['post']) == 0);

	$form['keywords'] = array(
		'#type' => 'textfield',
		'#title' => t('keywords'),
	); 

	$form['submit'] = array(
		'#type' => 'submit',
		'#title' => t('Search'),
		'#value' => t('Search'),
	); 

	if ($nopost && $keys = trim(tow_get_keys())) 
		$form['keywords']['#value'] = $keys;

//	$start = microtime(TRUE);
	$form['size'] = array(
		'#type' => 'select',
		'#options' => tow_facet_menu_solr_range('sis_tow_amount_records', array(
			'0' => t('any'),
			'10' => t('10+'),
			'100' => t('100+'),
			'500' => t('500+'),
			'1000' => t('1000+'),
			'2000' => t('2000+'),
			'5000' => t('5000+'),
		), TRUE),
		'#title' => t('# rows'),	
	);
//	$end = microtime(TRUE);
//	dpm($end-$start);
		
	if ($nopost && $value = $values['size']) 
		$form['size']['#value'] = $value;
		
	
	$form['saved_searches'] = array(
		'#type' => 'select',
		'#options' => tow_facet_menu_solr_range('sis_tow_amount_saved_searches', array(
			'0' => t('any'),
			'1' => t('1+'),
			'5' => t('5+'),
			'10' => t('10+'),
			'20' => t('20+'),
			'50' => t('50+'),
		), TRUE),
		'#title' => t('# searches'),
	);
	if ($nopost && $value = $values['saved_searches'])
		$form['saved_searches']['#value'] = $value;
		
	$form['access_status'] = array(
		'#type' => 'select',
		'#options' => tow_facet_menu('sis_cck_field_access_type', array(
			'any' => t('any'),
			'0' => t('open'),
			'1' => t('by request'),
//			'2' => t('private'),
		)),
		'#title' => t('access'),
	);


	if ($nopost && $values['access_status'] !== FALSE)
		$form['access_status']['#value'] = $values['access_status'];
		
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
		'#options' => tow_facet_menu('im_vid_2', tow_search_outer_filter_taxonomy()),
		'#title' => t('categories'),
	);

	if ($nopost && $value = $values['categories'])
		$form['categories']['#value'] = $value;

	$form['tags_query'] = array(
		'#type' => 'hidden',
		'#value' => $values['tags_query'],
	); 

	$form['search_placeholder'] = array(
		'#type' => 'hidden',
		'#value' => t('Searching...'),
	); 

	$form['current_tab'] = array(
		'#type' => 'hidden',
		'#value' => tow_get_homepage_tab(),
	); 
	
	$form['current_url'] = array(
		'#type' => 'hidden',
		'#value' => $_GET['q'] . ($_GET['filters'] ? '?filters=' . $_GET['filters'] : ''),
	); 
	
	if (apachesolr_has_searched()) {

		$response = apachesolr_static_response_cache();
		if (empty($response)) {
			return;
		}
		$query = apachesolr_current_query();

		$vid1_block = apachesolr_search_taxonomy_facet_block($response, $query,'im_vid_1');
		if ($tags = $vid1_block['content']) {
			$form['tags'] = array(
				'#type' => 'item',
				'#title' => 'tags',
				'#value' => $tags,
			);
		}
		
	}
	drupal_add_js(drupal_get_path('module', 'tow') . '/js/jquery.ba-hashchange.min.js');
	drupal_add_js(drupal_get_path('module', 'tow') . '/js/outer_search.js');
	
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

	$path = tow_get_homepage_tab() . '/' . $fv['keywords'];
	$query = 'filters=';

	// subjects
	// each consequent search option should be separated from previous by whitespace
	if ($form_state['values']['size']) {
		$query .= ' ' . 'sis_tow_amount_records:[' . $form_state['values']['size'] . ' TO *]';
	}

	if ($form_state['values']['saved_searches']) {
		$query .= ' ' . 'sis_tow_amount_saved_searches:[' . $form_state['values']['saved_searches'] . ' TO *]';
	}	

	if ($form_state['values']['access_status'] !== 'any') {
		$query .= ' ' . 'sis_cck_field_access_type:' . $form_state['values']['access_status'];
	}

	if ($form_state['values']['categories'] !== 'any') {
		$query .= ' ' . 'tid:' . $form_state['values']['categories'];
	}
	
	$matches_qty = preg_match_all('/(?<=tid:)\d{1,}/', $_GET['filters'], $n);
	foreach ($n[0] as $tid) {
		if (!in_array($tid, array_flip(tow_search_outer_filter_taxonomy()))) {
			$query .= ' ' . 'tid:' . $tid;
		}
	}
	
	if ($query == 'filters=') $query = NULL;

	drupal_goto($path, $query);
}

/**
 * Return array of 'Dataset category' terms with keys being tid's
 */
function tow_search_outer_filter_taxonomy() {
 
	$terms = array('any' => t('any'));
	
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
	return NULL;
}