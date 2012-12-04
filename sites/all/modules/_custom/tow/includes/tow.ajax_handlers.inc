<?php

/**
 * @file
 *
 * Ajax handlers.
 */
/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Handles the creation of a saved search with Ajax.
 */
function tow_ajax_handlers_create_saved_search($nid) {

    // Get current node.
    if (!is_numeric($nid)) {
        $nid = str_replace(' ', '+', $nid);
        $nid_array = explode('+', $nid);
        $node = node_load($nid_array[0]);
    } else {
        $node = node_load($nid);
    }
    $node->nid = $nid;

    // Get filters.
    $filters = $_GET['filters'];

    // Selected fields.
    $selected_fields = (isset($_GET['selected_fields'])) ? $_GET['selected_fields'] : json_encode(new stdClass());

    //Saved search comment (title)
    $ss_comment = $_GET['ss_comment'];

    // Rows amount.
    $rows_amount = $_GET['rows_amount'];

    // Create saved search.
    $saved_search = tow_saved_searches_saved_search_create($node, $ss_comment, $filters, $selected_fields, $rows_amount);

    // Validation.
    if ($saved_search == NULL) {
        $json = array(
            'saved' => FALSE,
            'message' => t("The search you are trying to save already exists."),
        );
    } else {
        try {
            node_save($saved_search);
            $block = tow_saved_searches_list_block($nid);
            $json = array(
                'saved' => TRUE,
                'saved_searches' => $block['content'],
            );
            
            //Update {apachesolr_search_node} table for indexing updates in saved_searches
            db_query("UPDATE {apachesolr_search_node} SET changed = %d WHERE nid = %d", time(), $nid);
        
        } catch (Exception $e) {
            watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
            $json = array(
                'saved' => FALSE,
                'message' => $e->getMessage(),
            );
        }
    }

    return drupal_json($json);
}

/**
 * Handles the change of access type to the dataset with Ajax.
 */
function tow_ajax_handlers_dataset_access_type($dnid) {

    // Get the dataset.
    $dataset = node_load($dnid);

    // Get the arguments.
    $access = $_GET['access'];
    $confirm = (isset($_GET['confirm']) && $_GET['confirm'] == 'true') ? TRUE : FALSE;

    // If the change was confirmed or the confirmation message was not shown (for 'by request' and 'private' types).
    if ($confirm) {
        $dataset->field_access_type[0]['value'] = $access;
        try {
            node_save($dataset);
        } catch (Exception $e) {
            watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
        }
    }

    // JSON.
    $json = array(
        'selected' => $dataset->field_access_type[0]['value'],
        'admin_tasks' => _tow_dataset_admin_tasks($dataset),
    );

    return drupal_json($json);
}

/**
 * Handles the change of bookmark count for the dataset with Ajax.
 */
function tow_ajax_handlers_dataset_bookmark_count($dnid) {

    // JSON.
    $flag = flag_get_flag('bookmarks');
    $json = array(
        'bookmark_count' => $flag->get_count($dnid),
    );

    return drupal_json($json);
}

/**
 * Handles refresh of table samples with Ajax.
 */
function tow_ajax_handlers_dataset_refresh_table_samples($dnid) {

    // Prepare dataset.
    $dataset = node_load($dnid);
    dcp($dataset);
    tow_dataset_summary($dataset);

    // Generate the output.
    $output = '';
    if (isset($dataset->summary['view_table'])) {
        $output .= '<div class="sample-tables">';
        $output .= '<span class="field-label">Some random samples from this data set</span>';
        $output .= '<span>&nbsp;(<a href= "node/' . $dataset->nid . '">refresh</a> for a new sample)</span>';
        foreach ($dataset->summary['view_table'] as $t) {
            $output .= '<div class="random-table">';
            $output .= $t['title'];
            $output .= $t['table'];
            $output .= '</div>';
        }
        $output .= '</div>';
    }

    // JSON.
    $json = array(
        'samples' => $output
    );

    // Return the output.
    return drupal_json($json);
}

/**
 * Builds part of solr query for getting facet numbers for table field.
 */
