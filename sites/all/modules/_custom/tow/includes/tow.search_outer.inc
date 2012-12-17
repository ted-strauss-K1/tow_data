<?php

/**
 * @file
 *
 * Functions for outer search.
 */
/* * **************************************************************************************************
 * Hook handlers
 * ************************************************************************************************** */

/**
 * Home page sponsored block.
 */
function tow_search_outer_sponsored_block() {

    // Construct block.
    $block = array();
    $block['title'] = 'Sponsored';
    $block['content'] = '<img src="' .  '/' . drupal_get_path('module', 'tow') . '/images/sponsored.png' . '" alt="">';

    return $block;
}

/**
 * All pages search input block.
 */
function tow_search_outer_searchinput_block() {

    $block = array();

    // Define block.
    $block['title'] = NULL;
    $block['content'] = drupal_get_form('tow_search_outer_searchinput_form');

    return $block;
}

/**
 * Home page search block.
 */
function tow_search_outer_filter_block() {

    $block = array();

    // Defone block.
    $block['title'] = 'Filters';
    $block['content'] = drupal_get_form('tow_search_outer_filter_form');

    return $block;
}

/**
 * Block displaying number of results found.
 */
function tow_search_outer_number_of_results_block() {
    $block = array();
    $block['title'] = NULL;

    if (apachesolr_has_searched()) {
        $response = apachesolr_static_response_cache();
        $total = $response->response->numFound;

        // Amount of datasets.
        if ($number_of_results == 1) {
            $content = '<span>' . $total . '</span>' . ' ' . t('dataset');
        } else {
            $content = '<span>' . $total . '</span>' . ' ' . t('datasets');
        }

        $keys = trim(tow_search_outer_get_keys());
        $values = tow_search_outer_get_filters();

        $filters = '';

        // Keywords.
        if ($keys) {
            $filters[] = 'keywords: ' . $keys;
        }

        // Rows amount.
        if ($value = $values['size']) {
            $filters[] = $value . '+ rows';
        }

        // Number of searches been saved.
        if ($value = $values['saved_searches']) {
            $filters[] = $value . '+ searches';
        }

        // Access status.
        if ($values['access_status'] !== FALSE) {
            $filters[] = 'access: ' . tow_access_get_access_status($values['access_status']);
        }

        // Taxonomy: categories.
        if ($value = $values['categories']) {
            $filters[] = 'category: ' . taxonomy_get_term($value)->name;
        }

        // Taxonomy: tags.
        foreach ($values['tags'] as $tid) {
            $filters[] = 'tag: ' . taxonomy_get_term($tid)->name;
        }

        // Construct block content.
        $block['content'] = $content . theme_item_list($filters);
    } else {
        $block['content'] = ' ';
    }

    return $block;
}

/**
 * Block displaying search results at home page.
 */
function tow_search_outer_results_block() {

    // Redirect to default page.
    if (!$_GET['q']) {
        $_GET['q'] = tow_search_outer_get_default_tab();
    }

    // Block settings.
    $block['title'] = NULL;
    $block['content'] = tow_search_outer_view();

    // Add necessary javascript functionality.
    drupal_add_js(drupal_get_path('module', 'tow') . '/js/jquery.ba-hashchange.min.js');
    drupal_add_js(drupal_get_path('module', 'tow') . '/js/outer_search.js');

    return $block;
}

/**
 * Block of facets for amount of tables (test facet block).
 */
