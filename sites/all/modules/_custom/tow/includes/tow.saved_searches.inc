<?php

/**
 * @file
 *
 * Performs Saved Searches functionality.
 */
/* * **************************************************************************************************
 * Hook handlers
 * ************************************************************************************************** */

/**
 * Saved searches block.
 */
function tow_saved_searches_list_block($nid = NULL) {
    $nid = (isset($nid)) ? $nid : arg(1);
    $nid = str_replace(' ', '+', $nid);

    $is_dataset = FALSE;
    if (is_numeric($nid)) {
        $o = node_load($nid);
        $is_dataset = ($o->type == 'dataset') ? TRUE : FALSE;
    }

    if ($is_dataset) {
        $query = db_query("

      SELECT
        ctss.nid AS nid

        FROM
          {content_type_saved_search} ctss

        WHERE
          ctss.field_dataset_nid_value = %d

    ", $nid);
    } else {
        $query = db_query("

      SELECT
        ctss.nid AS nid

        FROM
          {content_type_saved_search} ctss

        WHERE
          ctss.field_table_nid_value = '%s'

    ", $nid);
    }
    $saved_searches = array();
    while ($result = db_fetch_object($query)) {
        $saved_searches[] = node_load($result->nid);
    }

    $block['title'] = t('User analysis');
    $block['content'] = '';

    foreach ($saved_searches as $node) {

        $output = '<div class="tow-saved-search-block-item">';
        $output .= node_view($node);
        if (function_exists('comment_render') && $node->comment) {
            $output .= comment_render($node, NULL);
        }
        $output .= '</div>';

        $block['content'] .= $output;
    }

    return $block;
}

/**
 * Save this search block.
 */
function tow_saved_searches_save_search_block() {

    // Construct block.
    $block = array();
    $block['title'] = NULL;

    $saved_response = apachesolr_static_response_cache(NULL, 'tow_inner_search');
    $rows_amount = $saved_response->response->numFound;

    $block['content'] = drupal_get_form('tow_saved_searches_save_search_form', $rows_amount);

    return $block;
}

/**
 * Search description block.
 */
function tow_saved_searches_description_block() {

    // Construct block.
    $block = array();
    $block['title'] = t('Search description');

    if (arg(0) != 'search_dataset') {
        $block['content'] = '';
    } else {
        $saved_response = apachesolr_static_response_cache(NULL, 'tow_inner_search');
        $rows_amount = $saved_response->response->numFound;
        $ssd_form = tow_saved_searches_save_search_form(&$form_state, $rows_amount);
        $block['content'] = 'Choose your search parameters';
        if ($ssd_form['summary']['#value']) {
            $block['content'] = $ssd_form['summary']['#value'];
        }
    }

    return $block;
}

/* * **************************************************************************************************
 * Forms
 * ************************************************************************************************** */

/**
 * Defines a form with button for saving search.
 */
function tow_saved_searches_save_search_form(&$form_state, $rows_amount) {
    $form = array();

    $nid = arg(1);
    $filters = $_GET['filters'];
    $selected_fields = (isset($_GET['selected_fields'])) ? $_GET['selected_fields'] : json_encode(new stdClass(), TRUE);

    // Saved search comment(title).
    $form['comment'] = array(
        '#type' => 'textarea',
        '#title' => NULL,
        '#rows' => 2,
        '#resizable' => false,
        '#attributes' => array(
            'placeholder' => t('Explain your findings...'),
        ),
    );

    // Filters.
    $form['filters'] = array(
        '#type' => 'hidden',
        '#value' => $filters,
    );

    // Selected fields.
    $form['selected_fields'] = array(
        '#type' => 'hidden',
        '#value' => $selected_fields,
    );

    // Rows amount.
    $form['rows_amount'] = array(
        '#type' => 'hidden',
        '#value' => $rows_amount,
    );

    // Summary.
    $form['summary'] = array(
        '#value' => tow_saved_searches_summary($nid, $filters, $selected_fields, $rows_amount),
    );

    // Button.
    $form['save_search'] = array(
        '#type' => 'submit',
        '#value' => t('Share analysis'),
        '#validate' => array('tow_saved_searches_save_search_form_validate'),
        '#submit' => array('tow_saved_searches_save_search_form_submit'),
    );

    // Action.
    $form['#action'] = str_replace('/refresh_ajax', '', $_GET['q']);
    $form['#action'] = str_replace('search_dataset/', '', $form['#action']);
    $form['#action'] = '/search_dataset/' . $nid;

    return $form;
}

/* * **************************************************************************************************
 * Validation callbacks
 * ************************************************************************************************** */

/**
 * Checks whether current search has already been saved.
 */
function tow_saved_searches_save_search_form_validate(&$form, &$form_state) {

    // Get current node.
    $nid = arg(1);
    if (!is_numeric($nid)) {
        $nid = str_replace(' ', '+', $nid);
        $nid_array = explode('+', $nid);
        $node = node_load($nid_array[0]);
    } else {
        $node = node_load($nid);
    }
    $node->nid = $nid;

    // Get filters.
    $form['filters']['#value'] = $form['filters']['#post']['filters'];
    $form_state['values']['filters'] = $form['filters']['#value'];
    $filters = $form_state['values']['filters'];

    // Selected fields.
    $form['selected_fields']['#value'] = $form['selected_fields']['#post']['selected_fields'];
    $form_state['values']['selected_fields'] = $form['selected_fields']['#value'];
    $selected_fields = $form_state['values']['selected_fields'];

    // Rows amount.
    $form['rows_amount']['#value'] = $form['rows_amount']['#post']['rows_amount'];
    $form_state['values']['rows_amount'] = $form['rows_amount']['#value'];
    $rows_amount = $form_state['values']['rows_amount'];

    // Saved search comment(title).
    $ss_comment = $form['comment']['#value'];

    // Create saved search.
    $saved_search = tow_saved_searches_saved_search_create($node, $ss_comment, $filters, $selected_fields, $rows_amount);

    // Put it to the form.
    $form_state['#saved_search'] = $saved_search;

    // Validation.
    if ($saved_search == NULL) {
        form_set_error('filters', check_plain(t("The search you are trying to save already exists.")));
    }
}

/* * **************************************************************************************************
 * Submission callbacks
 * ************************************************************************************************** */

/**
 * Save search.
 */
function tow_saved_searches_save_search_form_submit(&$form, &$form_state) {
    $saved_search = $form_state['#saved_search'];
    if ($saved_search != NULL) {
        try {
            node_save($saved_search);
        } catch (Exception $e) {
            watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
        }
    }
}

/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Tells if there is a search with exactly the same constraints had been saved before.
 */
function tow_saved_searches_check_exists($saved_search) {

    // Get the NID of the dataset this search belongs to.
    $dataset_nid = $saved_search->field_dataset_nid[0]['value'];

    // Get all the saerches previously saved for the current dataset.
    $dataset_all_saved_searches = array();
    $query = db_query("

    SELECT
      t.nid AS nid

      FROM
        {content_type_saved_search} t

      WHERE
        t.field_dataset_nid_value = %d

  ", $dataset_nid);
    while ($record = db_fetch_object($query)) {
        $dataset_all_saved_searches[] = node_load($record->nid);
    }

    // Check if the current search is equal to any of the previously saved (checked by hash).
    $exists = FALSE;
    foreach ($dataset_all_saved_searches as $ss) {
        if ($ss->field_sc_hash[0]['value'] == $saved_search->field_sc_hash[0]['value']) {
            $exists = TRUE;
            break;
        }
    }

    return $exists;
}

/**
 * Gets and sorts an array of search constraints.
 */
function tow_saved_searches_get_constraints($filters) {

    // Return empty string if there are no filters.
    if (!isset($filters) || $filters == '') {
        return array();
    }

    // Get an array of constraints.
    preg_match_all('/((\S+):\*.*\*)|((\S+):".*")|((\S+):[\[\{](\S+) TO (\S+)[\]\}])/U', $filters, $matches);
    $constraints = $matches[0];
    foreach ($constraints as &$constraint) {
        trim($constraint);
        $filters = str_replace($constraint, '', $filters);
    }
    $constraints = array_merge(explode(' ', $filters), $constraints);
    $constraints = array_filter($constraints);

    // Sort.
    sort($constraints);

    return $constraints;
}

/**
 * Gets an array of selected fields.
 */
function tow_saved_searches_get_selected_fields($selected_fields) {

    // Return empty string if there are no filters.
    if (!isset($selected_fields) || $selected_fields == json_encode(new stdClass())) {
        return new stdClass();
    }

    // Get an array of constraints.
    $selected_fields = json_decode($selected_fields, TRUE);

    return $selected_fields;
}

/**
 * Constructs a Saved Search node object.
 */
function tow_saved_searches_saved_search_create($node, $ss_comment, $filters, $selected_fields, $rows_amount = 0) {
    module_load_include('inc', 'node', 'node.pages');

    // Prepare a new Saved Search object.
    $saved_search = new stdClass();
    $saved_search->type = 'saved_search';
    $saved_search->title = ($ss_comment) ? $ss_comment : t('New Saved Search');
    node_object_prepare($saved_search);

    // Do this if the search belongs to a whole dataset.
    if ($node->type == 'dataset') {
        $saved_search->field_dataset_nid[0]['value'] = $node->nid;
        $saved_search->field_table_nid[0]['value'] = 0;
    }

    // Do this if the search belongs to a specified table within a dataset..
    if ($node->type == 'table') {
        $saved_search->field_table_nid[0]['value'] = $node->nid;
        $saved_search->field_dataset_nid[0]['value'] = $node->field_dataset[0]['value'];
        $saved_search->body = tow_saved_searches_summary($node->nid, $filters, $selected_fields, $rows_amount);
        $saved_search->teaser = node_teaser($saved_search->body);
    }

    // Save search constraints.
    $filters = tow_saved_searches_sort_constraints($filters);
    $selected_fields = tow_saved_searches_sort_selected_fields($selected_fields);
    $saved_search->field_search_constraints[0]['value'] = $filters;
    $saved_search->field_selected_fields[0]['value'] = $selected_fields;

    // Calculate a hash.
    $saved_search->field_sc_hash[0]['value'] = md5($saved_search->field_search_constraints[0]['value'] . ' ' . $saved_search->field_selected_fields[0]['value']);

    // Check if the search already exists.
    $exists = tow_saved_searches_check_exists($saved_search);

    // Return saved search if it is a unique one or NULL otherwise.
    return (!$exists) ? $saved_search : NULL;
}

/**
 * Sorts filters.
 */
function tow_saved_searches_sort_constraints($filters) {

    $constraints = tow_saved_searches_get_constraints($filters);

    // Get the values for ecah constraint into an array.
    $sort_array = array();
    foreach ($constraints as $constraint) {
        $aux = explode(':', $constraint, 2);
        $sort_array[$aux[0]][] = $aux[1];
    }

    // Sort all the values within a constraint...
    foreach ($sort_array as &$constraint) {
        sort($constraint);
    }
    // ...and sort constraints themselves.
    asort($sort_array);

    // Construct a new filter string.
    $filters = '';
    foreach ($sort_array as $key => $items) {
        foreach ($items as $value) {
            $filters .= $key . ':' . $value . ' ';
        }
    }
    $filters = trim($filters);

    return $filters;
}

/**
 * Sorts selected fields.
 */
function tow_saved_searches_sort_selected_fields($selected_fields) {

    $sort_array = tow_saved_searches_get_selected_fields($selected_fields);

    // Sorting.
    if (!is_object($sort_array)) {
        asort($sort_array);
    }

    // Construct a new selected fields string.
    $selected_fields = json_encode($sort_array);

    return $selected_fields;
}

/**
 * Generates a summary for the saved search.
 */
function tow_saved_searches_summary($table_nid, $filters, $selected_fields, $rows_amount = 0) {

    $fields = tow_apachesolr_table_fields($table_nid, TRUE);

    // Get an array of constraints.
    $constraints = tow_saved_searches_get_constraints($filters);
    $selected_fields = tow_saved_searches_get_selected_fields($selected_fields);

    // Get the values for ecah constraint into an array.
    $info = array();
    foreach ($fields as $field) {
        if (empty($selected_fields) || in_array($field['title'] . '_' . $field['type'], $selected_fields)) {
            $info[$field['name']] = $field;
        }
    }

    foreach ($constraints as $constraint) {

        // Detect whether the constraint is included or excluded.
        $aux = explode(':', $constraint, 2);
        if (substr($aux[0], 0, 1) == '-') {
            $clude = 'excluding';
            $aux[0] = substr($aux[0], 1);
        } else {
            $clude = 'including';
        }

        // Generate an output of the ranged values.
        if (preg_match('@[\[\{](\S+) TO (\S+)[\]\}]@', $aux[1])) {

            // Get values.
            $aux[1] = substr($aux[1], 1, strlen($aux[1]) - 2);
            $aux[1] = str_replace(' TO ', ' ', $aux[1]);
            list($start, $end) = explode(' ', $aux[1]);

            // Generate output according to type.
            switch ($info[$aux[0]]['type']) {

                // Date.
                case 'date':
                    $start = ($start == '*') ? $start : date('Y-m-d', strtotime($start));
                    $end = ($end == '*') ? $end : date('Y-m-d', strtotime($end));
                    break;

                // Datetime.
                case 'datetime':
                    $start = ($start == '*') ? $start : date('Y-m-d H-i-s', strtotime($start));
                    $end = ($end == '*') ? $end : date('Y-m-d H-i-s', strtotime($end));
                    break;

                // Time.
                case 'time':
                    $start = ($start == '*') ? $start : date('H-i-s', strtotime($start));
                    $end = ($end == '*') ? $end : date('H-i-s', strtotime($end));
                    break;

                // Timestamp.
                case 'timestamp':
                    $start = ($start == '*') ? $start : date('Y-m-d H-i-s', strtotime($start));
                    $end = ($end == '*') ? $end : date('Y-m-d H-i-s', strtotime($end));
                    break;
            }

            // Set prefix and suffix according for specific search constraints.
            $prefix = '';
            $suffix = '';
            if (strpos($aux[0], '_length')) {
                $prefix = ' with';
                $suffix = ' characters';
            } elseif (strpos($aux[0], '_word')) {
                $prefix = ' with';
                $suffix = ' words';
            }

            // Define sentences according to limit presence.
            if ($start == '*' && $end == '*') {
                $info[$aux[0]]['search'][$clude][] = "empty values";
            } elseif ($start == '*') {
                $info[$aux[0]]['search'][$clude][] = "values$prefix less than $end$suffix";
            } elseif ($end == '*') {
                $info[$aux[0]]['search'][$clude][] = "values$prefix more than $start$suffix";
            } else {
                $info[$aux[0]]['search'][$clude][] = "values from $start to $end";
            }
        }

        // For text values.
        else {

            // Define the output text for each type of string.
            list($start, $end) = array(substr($aux[1], 0, 1), substr($aux[1], strlen($aux[1]) - 1, 1));
            if ($start == '*' && $end == '*') {
                $info[$aux[0]]['search'][$clude][] = 'values containing substring "' . substr($aux[1], 1, strlen($aux[1]) - 2) . '"';
            } elseif ($start == '*' && $end != '*') {
                $info[$aux[0]]['search'][$clude][] = 'values starting with substring "' . substr($aux[1], 1, strlen($aux[1]) - 1) . '"';
            } elseif ($end == '*' && $start != '*') {
                $info[$aux[0]]['search'][$clude][] = 'values ending with substring "' . substr($aux[1], 0, strlen($aux[1]) - 2) . '"';
            } elseif ($start == '"' && $end == '"') {
                $info[$aux[0]]['search'][$clude][] = "values equal to $aux[1]";
            } else {

                // Output of word/length dependent values.
                if (strpos($aux[0], '_length')) {
                    $info[$aux[0]]['search'][$clude][] = "values with $aux[1] characters";
                } elseif (strpos($aux[0], '_word')) {
                    $info[$aux[0]]['search'][$clude][] = "values with $aux[1] words";
                } else {
                    if ($info[$aux[0]]['type'] == 'enum') {
                        $aux[1] = $info[$aux[0]]['enum_options'][$aux[1]];
                    }
                    $info[$aux[0]]['search'][$clude][] = 'values equal to "' . $aux[1] . '"';
                }
            }
        }
    }

    // Build summary.
    $summary = '';

    // Searched for part.
    $summary .= t('Searched for');
    $empty = TRUE;
    foreach ($info as $field) {
        if (isset($field['search'])) {
            $empty = FALSE;
            $summary .= ' \'' . $field['title'] . '\' ';
            foreach ($field['search'] as $param_type => $param) {
                foreach ($param as $value) {
                    $summary .= $param_type . ' ' . $value . ' and ';
                }
            }
            $summary = rtrim($summary, ' and');
            $summary .= ',';
        }
    }
    if ($empty) {
        $summary = '';
    } else {
        $summary = rtrim($summary, ',');
        $summary .= '.';
    }

    // Displaying fields part.
    $titles = array();
    foreach ($info as $field) {
        $titles[] = $field['title'];
    }
    $titles = array_unique($titles);
    $summary .= (empty($titles)) ? '' : ' ' . 'Displaying fields:';
    foreach ($titles as $field) {
        $summary .= ' \'' . $field . '\'' . ',';
    }
    $summary = rtrim($summary, ',');
    $summary .= (empty($titles)) ? '' : '.';

    // Rows amount part
    $summary .= ($rows_amount === 0) ? '' : "<strong> $rows_amount row" . (($rows_amount == 1) ? "" : "s") . " of results.</strong>";

    // Finish building the summary.
    $summary .= '';

    return $summary;
}
