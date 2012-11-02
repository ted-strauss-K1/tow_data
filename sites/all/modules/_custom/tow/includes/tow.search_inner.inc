<?php

/**
 * @file
 *
 * Functions for inner search.
 */
/* * **************************************************************************************************
 * Hook handlers
 * ************************************************************************************************** */

/**
 * Part of hook_cron() implementation.
 * Clears old requests to solr from db table {tow_apachesolr_requests}.
 */
function tow_search_inner_cron() {
    $time = time() - (2 * 24 * 60 * 60); // 2 days before.
    $result = db_query("

    DELETE
      FROM
        {tow_apachesolr_requests}

      WHERE
        timestamp < %d

  ", $time);
}

/**
 * 'Inner search: field list' block.
 */
function tow_search_inner_field_list_block() {

    $selected = (isset($_GET['selected_fields'])) ? json_decode($_GET['selected_fields'], TRUE) : array();
    $selected = (is_array($selected)) ? $selected : array($selected);
    $nid = tow_search_inner_get_nid();

    if (arg(2) == 'refresh_ajax') {
        $show = TRUE;
        $nid_aux = explode('+', $nid);
        $node = node_load($nid_aux[0]);
        $dataset = ($node->type == 'dataset') ? $node : node_load($node->field_dataset[0]['value']);
    } elseif (is_numeric($nid)) {
        $dataset = node_load($nid);
        $show = ($dataset->type == 'dataset') ? TRUE : FALSE;
    } else {
        $show = FALSE;
    }

    $block['title'] = ($show) ? 'Fields' : '';
    $block['content'] = ($show) ? drupal_get_form('tow_search_inner_field_list_block_form', $dataset, $selected) : '';

    return $block;
}

/**
 * 'Inner search: simple search block' block.
 */
function tow_search_inner_simple_search_block() {

    $block = array();

    $block['title'] = 'Simple search';
    $block['content'] = drupal_get_form('tow_search_inner_simple_search_form');

    return $block;
}

/**
 * 'Inner search: facets block' block.
 */
function tow_search_inner_widgets_block() {

    // Get the current location (unset '/sort_ajax' as its the pointer to a specific ajax-call for the page).
    $path = $_GET['q'];

    // Define parametres of the sorting.
    $widgets_sort = isset($_GET['widgets_sort']) ? $_GET['widgets_sort'] : 'title';
    $sort = isset($_GET['sort']) ? $_GET['sort'] : 'asc';
    if (!in_array($widgets_sort, array('title', 'type', 'variability'))) {
        $widgets_sort = 'title';
    }
    if (!in_array($sort, array('asc', 'desc'))) {
        $sort = 'asc';
    }
    $sort_params = array(
        'widgets_sort' => $widgets_sort,
        'sort' => $sort
    );

    $zoom = json_decode($_GET['zoom'], TRUE);

    // Define the block.
    $block = array();
    $block['title'] = 'Filters';

    // Get info from ApacheSolr.
    $saved_response = apachesolr_static_response_cache(NULL, 'tow_inner_search');
    $saved_query = apachesolr_current_query(NULL, 'tow_inner_search');
    $query = clone $saved_query;
    $params = apachesolr_search_basic_params($query);
    $rows_amount = $saved_response->response->numFound;

    // Get table's Node ID.
    if (isset($saved_response->response->docs[0]))
        $table_nid = $saved_response->response->docs[0]->ss_tow_field_table;
    else {
        $table_nid = tow_search_inner_get_nid();
    }

    // Get stats for full range.
    foreach (tow_apachesolr_table_fields($table_nid, TRUE) as $index => $field) {
        $query->remove_filter($field['name']);
    }
    $query->set_keys('');

    // Get some more info from ApacheSolr.
    list($final_query, $stats_response) = apachesolr_do_query('tow_inner_search', $query, $params);
    $query = clone $saved_query;
    $params = apachesolr_search_basic_params($query);

    // Save query and stats to db.
    $serialized_query = serialize($query);
    $serialized_stats = serialize($stats_response->stats->stats_fields);
    $hash = mt_rand(1000, 9999);
    db_query("

    INSERT
      INTO
        {tow_apachesolr_requests}
          (tnid, query, stats, hash, timestamp)

      VALUES
        (%d, '%s', '%s', %d, %d)

  ", $table_nid, $serialized_query, $serialized_stats, $hash, time());
    $rid = db_last_insert_id('tow_apachesolr_requests', 'rid');

    // Set the block's content.
    $block['content'] = drupal_get_form('tow_search_inner_hash_form', $rid, $hash, $sort_params);

    // Building solr query for facets for each field.
    $table_fields = tow_apachesolr_table_fields($table_nid);
    foreach ($table_fields as $index => $field) {
        $stats = isset($stats_response->stats->stats_fields->$field['name']) ? $stats_response->stats->stats_fields->$field['name'] : NULL;
        $facet_counts = (array) $stats_response->facet_counts->facet_fields->$field['name'];
        $f = $field['name'] . '_highchart';
        $max = isset($zoom[$f]) ? $zoom[$f]['max'] : 0;
        $min = isset($zoom[$f]) ? $zoom[$f]['min'] : 0;
        tow_search_inner_facet_query($params, $field['name'], $field['type'], $stats, count($facet_counts) < 6, $max - $min);
    }

    // Request to solr using request built.
    $params['rows'] = 0;
    $params['facet'] = 'true';
    $params['facet.limit'] = 100;
    $params['facet.missing'] = 'true';
    $params['facet.range.include'][] = 'edge';
    $params['facet.range.include'][] = 'lower';
    $params['facet.range.hardend'] = 'true';

    list($final_query, $response) = apachesolr_do_query('tow_inner_search', $query, $params);
    if (empty($response)) {
        return;
    }

    apachesolr_static_response_cache($saved_response, 'tow_inner_search');
    apachesolr_current_query($saved_query, 'tow_inner_search');

    // Sort fields.
    if (isset($_GET['widgets_sort'])) {
        $table_fields = tow_search_inner_sort_fields($table_fields, $sort_params);
    }

    $selected_fields = (isset($_GET['selected_fields'])) ? json_decode($_GET['selected_fields'], TRUE) : array();

    // Creating facet for each field.
    foreach ($table_fields as $index => $field) {

        global $_tow_search_inner_field_delta;
        $_tow_search_inner_field_delta = $field['delta'];

        $field_filters = $query->get_filters('{!tag=' . $field['name'] . '}' . $field['name']);
        foreach ($field_filters as $field_filter) {
            $query->remove_filter('{!tag=' . $field['name'] . '}' . $field['name'], $field_filter['#value']);
            $query->add_filter($field['name'], $field_filter['#value']);
        }
        $newquery = clone $saved_query;
        $newquery->remove_filter($field['name']);
        $newquery->remove_filter('f_' . $field['name']);
        $queryvalues = $newquery->get_url_queryvalues();

        $querystring = tow_search_inner_compose_query($queryvalues['filters'], FALSE);
        unset($querystring['zoom']);
        $reset = l(t('Reset'), $path, array('query' => $querystring));
        $whole_counts = count((array) $stats_response->facet_counts->facet_fields->$field['name']) - 1; // exclude _empty_ field
        $widget = tow_search_inner_widget($field['name'], $field['type'], $response, $query, $stats_response->stats->stats_fields->$field['name'], $whole_counts, 'sis_tow_field_' . $field['type'] . '_' . $field['delta']);

        if (!empty($selected_fields)) {
            if (in_array($field['title'] . '_' . $field['type'], $selected_fields)) {
                $block['content'] .= drupal_get_form('tow_search_inner_widget_form', $field['title'], $widget, $reset);
            }
        } else {
            $block['content'] .= drupal_get_form('tow_search_inner_widget_form', $field['title'], $widget, $reset);
        }
    }
    //$block['content'] .= drupal_get_form('tow_saved_searches_save_search_form', $rows_amount);

    return $block;
}

/**
 * Page for inner search within dataset menu callback.
 */
function tow_search_inner_within_dataset_callback($nid) {

    global $_tow_dataset;
    $_tow_dataset = node_load($nid);
    
    //Don't allow page loading 'search_dataset/nid' if node type isn't dataset
    if ($_tow_dataset->type != 'dataset') {
      drupal_not_found();
      die();
    }
    
    global $_tow_dataset_search;
    $_tow_dataset_search = TRUE;

    // Check for redirect.
    if (!is_numeric($nid)) {

        // Get current NIDs order and a sorted one.
        $nid_array = explode(' ', $nid);
        $sort_nid = explode(' ', $nid);
        sort($sort_nid);

        // Check sorted.
        $sorted = TRUE;
        foreach ($nid_array as $k => $v) {
            if ($nid_array[$k] != $sort_nid[$k]) {
                $sorted = FALSE;
                break;
            }
        }

        if (!$sorted) {

            // Build new NID.
            $sort_nid = implode('+', $sort_nid);
            $path = str_replace($nid, $sort_nid, $_GET['q']);
            drupal_goto($path);
        }
    }

    $type = 'apachesolr_search';
    $keys = trim(_tow_search_inner_get_keys());

    // We did not redirect, so run the search if needed.
    $results = '';

    $filters = trim($_GET['filters']);

    // Log the search keys:
    $log = $keys;
    if ($filters) {
        $log .= 'filters=' . $filters;
    }

    // Collect the search results:
    $results = search_data($keys, 'tow_inner_search');

    // Add javascript.
    drupal_add_js(drupal_get_path('module', 'tow') . '/js/jquery.ba-hashchange.min.js');
    drupal_add_js(drupal_get_path('module', 'tow') . '/js/inner_search.js');
    drupal_add_js(drupal_get_path('module', 'tow') . '/js/highcharts.js');
    drupal_add_js(drupal_get_path('module', 'datatables') . '/dataTables/extras/Scroller/media/js/dataTables.scroller.js');
    drupal_add_js(drupal_get_path('module', 'datatables') . '/dataTables/extras/ColVis/media/js/ColVis.js');

    // Return results.
    if ($results) {
        return $results;
    } else {
        return theme('box', t('Your search yielded no results'), variable_get('apachesolr_search_noresults', apachesolr_search_noresults()));
    }
}

/* * **************************************************************************************************
 * Forms
 * ************************************************************************************************** */

/**
 * Field selection block form.
 */
function tow_search_inner_field_list_block_form($form_state, $dataset, $selected_fields = array()) {
    $form = array();
    $nid = array();
    $selected_tableset = array();

    // Define selected tableset and nid of the selected table.
    $fields = array();
    list($tables, $tablesets) = tow_search_inner_linked_build_tablesets($dataset->nid);
    $linking_fields = array();
    foreach ($tablesets as $tableset) {
        $tableset_nids = array_keys($tableset);
        sort($tableset_nids);
        $key = implode('+', $tableset_nids);
        $linking_fields[$key] = _tow_search_inner_linking_fields($tableset_nids);
    }
    foreach ($tables as $table_nid) {
        $table_fields = tow_apachesolr_table_fields($table_nid);
        foreach ($table_fields as $field) {
            if (in_array($field['title'] . '_' . $field['type'], $selected_fields)) {
                if (empty($selected_tableset)) {
                    foreach ($tablesets as $tableset) {
                        foreach ($tableset as $key => $value) {
                            if ($key == $table_nid) {
                                $selected_tableset = array_keys($tableset);
                                break 2;
                            }
                        }
                    }
                    if (empty($selected_tableset)) {
                        $selected_tableset = array($table_nid);
                    }
                }
                $nid[] = $table_nid;
                break;
            }
        }
    }

    // Links.
    foreach ($tables as $table_nid) {
        $table_fields = tow_apachesolr_table_fields($table_nid);

        $key = tow_search_inner_linked_build_table_tablesets($table_nid);
        if (isset($key)) {
            $key = array_keys($key);
            sort($key);
            $key = implode('+', $key);
        }

        foreach ($table_fields as $field) {

            $is_linking_field = FALSE;
            if (isset($key)) {
                foreach ($linking_fields[$key] as $l_field) {
                    if (($l_field['title'] == $field['title']) && ($l_field['type'] == $field['type'])) {
                        $is_linking_field = TRUE;
                        break;
                    }
                }
            }

            // Future NID.
            $nid_aux = ($is_linking_field) ? explode('+', $key) : $nid;
            $nid_aux[] = $table_nid;
            $nid_aux = array_unique($nid_aux);
            sort($nid_aux);
            $nid_aux = implode('+', $nid_aux);

            // Attributes.
            $options = array(
                'attributes' => array(
                    'class' => 'tow-dataset-field-link',
                    'f_type' => $field['type'],
                ),
            );
            if (in_array($field['title'] . '_' . $field['type'], $selected_fields)) {
                $options['attributes']['class'] .= ' selected';
            }

            // Fields as links or plain text according to selectedness and availability.
            if (empty($selected_tableset) || in_array($table_nid, $selected_tableset)) {
                $fields[$table_nid][] = '<button class="' . $options['attributes']['class'] . ' btn btn-mini" href="/search_dataset/' . $nid_aux . '/refresh_ajax" f_type="' . $options['attributes']['f_type'] . '">' . $field['title'] . '</button>'; //l($field['title'], "search_dataset/$nid_aux/refresh_ajax", $options);
            } else {
                $fields[$table_nid][] = '<button class="tow-dataset-field-link btn btn-mini disabled">' . $field['title'] . '</button>';
            }
        }
    }

    // Form elements.
    foreach ($fields as $table_nid => $table_fields) {
        $table = node_load($table_nid);
        $form["tow_table_fields_$table_nid"] = array(
            '#type' => 'fieldset',
            '#title' => filter_xss($table->title),
            '#collapsible' => TRUE,
            '#collapsed' => (!(empty($selected_tableset) || in_array($table_nid, $selected_tableset))) ? TRUE : FALSE,
        );
        /* $fields_list = '<ul>';
          foreach ($table_fields as $f) {
          $fields_list .= '<li>' . $f . '</li>';
          }
          $fields_list .= '</ul>'; */
        $fields_list = '<div class="btn-group" data-toggle="buttons-checkbox">';
        foreach ($table_fields as $f) {
            $fields_list .= $f;
        }
        $fields_list .= '</div>';
        $form["tow_table_fields_$table_nid"]['fields_list'] = array(
            '#value' => $fields_list,
        );
    }

    return $form;
}

/**
 * Form at seach inner page containing hidden values for rid and hash
 * for chart updating while zooming.
 * Sort links are also included here.
 */
function tow_search_inner_hash_form($form_state, $rid, $hash, $sort_params) {
    $form = array();

    // Get path.
    $path = $_GET['q'];
    $form['#action'] = $path;

    // Get sort parameters.
    extract($sort_params);

    // RID.
    $form['rid'] = array(
        '#type' => 'hidden',
        '#value' => $rid,
    );

    // Hash.
    $form['hash'] = array(
        '#type' => 'hidden',
        '#value' => $hash,
    );

    // Define links' settings.
    $tow_widgets_sort_type = array('title' => 'asc', 'type' => 'asc', 'variability' => 'asc');
    $tow_widgets_sort_type[$widgets_sort] = ($sort == 'asc') ? 'desc' : 'asc';

    // Make links.
    $links = array('title', 'type', 'variability');
    foreach ($links as $link_name) {
        $params = array(
            'attributes' => array(
                'class' => 'sort-link',
            ),
            'query' => array(
                'widgets_sort' => $link_name,
                'sort' => $tow_widgets_sort_type[$link_name],
            ),
        );
        ${$link_name} = l(t($link_name), $path, $params);
    }
    $reset_all = l(t('Reset all'), $path, array('attributes' => array('class' => 'sort-link-reset')));

    // Sort block.
    $form['sort'] = array(
        '#type' => 'markup',
        '#prefix' => '<div class="tow-inner-search-widget-sort">',
        '#value' => 'Sort by ' . $title . ' | ' . $type . ' | ' . $variability . '<br>' . $reset_all,
        '#suffix' => '</div>',
    );

    return $form;
}

/**
 * Returns form for numeric facets.
 */
function tow_search_inner_numeric_facet_form($form_state, $field, $min, $max, $empty, $selection_min, $selection_max, $data = NULL, $tooltips = NULL, $fieldtype = 'int', $max_count = 0) {
    $form = array();

    $saved_query = apachesolr_current_query(NULL, 'tow_inner_search');
    $query = clone $saved_query;

    // Handling empty fields.
    $empty_query = clone $saved_query;

    $empty_included = FALSE;
    $no_other_filters = TRUE;
    foreach ($empty_query->get_filters($field) as $filter) {
        if ($filter['#value'] == '[* TO *]') {
            $empty_included = TRUE;
        } else {
            $value = $filter['#value'];
            $no_other_filters = FALSE;
        }
    }

    if ($no_other_filters) {
        $empty_included = !$empty_included;
    }

    if ($no_other_filters && $empty_included) {

        // 1-1: default state.
        // Link for button submit.
        $query->add_filter($field, '[* TO *]', TRUE);
        $query->add_filter($field, $field . '_placeholder');

        // Link for click on checkbox.
        $empty_query->add_filter($field, '[* TO *]');
    } elseif (!$no_other_filters && $empty_included) {

        // 0-1.
        // Link for button submit.
        $query->remove_filter($field, $value);
        $query->add_filter($field, $field . '_placeholder');

        // Link for click on checkbox.
        $empty_query->remove_filter($field, '[* TO *]');
    } elseif ($no_other_filters && !$empty_included) {

        // 1-0.
        // Link for button submit.
        $query->remove_filter($field, '[* TO *]');
        $query->add_filter($field, $field . '_placeholder');

        // Link for click on checkbox.
        $empty_query->remove_filter($field, '[* TO *]');
        $empty_query->remove_filter($field, $value);
    } else {

        // 0-0.
        // Link for button submit.
        $query->remove_filter($field, $value);
        $query->add_filter($field, $field . '_placeholder');

        // Link for click on checkbox.
        $empty_query->add_filter($field, '[* TO *]', TRUE);
    }

    $filters = $query->get_url_queryvalues();
    $querystring = tow_search_inner_compose_query($filters['filters'], FALSE);
    $url = url($_GET['q'], array('query' => $querystring));

    $filters = $empty_query->get_url_queryvalues();
    $querystring = tow_search_inner_compose_query($filters['filters'], FALSE);
    $include_empty_url = url($_GET['q'], array('query' => $querystring));

    // Form.
    $form['data'] = array(
        '#type' => 'hidden',
        '#value' => $data,
    );

    $form['tooltips'] = array(
        '#type' => 'hidden',
        '#value' => $tooltips,
    );

    $form['max_count'] = array(
        '#type' => 'hidden',
        '#value' => $max_count,
    );

    $form['url'] = array(
        '#type' => 'hidden',
        '#value' => $url,
    );

    $form['field'] = array(
        '#type' => 'hidden',
        '#value' => $field,
    );

    $form['fieldtype'] = array(
        '#type' => 'hidden',
        '#value' => $fieldtype,
    );

    $form['include_empty_url'] = array(
        '#type' => 'hidden',
        '#value' => $include_empty_url,
    );

    $filters = $_GET['filters'] ? 'filters=' . $_GET['filters'] : '';

    $form['include_empty'] = array(
        '#type' => 'checkbox',
        '#title' => t('Empty') . ' (' . $empty . ')',
        '#value' => $empty_included,
    );

    $form['global_min'] = array(
        '#type' => 'hidden',
        '#value' => $min,
    );

    $form['global_max'] = array(
        '#type' => 'hidden',
        '#value' => $max,
    );

    $form['selection_min'] = array(
        '#type' => 'hidden',
        '#value' => ($selection_min === FALSE) ? $min : $selection_min,
    );

    $form['selection_max'] = array(
        '#type' => 'hidden',
        '#value' => ($selection_max === FALSE) ? $max : $selection_max,
    );

    $form['min'] = array(
        '#type' => 'hidden',
        '#value' => $min,
    );

    $form['max'] = array(
        '#type' => 'hidden',
        '#value' => $max,
    );

    $form['search'] = array(
        '#type' => 'button',
        '#title' => t('Search'),
        '#value' => t('Search'),
        '#attributes' => array(
            'class' => 'search-button',
            'onclick' => 'return false;',
        ),
    );

    return $form;
}

/**
 * 'Inner search: simple search block' form.
 */
function tow_search_inner_simple_search_form() {
    $form = array();

    // NID.
    $form['nid'] = array(
        '#type' => 'hidden',
        '#value' => tow_search_inner_get_nid(),
    );

    // Keywords.
    $form['keywords'] = array(
        '#type' => 'textfield',
        '#value' => _tow_search_inner_get_keys(),
    );

    return $form;
}

/**
 * Form for text length.
 */
function tow_search_inner_text_length_widget_form($form_state, $length_field) {
    $form = array();

    // Field info.
    $form['fieldtype'] = array(
        '#type' => 'hidden',
        '#value' => 'length',
    );
    $form['field'] = array(
        '#type' => 'hidden',
        '#value' => $length_field,
    );

    // Search by # of words/characters.
    $saved_query = apachesolr_current_query(NULL, 'tow_inner_search');
    $query = clone $saved_query;
    $placeholder = $length_field . '_placeholder';

    $prefix = '_words';
    $active_filters = $query->get_filters($length_field . $prefix);
    if (!$active_filters) {
        $prefix = '_length';
        $active_filters = $query->get_filters($length_field . $prefix);
    }
    if ($active_filters) {
        if (is_numeric($active_filters[0]['#value'])) {

            // =
            $value = $active_filters[0]['#value'];
            $option = $prefix . ':#';
        } elseif ($active_filters[0]['#start'] != '*') {

            // >
            $value = $active_filters[0]['#start'] - 1;
            $option = $prefix . ':[# TO *]';
        } else {

            // <
            $value = $active_filters[0]['#end'] + 1;
            $option = $prefix . ':[* TO #]';
        }
    }

    // Positive search (string includes substring).
    $query->remove_filter($length_field . '_words');
    $query->remove_filter($length_field . '_length');

    $query->add_filter($length_field, $placeholder);
    $filters = $query->get_url_queryvalues();
    $querystring = tow_search_inner_compose_query($filters['filters'], FALSE);
    $url = url($_GET['q'], array('query' => $querystring));

    $form['url'] = array(
        '#type' => 'hidden',
        '#value' => $url,
    );

    // Input fields for search by # of words/characters.
    $form['option'] = array(
        '#type' => 'select',
        '#options' => array(
            '_length:#' => '# characters =',
            '_length:[* TO #]' => '# characters <',
            '_length:[# TO *]' => '# characters >',
            '_words:#' => '# words =',
            '_words:[* TO #]' => '# words <',
            '_words:[# TO *]' => '# words >',
        ),
    );

    if (isset($option)) {
        $form['option']['#value'] = $option;
    }

    $form['value'] = array(
        '#type' => 'textfield',
    );
    if (isset($value)) {
        $form['value']['#value'] = $value;
    }

    // Search button.
    $form['search'] = array(
        '#type' => 'button',
        '#title' => t('Search'),
        '#value' => t('Search'),
        '#attributes' => array(
            'class' => 'search-button',
            'onclick' => 'return false;'
        ),
    );

    return $form;
}

/**
 * Widget form description.
 */
function tow_search_inner_widget_form(&$form_state, $field_title, $widget, $reset) {
    $form = array();

    // Get current location.
    $path = $_GET['q'];
    $path = str_replace('/refresh_ajax', '', $path);

    // Wrap the form with div-wrapper.
    $form['#prefix'] = '<div class="tow-inner-search-widget">';
    $form['#suffix'] = '</div>';
    $form['#action'] = $path;

    // Wrap the controls with fieldset.
    $form['tow_inner_search_widget'] = array(
        '#type' => 'fieldset',
        '#title' => check_plain($field_title),
        '#collapsible' => TRUE,
        '#collapsed' => _tow_search_inner_widget_fieldset_collapsed(),
        '#attributes' => array(
            'class' => $field_title,
        ),
        // Widgets itself.
        'widget' => array(
            '#type' => 'markup',
            '#value' => $widget,
        ),
        // Rest link.
        'reset' => array(
            '#type' => 'markup',
            '#value' => $reset,
        ),
    );

    return $form;
}

/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Returns bar chart widget.
 */
function tow_search_inner_bar_widget($field, $field_type, $facet_counts, $empty, $stats) {
    $data = array();
    $function = '_tow_solr_to_' . $field_type;
    $function_exists = function_exists($function);
    foreach ($facet_counts as $value => $count) {
        if ($count) {
            if ($function_exists) {
                $function($value);
            }
            $facets_list[] = $value;
            $data[] = $count;
        }
    }

    // Limits of selection range.
    $selection_limits = tow_search_inner_range_limits($field);
    if ($function_exists) {
        if ($selection_limits['min']) {
            $function($selection_limits['min']);
        }
        if ($selection_limits['max']) {
            $function($selection_limits['max']);
        }
        $function($stats->min);
        $function($stats->max);
    }

    $data = drupal_to_js($data);
    $facets_list = drupal_to_js($facets_list);

    $form = drupal_get_form('tow_search_inner_numeric_facet_form', $field, $stats->min, $stats->max, $empty, $selection_limits['min'], $selection_limits['max'], $data, $facets_list, $field_type);
    $container = '<div class="tow-inner-search-highcharts-bar-container" id="' . $field . '_highchart"></div>';
    return $container . $form;
}

/**
 * Returns array of data for chart (points and tooltips).
 */
function tow_search_inner_chart_data($field, $field_type, $facet_counts) {

    $facets_list = array();
    $gap = _tow_search_inner_solr_gap_to_js($field_type, $facet_counts->gap);
    $start = _tow_search_inner_solr_to_js($field_type, $facet_counts->start);
    $end = _tow_search_inner_solr_to_js($field_type, $facet_counts->end);
    $is_natural_gap = ($gap == _tow_search_inner_natural_gap($field_type, TRUE));

    $group_start = $start;
    $counts = $facet_counts->counts;
    $data = array();
    $sum = 0;
    $max_count = 0;
    if ($field_type == 'float') {
        $counts = (array) $counts;

        $count_in_bin = array();
        foreach ($counts as $group_start => $count) {
            $number_of_bin = round(($group_start - $start) / $gap);
            $count_in_bin[$number_of_bin] = $count;
        }

        $data = array();
        $ii = 0;
        $group_start = $start;
        while ($group_start + $gap / 2 < $end) {
            $group_end = $start + ($ii + 1) * $gap;
            if (isset($count_in_bin[$ii])) {
                $group_counts = $count_in_bin[$ii];
                if ($group_counts > $max_count) {
                    $max_count = $group_counts;
                }
            } else {
                $group_counts = 0;
            }

            $facets_list[] = _tow_search_inner_range($group_start, $group_end, $is_natural_gap) . _tow_search_inner_number_of_results($group_counts);
            $data[] = array(($group_start + $group_end) / 2, $group_counts);
            $sum += $group_counts;
            $ii++;
            $group_start = $group_end;
        }
        if (isset($count_in_bin[$ii])) {
            $group_counts += $count_in_bin[$ii];
        }
    } else {
        while ($group_start < $end) {
            $interval_start = _tow_search_inner_js_to_solr($field_type, $group_start);
            $interval_end = _tow_search_inner_js_to_solr($field_type, $group_start + $gap - 1);
            $function = '_tow_solr_to_' . $field_type;
            if (function_exists($function)) {

                // Turn date and time to displayed view.
                $function($interval_start);
                $function($interval_end);
            }

            // Turn group start to solr format.
            $solr_group_start = _tow_search_inner_js_to_solr($field_type, $group_start);

            if (isset($counts->$solr_group_start)) {
                $group_counts = $counts->$solr_group_start;
                if ($group_counts > $max_count) {
                    $max_count = $group_counts;
                }
            } else {
                $group_counts = 0;
            }

            $facets_list[] = _tow_search_inner_range($interval_start, $interval_end, $is_natural_gap) . _tow_search_inner_number_of_results($group_counts);
            if ($is_natural_gap) {
                $data[] = array($group_start, $group_counts);
            } else {
                $data[] = array($group_start + ($gap - 1) / 2, $group_counts);
            }
            $sum += $group_counts;
            $group_start += $gap;
        }
        if (isset($facet_counts->after)) {
            $group_counts += $facet_counts->after;
        }
    }

    // Include end value to last point.
    array_pop($facets_list);
    array_pop($data);
    $interval_start = _tow_search_inner_js_to_solr($field_type, $group_start - $gap);
    $interval_end = _tow_search_inner_js_to_solr($field_type, $end);
    $function = '_tow_solr_to_' . $field_type;
    if (function_exists($function)) {

        // Turn date and time to displayed view.
        $function($interval_start);
        $function($interval_end);
    }
    array_push($facets_list, _tow_search_inner_range($interval_start, $interval_end, $is_natural_gap) . _tow_search_inner_number_of_results($group_counts));
    if ($is_natural_gap) {
        array_push($data, array($group_start - $gap, $group_counts));
        $end = $group_start - $gap;
    } else {
        array_push($data, array(($group_start - $gap + $end) / 2, $group_counts));
    }

    return array($data, $facets_list, $start, $end, $max_count);
}

/**
 * Returns line chart widget.
 */
function tow_search_inner_chart_widget($field, $field_type, $facet_counts, $empty) {

    // Prepare data.
    list($data, $facets_list, $start, $end, $max_count) = tow_search_inner_chart_data($field, $field_type, $facet_counts);
    $data = drupal_to_js($data);
    $facets_list = drupal_to_js($facets_list);

    // Limits of selection range.
    $selection_limits = tow_search_inner_range_limits($field);
    if ($selection_limits['min']) {
        $selection_limits['min'] = _tow_search_inner_solr_to_js($field_type, $selection_limits['min']);
    }
    if ($selection_limits['max']) {
        $selection_limits['max'] = _tow_search_inner_solr_to_js($field_type, $selection_limits['max']);
    }

    // Generate output.
    $form = drupal_get_form('tow_search_inner_numeric_facet_form', $field, $start, $end, $empty, $selection_limits['min'], $selection_limits['max'], $data, $facets_list, $field_type, $max_count);
    $container = '<div class="tow-inner-search-highcharts-container" id="' . $field . '_highchart"></div>';
    return $container . $form;
}

/**
 * Builds GET query as ('filters' => filters, 'zoom' => zoom),
 * omitting param with empty value and getting value from GET
 * if value is FALSE.
 */
function tow_search_inner_compose_query($filters = FALSE, $zoom = FALSE) {
    $query = array();
    _tow_search_inner_add_queryvalue($query, 'filters', $filters);
    _tow_search_inner_add_queryvalue($query, 'zoom', $zoom);
    return $query;
}

/**
 * Builds part of solr query for getting facet numbers for table field.
 */
function tow_search_inner_facet_query(&$params, $field, $type, $stats, $barchart, $visible_range = 0) {

    // Define range.
    if (!$barchart) {
        switch ($type) {

            // Integer.
            case 'int':
                $min = $stats->min;
                $max = $stats->max;
                $gap = _tow_search_inner_facet_gap($max - $min, $visible_range, _tow_search_inner_natural_gap($type));
                _tow_search_inner_add_facet_range(&$params, $field, $min, $max, $gap, $type);
                break;

            // Float.
            case 'float':
                $min = $stats->min;
                $max = $stats->max;
                $gap = _tow_search_inner_facet_gap($max - $min, $visible_range);
                $params['f.' . $field . '.facet.range.other'][] = 'after';
                _tow_search_inner_add_facet_range(&$params, $field, $min, $max, $gap, $type);
                break;

            // Date.
            case 'date':
                if ($stats) {
                    $min = $stats->min;
                    $max = $stats->max;
                    $timestamp_min = strtotime($min);
                    $timestamp_max = strtotime($max);
                    $gap = _tow_search_inner_facet_gap($timestamp_max - $timestamp_min, $visible_range / 1000, _tow_search_inner_natural_gap($type));
                    $gap = '+' . $gap . 'SECOND';
                    _tow_search_inner_add_facet_range(&$params, $field, $min, $max, $gap, $type);
                }
                break;

            // Datetime.
            case 'datetime':
                if ($stats) {
                    $min = $stats->min;
                    $max = $stats->max;
                    $timestamp_min = strtotime($min);
                    $timestamp_max = strtotime($max);
                    $gap = _tow_search_inner_facet_gap($timestamp_max - $timestamp_min, $visible_range / 1000, _tow_search_inner_natural_gap($type));
                    _tow_search_inner_add_facet_range(&$params, $field, $min, $max, '+' . $gap . 'SECOND', $type);
                }
                break;

            // Time.
            case 'time':
                $gap = _tow_search_inner_facet_gap(24 * 60 * 60, $visible_range / 1000, _tow_search_inner_natural_gap($type), 24);
                _tow_search_inner_add_facet_range(&$params, $field, '0001-01-01T00:00:00Z', '0001-01-01T23:59:59Z', '+' . $gap . 'SECOND', $type);
                break;

            // Timestamp.
            case 'timestamp':
                $min = $stats->min;
                $max = $stats->max;
                $gap = _tow_search_inner_facet_gap($max - $min, $visible_range / 1000, _tow_search_inner_natural_gap($type));
                _tow_search_inner_add_facet_range(&$params, $field, $min, $max, $gap, $type);
                break;

            // Boolean and enums.
            case 'bool':
            case 'enum':
                $params['facet.field'][] = '{!ex=' . $field . '}' . $field; //
                break;

            // Text.
            case 'text':
                $field = 'f_' . $field;
                $params['facet.field'][] = '{!ex=' . $field . '}' . $field;
                break;

            // Char and code.
            case 'char':
            case 'code':
            default:
                $params['facet.field'][] = '{!ex=' . $field . '}' . $field;
                break;
        }
    } else {
        $params['facet.field'][] = '{!ex=' . $field . '}' . $field;
        switch ($type) {

            case 'int':
            case 'float':
            case 'date':
            case 'datetime':
            case 'time':
            case 'timestamp':
                $params['f.' . $field . '.facet.sort'] = 'index';
                break;

            case 'bool':
            case 'enum':
            case 'char':
            case 'code':
            case 'text':
            default:
                break;
        }
    }
}

/**
 * Function for displaying a facet block.
 */
function tow_search_inner_facet_widget($response, $query, $module, $delta, $facet_field, $filter_by, $facet_callback = FALSE) {
    if (!empty($response->facet_counts->facet_fields->$facet_field)) {
        $contains_active = FALSE;
        $items = array();

        foreach ($response->facet_counts->facet_fields->$facet_field as $facet => $count) {
            $options = array();
            $exclude = FALSE;

            // Solr sends this back if it's empty.
            if ($facet == '_empty_') {
                $exclude = TRUE;
                $facet = '[* TO *]';
                $options['html'] = TRUE;
            }
            $options['html'] = TRUE;

            if ($facet_callback && function_exists($facet_callback)) {
                $facet_text = $facet_callback($facet, $options);
            } elseif ($exclude) {
                $facet_text = theme('placeholder', t('<empty>'));
            } else {
                $facet_text = $facet;
            }

            $active = $query->has_filter($facet_field, $facet);

            if ($active) {

                // '*' sorts before all numbers.
                $sortpre = '*';
            } elseif ($exclude) {

                // '-' sorts before all numbers, but after '*'.
                $sortpre = '-';
            } else {
                $sortpre = 1000000 - $count;
            }

            $new_query = clone $query;
            if ($active) {
                $contains_active = TRUE;
                $new_query->remove_filter($facet_field, $facet);
                $options['query'] = $new_query->get_url_queryvalues();
                $link = theme('tow_search_inner_unclick_link', $facet_text, $new_query->get_path(), $options, $count, FALSE, $response->response->numFound);
            } else {
                $new_query->add_filter($facet_field, $facet, $exclude);
                $options['query'] = $new_query->get_url_queryvalues();
                $link = theme('tow_search_inner_facet_link', $facet_text, $new_query->get_path(), $options, $count, FALSE, $response->response->numFound);
            }

            if ($count || $active) {
                $items[$sortpre . '*' . $facet_text] = $link;
            }
        }

        // Unless a facet is active only display 2 or more.
        if ($items && ($response->response->numFound >= 0 || $contains_active)) {
            ksort($items, SORT_STRING);

            // Get information needed by the rest of the blocks about limits.
            $initial_limits = variable_get('apachesolr_facet_query_initial_limits', array());
            $limit = isset($initial_limits[$module][$delta]) ? $initial_limits[$module][$delta] : variable_get('apachesolr_facet_query_initial_limit_default', 10);
            $output = theme('apachesolr_facet_list', $items, $limit);
            return array('subject' => $filter_by, 'content' => $output);
        }
    }
    return array('content' => t('No results found'));
}

/**
 * Returns graphical widget.
 */
function tow_search_inner_graph_widget($field, $type, $facet_counts, $empty, $stats) {
    return tow_search_inner_chart_widget($field, $type, $facet_counts->facet_ranges->$field, $empty);
}

/**
 * Retrieves table nid from GET[q] search_dataset/nid -> nid
 */
function tow_search_inner_get_nid() {
    $path = explode('/', $_GET['q']);
    $return = count($path) >= 2 ? $path[1] : FALSE;
    $return = str_replace(' ', '+', $return);
    return $return;
}

/**
 * Get range limits.
 */
function tow_search_inner_range_limits($current_field) {

    $query = apachesolr_current_query(NULL, 'tow_inner_search');
    $filters = $query->get_filters($current_field);

    foreach ($filters as $filter) {
        if (!$filter['#exclude'] && ($filter['#start'] != '*') && ($filter['#end'] != '*')) {
            $limits = array(
                'min' => $filter['#start'],
                'max' => $filter['#end']
            );
            return $limits;
        }
    }

    $limits = array(
        'min' => FALSE,
        'max' => FALSE
    );

    return $limits;
}

/**
 * Sort fields according to the parameters selected.
 */
function tow_search_inner_sort_fields($fields, $sort_params) {

    // Auxiliary arrays.
    $aux_fields = array();
    $index_fields = array();
    $sorted_fields = array();

    // Sort parameters.
    extract($sort_params);

    // Sort according sort type.
    switch ($widgets_sort) {

        case 'title':

            // Prepare sort by widget names.
            foreach ($fields as $index => $field) {
                $aux_fields[$field['name']] = $field['title'];
                $index_fields[$field['name']] = $index;
            }
            break;

        // Prepare sort by widget field type.
        case 'type':
            foreach ($fields as $index => $field) {
                $aux_fields[$field['name']] = $field['type'];
                $index_fields[$field['name']] = $index;
            }

            break;

        // Sort by variability.
        case 'variability':
            # TODO:
            break;
    }

    // Sorting.
    if ($sort == 'asc') {
        asort($aux_fields);
    } else {
        arsort($aux_fields);
    }

    // Prepare results.
    foreach ($aux_fields as $field_name => $field_title) {
        $sorted_fields[] = $fields[$index_fields[$field_name]];
    }

    return $sorted_fields;
}

/**
 * Returns form for textual widgets.
 */
function tow_search_inner_text_widget_form($form_state, $field, $fieldtype) {

    $form = array();

    // Field info.
    $form['field'] = array(
        '#type' => 'hidden',
        '#value' => $field,
    );
    $form['fieldtype'] = array(
        '#type' => 'hidden',
        '#value' => $fieldtype,
    );
    $form['length_field'] = array(
        '#type' => 'hidden',
        '#value' => $length_field,
    );


    // Search by substring.
    $saved_query = apachesolr_current_query(NULL, 'tow_inner_search');
    $query = clone $saved_query;
    $active_filters = $query->get_filters($field);
    if ($active_filters) {
        $option = preg_replace('/[^\*]++/', '_', $active_filters[0]['#value']);
        $value = preg_replace('/\*/', '', $active_filters[0]['#value']);
        if ($active_filters[0]['#exclude']) {
            $option = '-' . $option;
        }
        $value = preg_replace('/(?<!\\\\)\+/', ' ', $value);
        $value = preg_replace('/\\\\(\+|-|&&|\|\||!|\(|\)|\{|\}|\[|\]|\^|"|~|\*|\?|:|\\\\)/', '$1', $value);
    }

    // Positive search (string includes substring).
    $query->remove_filter($field);
    $placeholder = $field . '_placeholder';
    $query->add_filter($field, $field . '_placeholder');
    $filters = $query->get_url_queryvalues();
    $querystring = tow_search_inner_compose_query($filters['filters'], FALSE);
    $url = url($_GET['q'], array('query' => $querystring));

    $form['url'] = array(
        '#type' => 'hidden',
        '#value' => $url,
    );

    // Negative search (string doesn't include substring).
    $query->remove_filter($field, $field . '_placeholder');
    $query->add_filter($field, $field . '_placeholder', TRUE);
    $filters = $query->get_url_queryvalues();
    $querystring = tow_search_inner_compose_query($filters['filters'], FALSE);
    $negative_url = url($_GET['q'], array('query' => $querystring));

    $form['negative_url'] = array(
        '#type' => 'hidden',
        '#value' => $negative_url,
    );

    // Input fields for search by substring.
    $form['option'] = array(
        '#type' => 'select',
        '#options' => array(
            '_*' => 'starts with',
            '*_' => 'ends with',
            '*_*' => 'contains',
            '-*_*' => 'not contains',
            '_' => 'equals',
            '-_' => 'not equals',
        ),
    );

    if (isset($option)) {
        $form['option']['#value'] = $option;
    }

    $form['value'] = array(
        '#type' => 'textfield',
    );

    if (isset($value)) {
        $form['value']['#value'] = $value;
    }

    // Search button.
    $form['search'] = array(
        '#type' => 'button',
        '#title' => t('Search'),
        '#value' => t('Search'),
        '#attributes' => array(
            'class' => 'search-button',
            'onclick' => 'return false;'
        ),
    );

    return $form;
}

/**
 * Retrieves zooming factor from GET[zoom].
 */
function tow_search_inner_timezoom($current_field) {
    $zoom_string = $_GET['zoom'];
    $zoom_array = explode(' ', $zoom_string);
    $current_zoom = 0;
    foreach ($zoom_array as $fieldzoom) {
        list($field, $zoom) = explode(':', $fieldzoom);
        if ($field == $current_field) {
            $current_zoom = $zoom;
            break;
        }
    }
    $num_of_points = _tow_search_inner_timezoom_points($current_zoom);
    return $num_of_points ? $num_of_points : _tow_search_inner_zoom_points('0');
}

/**
 * Builds part of inner search facets block for specific field.
 */
function tow_search_inner_widget($field, $type, $response, $query, $stats, $whole_counts, $length_field = NULL) {
    $empty = $response->facet_counts->facet_fields->$field->_empty_;

    switch ($type) {

        // Charts.
        case 'int':
        case 'float':
        case 'date':
        case 'datetime':
        case 'time':
        case 'timestamp':
            if (isset($response->facet_counts->facet_ranges->$field)) {
                return tow_search_inner_graph_widget($field, $type, $response->facet_counts, $empty, $stats);
            } else {
                $facets_list = tow_search_inner_facet_widget($response, $query, 'tow_inner_search', $field, $field, '');
                return $facets_list['content'];
            }

        // Bool and code facets.
        case 'bool':
        case 'code':
            $facets_list = tow_search_inner_facet_widget($response, $query, 'tow_inner_search', $field, $field, '');
            $facets_list['content'] .= '<input name="field" id="edit-field" value="' . $field . '" type="hidden">';
            $facets_list['content'] .= '<input name="fieldtype" id="edit-fieldtype" value="' . $type . '" type="hidden">';
            return $facets_list['content'];

        // Enum facets.
        case 'enum':
            $facets_list = tow_search_inner_facet_widget($response, $query, 'tow_inner_search', $field, $field, '', '_tow_search_inner_facet_enum_option');
            $facets_list['content'] .= '<input name="field" id="edit-field" value="' . $field . '" type="hidden">';
            $facets_list['content'] .= '<input name="fieldtype" id="edit-fieldtype" value="' . $type . '" type="hidden">';
            return $facets_list['content'];

        // Text facets.
        case 'text':
            $facet_field = 'f_' . $field;
            $facet_list = tow_search_inner_facet_widget($response, $query, 'tow_inner_search', $facet_field, $facet_field, '', '_tow_search_inner_facet_trim_text');
            $form = drupal_get_form('tow_search_inner_text_length_widget_form', $length_field) . drupal_get_form('tow_search_inner_text_widget_form', $field, $type);
            return $facet_list['content'] . $form;

        // Char facets.
        case 'char':
            $facet_field = 'f_' . $field;
            $facet_list = tow_search_inner_facet_widget($response, $query, 'tow_inner_search', $facet_field, $facet_field, '');
            $form = drupal_get_form('tow_search_inner_text_length_widget_form', $length_field) . drupal_get_form('tow_search_inner_text_widget_form', $field, $type);
            return ($whole_counts > 50) ? $facet_list['content'] . $form : $facet_list['content'];

        default:
            return NULL;
    }
}

/**
 * Retrieves zooming factor from GET[zoom].
 */
function tow_search_inner_zoom($current_field) {
    $zoom_string = $_GET['zoom'];
    $zoom_array = explode(' ', $zoom_string);
    $current_zoom = 0;
    foreach ($zoom_array as $fieldzoom) {
        list($field, $zoom) = explode(':', $fieldzoom);
        if ($field == $current_field) {
            $current_zoom = $zoom;
            break;
        }
    }
    $num_of_points = _tow_search_inner_zoom_points($current_zoom);
    return $num_of_points ? $num_of_points : _tow_search_inner_zoom_points('0');
}

/* * **************************************************************************************************
 * Helper functions
 * ************************************************************************************************** */

/**
 * Adds facet range for specific field to solr request.
 */
function _tow_search_inner_add_facet_range(&$params, $field, $min, $max, $gap, $type) {
    if ($gap == _tow_search_inner_natural_gap($type)) {
        $params['f.' . $field . '.facet.range.other'] = 'after';
        $params['f.' . $field . '.facet.range.include'] = array('lower');
    }
    if (!isset($params['facet.range']) || !in_array($field, $params['facet.range'])) {
        $params['facet.range'][] = '{!ex=' . $field . '}' . $field;
    }
    $params['f.' . $field . '.facet.range.start'] = $min;
    $params['f.' . $field . '.facet.range.end'] = $max;
    $params['f.' . $field . '.facet.range.gap'] = $gap;
}

/**
 * Builds GET query for $param,
 * omitting param with empty value and getting value from GET
 * if value is FALSE.
 */
function _tow_search_inner_add_queryvalue(&$query, $param, $value) {
    if (!empty($value) && ($value !== FALSE)) {
        $query[$param] = $value;
    } elseif (!empty($_GET[$param]) && ($value === FALSE)) {
        $query[$param] = $_GET[$param];
    }
}

/**
 * Returns enum options.
 */
function _tow_search_inner_facet_enum_option($facet, &$options) {
    global $_tow_search_inner_field_delta;

    $response = apachesolr_static_response_cache(NULL, 'tow_inner_search');

    if (isset($response->response->docs[0])) {
        $table_nid = $response->response->docs[0]->ss_tow_field_table;
    }

    $table_node = node_load($table_nid);
    $enum_options = unserialize($table_node->field_enum_options[$_tow_search_inner_field_delta]['value']);

    if ($enum_options[$facet]) {
        return $enum_options[$facet];
    } else {
        $options['html'] = TRUE;
        return theme('placeholder', t('<empty>'));
    }
}

/**
 * Calculates optimal facet gap.
 */
function _tow_search_inner_facet_gap($range, $visible_range, $step = NULL, $number_of_points = 40) {
    if ($visible_range == 0) {
        $visible_range = $range;
    }

    $minstep = $visible_range / $number_of_points;
    if (is_null($step)) {
        return $minstep;
    } elseif ($minstep < $step) {
        return $step;
    } else {
        return $step * ceil($minstep / $step);
    }
}

/**
 * Trim facet text.
 */
function _tow_search_inner_facet_trim_text($facet, &$options) {
    if ($facet == '[* TO *]') {
        return theme('placeholder', t('<empty>'));
    } else {
        $alter = array(
            'html' => TRUE,
            'ellipsis' => TRUE,
            'word_boundary' => FALSE,
            'max_length' => 15,
        );

        return views_trim_text($alter, $facet);
    }
}

/**
 * Get keys.
 */
function _tow_search_inner_get_keys() {
    static $return;
    if (!isset($return)) {
        $path = explode('/', $_GET['q'], 3);
        $keys = empty($_REQUEST['keys']) ? '' : $_REQUEST['keys'];
        $return = count($path) == 3 ? $path[2] : $keys;
    }
    return $return;
}

/**
 * Helper function. Converts value from js format to solr format
 * (actions on date and time formats only).
 */
function _tow_search_inner_js_to_solr($type, $value) {
    switch ($type) {
        case 'int':
        case 'float':
            return $value;
        case 'date':
        case 'datetime':
            $value = round($value / 1000);
            _tow_apachesolr_timestamp_to_solr($value);
            return $value;
        case 'timestamp':
            return round($value / 1000);
        case 'time':
            $value = round($value / 1000);
            $value = gmdate('H:i:s', $value);
            return '0001-01-01T' . $value . 'Z';
        default:
            return NULL;
    }
}

/**
 * Returns natural gap for specifiec field type
 * 1 for int, +1SECOND for date and datetime etc.
 */
function _tow_search_inner_natural_gap($field_type, $js = FALSE) {
    switch ($field_type) {
        case 'int':
            return 1;
        case 'date':
            if ($js) {
                return 24 * 60 * 60 * 1000;
            } else {
                return 24 * 60 * 60;
            }
        case 'datetime':
        case 'time':
        case 'timestamp':
            if ($js) {
                return 1000;
            } else {
                return 1;
            }
        case 'float':
        default:
            return NULL;
    }
}

/**
 * Theming function. Return 'N results' for (N != 1) and '1 result' for (N = 1).
 */
function _tow_search_inner_number_of_results($N) {
    if ($N == 1) {
        return t('1 result');
    } else {
        return $N . t(' results');
    }
}

/**
 * Theme range.
 */
function _tow_search_inner_range($start, $end, $is_natural_step) {
    if ($is_natural_step) {
        return $start . ':\n';
    } else {
        return $start . '...\n' . $end . ':\n';
    }
}

/**
 * Helper function. Converts gap value from solr format to js format
 * (actions on date and time formats only).
 */
function _tow_search_inner_solr_gap_to_js($type, $value) {
    switch ($type) {
        case 'int':
        case 'float':
            return $value;
        case 'date':
        case 'datetime':
        case 'time':
            $value = preg_replace('/\D/', '', $value);
            return $value * 1000;
        case 'timestamp':
            return $value * 1000;
        default:
            return NULL;
    }
}

/**
 * Helper function. Converts value from solr format to js format
 * (actions on date and time formats only)
 */
function _tow_search_inner_solr_to_js($type, $value) {
    switch ($type) {
        case 'int':
        case 'float':
            return $value;
        case 'date':
        case 'datetime':
            return strtotime($value) * 1000;
        case 'timestamp':
            return $value * 1000;
        case 'time':
            _tow_apachesolr_solr_to_time($value);
            return strtotime('1970-01-01 ' . $value . 'Z') * 1000;
        default:
            return NULL;
    }
}

/**
 * Timezoom points.
 */
function _tow_search_inner_timezoom_points($zoom) {
    $num_of_points = array(
        '-5' => 4,
        '-4' => 6,
        '-3' => 8,
        '-2' => 12,
        '-1' => 24,
        '0' => 48,
        '1' => 72,
        '2' => 96,
        '3' => 144,
        '4' => 288,
    );
    return isset($num_of_points[$zoom]) ? $num_of_points[$zoom] : FALSE;
}

/**
 * Defines whether the widget's fieldset has to be shown collapsed.
 */
function _tow_search_inner_widget_fieldset_collapsed() {
    return FALSE;
}

/**
 * Zoom points.
 */
function _tow_search_inner_zoom_points($zoom) {
    $num_of_points = array(
        '-5' => 10,
        '-4' => 15,
        '-3' => 21,
        '-2' => 28,
        '-1' => 39,
        '0' => 50,
        '1' => 67,
        '2' => 89,
        '3' => 119,
        '4' => 160,
    );
    return isset($num_of_points[$zoom]) ? $num_of_points[$zoom] : FALSE;
}