function tow_ajax_handlers_field_zooming() {
    $rid = $_GET['rid'];
    $hash = $_GET['hash'];

    if ($_GET['op'] == 'zoom') {
        $visible_min = $_GET['visible_min'];
        $visible_max = $_GET['visible_max'];
        $field_type = $_GET['field_type'];
        $field = $_GET['field'];

        db_query("

      UPDATE
        {tow_apachesolr_requests}

        SET
          timestamp = '%s'

        WHERE
            rid = %d
          AND
            hash = %d

    ", time(), $rid, $hash);

        $result = db_fetch_array(db_query("

      SELECT
        t.query AS query,
        t.stats AS stats,
        t.tnid AS tnid

        FROM
          {tow_apachesolr_requests} t

        WHERE
            t.rid = %d
          AND
            t.hash = %d

    ", $rid, $hash));

        $serialized_query = $result['query'];
        $serialized_stats = $result['stats'];
        $table_nid = $result['tnid'];
        $_GET['q'] = 'search_dataset/' . $table_nid;

        $query = apachesolr_drupal_query(NULL, NULL, NULL, NULL);
        $serialized_query = str_replace("~~NULL_BYTE~~", "\0", $serialized_query);
        $query = unserialize($serialized_query);

        $serialized_stats = str_replace("~~NULL_BYTE~~", "\0", $serialized_stats);
        $stats = unserialize($serialized_stats);
        $params = array();

        // Building solr query for facets for each field.
        $stats = isset($stats->$field) ? $stats->$field : NULL;
        tow_search_inner_facet_query($params, $field, $field_type, $stats, FALSE, $visible_max - $visible_min);

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

            // JSON.
            $json = NULL;

            return drupal_json($json);
        }

        list($data, $facets_list, $start, $end, $max_count) = tow_search_inner_chart_data($field, $field_type, $response->facet_counts->facet_ranges->$field);
        $series = array();
        foreach ($data as $index => $point) {
            $series[] = array(
                'x' => $point[0],
                'y' => $point[1],
                'name' => $facets_list[$index],
            );
        }

        // JSON.
        $json = array(
            'data' => $series,
            'yMax' => $max_count
        );

        return drupal_json($json);
    } else {
        db_query("

      DELETE
        FROM
          {tow_apachesolr_requests}

        WHERE
            rid = %d
          AND
            hash = %d
    ", $rid, $hash);
    }
}

/**
 * Gets progress stage.
 */
function tow_ajax_handlers_import_progress_get($hash, $stage) {
    global $user;

    // Get stage.
    $response = db_result(db_query("SELECT stage%s FROM {tow_import_progress} WHERE sid = '%s' AND hash = %d", $stage, $user->sid, $hash));

    // Delete actions on delete satge.
    if ($stage == 6) {
        tow_import_progress_delete($hash);
    }

    // JSON.
    $json = array(
        'status' => TRUE,
        'response' => $response
    );

    return drupal_json($json);
}

/**
 * When user clicks 'add more' return rebuilt form.
 * This is just interface, rebuilding goes in tow_register_form_alter().
 */
function tow_ajax_handlers_register_add_more() {

    $form_state = array('storage' => NULL, 'submitted' => FALSE);
    $form_build_id = $_POST['form_build_id'];
    $form = form_get_cache($form_build_id, $form_state);

    $args = $form['#parameters'];
    $form_id = array_shift($args);
    $form_state['post'] = $form['#post'] = $_POST;
    $form['#programmed'] = $form['#redirect'] = FALSE;

    drupal_process_form($form_id, $form, $form_state);
    $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

    $items = $form['feeds']['items'];
    $output = drupal_render($items);

    // JSON.
    $json = array(
        'status' => TRUE,
        'data' => $output
    );

    return drupal_json($json);
}

/**
 * Refresh search results according to filters set with Ajax.
 */
function tow_ajax_handlers_refresh_search_results($nid) {

    // Fields list block content.
    $fields = tow_search_inner_field_list_block();

    // Emulate we are on the search page.
    $_GET['q'] = str_replace('/refresh_ajax', '', $_GET['q']);

    // Define search data.
    $search = tow_search_inner_within_dataset($nid);

    // Generate new widgets block output.
    $widgets = tow_search_inner_widgets_block();

    // Generate new Search description block output.
    $save_search = tow_saved_searches_description_block();

    // Generate new Save this search block output.
    $save_this_search = tow_saved_searches_save_search_block();
    $save_this_search['content'] = str_replace($save_search['content'], '', $save_this_search['content']);

    global $user;

    // JSON.
    $json = array(
        'search' => $search,
        'widgets' => $widgets['content'],
        'fields' => $fields['content'],
        'save_search' => $save_search['content'],
        'save_this_search' => ($user->uid) ? $save_this_search['content'] : NULL,
    );

    return drupal_json($json);
}

/**
 * Proxy for outer search using Ajax.
 */
function tow_ajax_handlers_search_outer() {
    $_GET['q'] = str_replace('search_outer_ajax/', '', $_GET['q']);
    $results = tow_search_outer_view();
    $form = drupal_get_form('tow_search_outer_filter_form');
    $number_block = tow_search_outer_number_of_results_block();
    $tabs_block = tow_search_outer_tabs_block();

    // JSON.
    $json = array(
        'results' => $results,
        'form' => $form,
        'number' => $number_block['content'],
        'tabs' => $tabs_block['content'],
    );

    return drupal_json($json);
}

//Database change user dataset access through noty messages
function tow_ajax_handlers_noty_access($status, $nid, $uid) {
    db_query("

          UPDATE
            {tow_access_requests}

            SET
              status = %d

            WHERE
                nid = %d
              AND
                uid = %d

     ", $status, $nid, $uid);
    
    //Update {apachesolr_search_node} table for indexing updates in access
    db_query("UPDATE {apachesolr_search_node} SET changed = %d WHERE nid = %d", time(), $nid);
}