function tow_search_outer_sis_tow_amount_tables_block() {

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

/**
 * Block displaying tabs at home page.
 */
function tow_search_outer_tabs_block() {

    // Get tab.
    $tab = tow_search_outer_get_homepage_tab();
    if (!$tab) {
        $tab = tow_search_outer_get_default_tab();
    }

    // Attributes.
    $content = '';
    $disabled = array(
        'attributes' => array(
            'disabled' => 'true'
        )
    );

    // A set of links.
    $items = array();
    $items['featured'] = tow_search_outer_tab_link($tab, 'Featured', 'featured', TRUE);
    $items['newest'] = tow_search_outer_tab_link($tab, 'Newest', 'newest');
    $items['popular'] = tow_search_outer_tab_link($tab, 'Popular', 'popular');
    $items['debated'] = tow_search_outer_tab_link($tab, 'Debated', 'debated', TRUE);
    global $user;
    $items['newdataset'] = ($user->uid) ? l('New dataset', 'node/add/dataset') : l('New dataset', 'user/login', array('query' => 'automodalReload=true', 'attributes' => array('class' => 'automodal')));

    // Theme the block.
    $block['content'] = theme_item_list($items, NULL, 'ul');

    return $block;
}

/**
 * Re-implementation of search_view(). Modification of apachesolr_search_view().
 */
function tow_search_outer_view($type = 'apachesolr_search') {
    if ($_GET['q'] == 'home') {
        $_GET['q'] = 'newest';
    }

    $type = 'apachesolr_search';
    $keys = trim(tow_search_outer_get_keys());

    // We did not redirect, so run the search if needed.
    $results = '';

    $filters = tow_search_outer_get_filters();
    $filters = $filters['query'];
    $_GET['filters'] = $filters;

    // Log the search keys:
    $log = $keys;
    if ($filters) {
        $log .= 'filters=' . $filters;
    }

    // Collect the search results:
    $results = search_data($keys, 'tow');

    if ($results) {
        $results = theme('box', NULL/*t('Search results')*/, $results);
    } else {
        $results = theme('box', t('Your search yielded no results'), variable_get('apachesolr_search_noresults', apachesolr_search_noresults()));
    }
    
    //Set breadcrumbs
    $bc = array();
    $bc[] = t('Data');
    drupal_set_breadcrumb($bc);
    
    return $results;
}

/* * **************************************************************************************************
 * Forms
 * ************************************************************************************************** */

/**
 * Create form elements for all pages search input block.
 */
function tow_search_outer_searchinput_form($form_state) {

    $form['keywords'] = array(
        '#type' => 'textfield',
    );

    $form['submit'] = array(
        '#type' => 'submit',
        '#title' => t('Search'),
        '#value' => t('Search'),
    );

    return $form;
}

/**
 * Creates form elements for home page search block.
 */
function tow_search_outer_filter_form($form_state) {
    $values = tow_search_outer_get_filters();

    $nopost = (count($form_state['post']) == 0);

    //Keywords
    $form['keywords'] = array(
        '#type' => 'textfield',
        '#title' => t('keywords'),
    );

    $form['submit'] = array(
        '#type' => 'submit',
        '#title' => t('Search'),
        '#value' => t('Search'),
    );

    if ($nopost && $keys = trim(tow_search_outer_get_keys())) {
        $form['keywords']['#value'] = $keys;
    }

    //Rows
  $form['size'] = array(
    '#type' => 'select',
    '#options' => tow_apachesolr_facet_menu_solr_range('sis_tow_amount_records', array(
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

    if ($nopost && $value = $values['size']) {
        $form['size']['#value'] = $value;
    }

    //Searches
  $form['saved_searches'] = array(
    '#type' => 'select',
    '#options' => tow_apachesolr_facet_menu_solr_range('sis_tow_amount_saved_searches', array(
      '0' => t('any'),
      '1' => t('1+'),
      '5' => t('5+'),
      '10' => t('10+'),
      '20' => t('20+'),
      '50' => t('50+'),
    ), TRUE),
    '#title' => t('# searches'),
  );

    if ($nopost && $value = $values['saved_searches']) {
        $form['saved_searches']['#value'] = $value;
    }

    //Access
  $form['access_status'] = array(
    '#type' => 'select',
    '#options' => tow_apachesolr_facet_menu('sis_cck_field_access_type', array(
      'any' => t('any'),
      '0' => t('open'),
      '1' => t('by request'),
    )),
    '#title' => t('access'),
  );

    if ($nopost && $values['access_status'] !== FALSE) {
        $form['access_status']['#value'] = $values['access_status'];
    }

    //Forum
 $form['forum'] = array(
    '#type' => 'select',
    '#options' => array(
      t('any'),
      t('active'),
    ),
    '#title' => t('forum'),
    '#disabled' => TRUE,
  );

    //Docs
    $form['docs'] = array(
        '#type' => 'checkbox',
        '#title' => t('docs'),
        '#disabled' => TRUE,
    );

    //Categories
  $form['categories'] = array(
    '#type' => 'select',
    '#options' => tow_apachesolr_facet_menu('im_vid_' . _tow_apachesolr_taxonomy_get_vid('Dataset categories'), tow_search_outer_filter_taxonomy()),
    '#title' => t('categories'),
  );

    if ($nopost && $value = $values['categories']) {
        $form['categories']['#value'] = $value;
    }

    //Tags
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
        '#value' => tow_search_outer_get_homepage_tab(),
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

        $vid1_block = apachesolr_search_taxonomy_facet_block($response, $query, 'im_vid_' . _tow_apachesolr_taxonomy_get_vid('Dataset tags'));
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
 * Composes an URL upon submission of form at all pages dataset search input block
 * URL is understandable by apachesolr and contains keyword parameter/
 */
function tow_search_outer_searchinput_form_submit($form, &$form_state) {

    $form_state['rebuild'] = TRUE;

    $fv = $form_state['values'];

    $path = tow_search_outer_get_homepage_tab() . '/' . $fv['keywords'];
    $query = 'filters=';

    if ($query == 'filters=')
        $query = NULL;

    drupal_goto('', $query, $path);
    //drupal_goto($path, $query);
}

/**
 * Composes an URL upon submission of form at homepage dataset search block
 * URL is understandable by apachesolr and contains all the search parameters/
 */
function tow_search_outer_filter_form_submit($form, &$form_state) {

    $form_state['rebuild'] = TRUE;

    $fv = $form_state['values'];

    $path = tow_search_outer_get_homepage_tab() . '/' . $fv['keywords'];
    $query = 'filters=';

    // Subjects.
    // Each consequent search option should be separated from previous by whitespace.
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

    if ($query == 'filters=')
        $query = NULL;

    drupal_goto($path, $query);
}

/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Return array of 'Dataset category' terms with keys being tid's.
 */
function tow_search_outer_filter_taxonomy() {

    $terms = array('any' => t('any'));

    $res = db_query("SELECT td.tid, td.name FROM {term_data} td JOIN {vocabulary} v ON td.vid = v.vid WHERE v.name = '%s' ORDER BY name ASC", 'Dataset categories');
    while ($term = db_fetch_array($res)) {
        $terms[$term['tid']] = $term['name'];
    }

    return $terms;
}

/**
 * Returns default tab for home page.
 */
function tow_search_outer_get_default_tab() {
    return 'newest';
}

/**
 * Returns values for outer search filters retrieved from $_GET array.
 */
function tow_search_outer_get_filters() {
    static $values;

    if (!isset($values)) {
        $filters = $_GET['filters'];
        $values = array();
        $values['query'] = '';

        // Size.
        if (preg_match('/(?<=sis_tow_amount_records:\[)\d{1,}/', $filters, $n)) {
            $values['size'] = $n[0];
            $values['query'] .= 'sis_tow_amount_records:[' . $n[0] . ' TO *] ';
        } else {
            $values['size'] = FALSE;
        }

        // Saved searches.
        if (preg_match('/(?<=sis_tow_amount_saved_searches:\[)\d{1,}/', $filters, $n)) {
            $values['saved_searches'] = $n[0];
            $values['query'] .= 'sis_tow_amount_saved_searches:[' . $n[0] . ' TO *] ';
        } else {
            $values['saved_searches'] = FALSE;
        }

        // Access status.
        if (preg_match('/(?<=sis_cck_field_access_type:)\d{1}/', $filters, $n)) {
            $values['access_status'] = $n[0];
            if ($values['access_status'] == 2) {
                $values['access_status'] = FALSE;
            }
            if ($values['access_status']) {
                $values['query'] .= 'sis_cck_field_access_type:' . $n[0] . ' ';
            }
        } else {
            $values['access_status'] = FALSE;
        }

        $matches_qty = preg_match_all('/(?<=tid:)\d{1,}/', $filters, $n);

        // Taxonomy.
        $values['categories'] = '';
        $values['tags_query'] = '';
        $values['tags'] = array();

        $outer_tids = array_flip(tow_search_outer_filter_taxonomy());
        foreach ($n[0] as $tid) {
            if (in_array($tid, $outer_tids)) {
                $values['categories'] = $tid;
                $values['query'] .= 'tid:' . $tid . ' ';
            } else {
                $values['tags_query'] .= ' ' . 'tid:' . $tid;
                $values['tags'][] = $tid;
                $values['query'] .= 'tid:' . $tid . ' ';
            }
        }
    }

    return $values;
}

/**
 * Returns tab for homepage: featured, newest, popular or debated.
 */
function tow_search_outer_get_homepage_tab() {
    static $tab;

    if (!isset($tab)) {

        // Extract keys as remainder of path
        // Note: support old GET format of searches for existing links.
        $path = explode('/', $_GET['q'], 2);
        $tab = $path[0];
        if (!in_array($tab, array('featured', 'newest', 'popular', 'debated', 'search_dataset', 'search_inner_zoom_ajax', 'user'))) {
            $tab = tow_search_outer_get_default_tab();
        }
    }

    return $tab;
}

/**
 * Modification of search_get_keys().
 */
function tow_search_outer_get_keys() {
    static $return;

    if (!isset($return)) {
        if (arg(0) == 'user') {
            return '';
        }

        // Extract keys as remainder of path
        // Note: support old GET format of searches for existing links.
        $path = explode('/', $_GET['q'], 2);
        $keys = empty($_REQUEST['keys']) ? '' : $_REQUEST['keys'];
        $return = count($path) == 2 ? $path[1] : $keys;
    }

    return $return;
}

/**
 * Returns link to homepage tab.
 */
function tow_search_outer_tab_link($current_tab, $title, $tab, $disable = FALSE) {
    if ($disable) {
        return $title;
    } elseif ($current_tab == $tab) {
        return '<b>' . $title . '</b>';
    } else {
        return l($title, $tab, array('attributes' => array('class' => 'tow-search-outer-tab')));
    }
}
