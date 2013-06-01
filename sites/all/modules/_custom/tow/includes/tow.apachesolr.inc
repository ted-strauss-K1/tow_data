<?php

/**
 * @file
 *
 * Apachesolr interface extensions.
 */
/* * **************************************************************************************************
 * Hooks
 * ************************************************************************************************** */

/**
 * Part of hook_form_alter() implementation.
 */
function tow_apachesolr_form_alter(&$form, &$form_state, $form_id) {
    
    // New possible cron limits.
    $numbers = drupal_map_assoc(array(1, 5, 10, 20, 50, 100, 200, 500, 750, 1000, 2000, 5000, 10000));
    
    // Apachesolr cron limit may be overridden in settings.php. If its current value is not among the default set of options, add it.
    $default_cron_limit = variable_get('apachesolr_cron_limit', 50);
    if (!isset($numbers[$default_cron_limit])) {
        $numbers[$default_cron_limit] = $default_cron_limit;
    }
    
    $form['apachesolr_cron_limit']['#options'] = $numbers;
}

/* * **************************************************************************************************
 * Apachesolr hooks
 * ************************************************************************************************** */

/**
 * Implementation of hook_apachesolr_ajax_settings().
 */
function tow_apachesolr_ajax_settings() {

    // Set content and regions.
    $settings = array(
        'content' => '#content-inner-inner',
        'regions' => array(
            'sidebar_last' => '.sidebar-last',
        ),
    );

    // Blocks.
    $modules = apachesolr_ajax_modules();
    foreach ($modules as $module) {
        if (module_exists($module)) {
            if ($list = module_invoke($module, 'block', 'list')) {
                foreach (array_keys($list) as $delta) {
                    $settings['blocks'][$module . '_' . $delta] = '#block-' . $module . '-' . $delta;
                }
            }
        }
    }
    return $settings;
}

/**
 * Implementation of hook_apachesolr_modify_query().
 */
function tow_apachesolr_modify_query(&$query, &$params) {

    $current_tab = tow_search_outer_get_homepage_tab();
    $query->add_filter('hash', apachesolr_site_hash());

    if ($current_tab == 'search_dataset') {

        // Inner search.
        tow_apachesolr_record_modify_query($query, $params);
    } elseif ($current_tab == 'newest' || $current_tab == 'debated' || $current_tab == 'featured' || $current_tab == 'popular') {

        // Outer search.
        tow_apachesolr_dataset_modify_query($query, $params);
    } elseif ($current_tab == 'user') {

        // Outer search for user.
        if (arg(2) == 'datasets') {
            tow_apachesolr_dataset_user_modify_query($query, $params);
        }
        if (arg(2) == 'saved_searches') {
            tow_apachesolr_saved_search_user_modify_query($query, $params);
        }
    }
}

/**
 * Implementation of hook_apachesolr_process_results().
 */
function tow_apachesolr_process_results(&$results) {

    if ($results[0]['node']->type == 'dataset') {

        // If search was made for datasets.
        //foreach ($results as $key => $result) {
        //if ($results[$key]['node']->sis_cck_field_access_type == 2) {
        //unset ($results[$key]);
        //}
        //}
        tow_apachesolr_dataset_process_results($results);
    } elseif ($results[0]['node']->type == 'saved_search') {

        // If search was made for saved searches.
        tow_apachesolr_saved_search_process_results($results);
    } else {

        // Search for records option.
        tow_apachesolr_record_process_results($results);
    }
}

/**
 * Implementation of hook_apachesolr_update_index().
 * */
function tow_apachesolr_update_index(&$document, $node) {

    // Dataset processing.
    if ($node->type == 'dataset') {
        $document->sis_tow_amount_tables = db_result(db_query("

      SELECT
        COUNT(*)

        FROM
          {content_type_table}

        WHERE
          field_dataset_value = %d

    ", $node->nid));
        $document->sis_tow_amount_tables = (isset($document->sis_tow_amount_tables) && $document->sis_tow_amount_tables) ? $document->sis_tow_amount_tables : 0;

        // Get amount of records.
        $document->sis_tow_amount_records = db_result(db_query("

      SELECT
        COUNT(*) AS qty

        FROM
            {content_type_record} r
          INNER JOIN
            {content_type_table} t
            ON
                r.field_table_value = t.nid
              AND
                t.field_dataset_value = %d

    ", $node->nid));
        $document->sis_tow_amount_records = (isset($document->sis_tow_amount_records) && $document->sis_tow_amount_records) ? $document->sis_tow_amount_records : 0;

        // Get amount of saved searches.
        $document->sis_tow_amount_saved_searches = db_result(db_query("

      SELECT
        COUNT(*)

        FROM
          {content_type_saved_search} ss

        WHERE
          field_dataset_nid_value = %d

    ", $node->nid));
        $document->sis_tow_amount_saved_searches = (isset($document->sis_tow_amount_saved_searches) && $document->sis_tow_amount_saved_searches) ? $document->sis_tow_amount_saved_searches : 0;
        
        // Get amount for #Debated.
        $amount_debated = 0;
        $amount_debated += $document->sis_tow_amount_saved_searches;
        $res = db_query("

            SELECT
              ctss.nid AS nid,
              ncs.comment_count AS comment_count

              FROM
                {content_type_saved_search} ctss

              JOIN
                {node_comment_statistics} ncs

              ON
                ctss.nid = ncs.nid

              WHERE
                field_dataset_nid_value = %d

        ", $node->nid);
        while($row = db_fetch_object($res)) {
            $votes_number = db_result(db_query("

                SELECT
                  COUNT(*)

                  FROM
                    {votingapi_vote} vv

                  WHERE
                    content_id = %d

            ", $row->nid));

            $amount_debated += $row->comment_count;
            $amount_debated += $votes_number;
        }
        $document->sis_tow_amount_debated = (isset($document->sis_tow_amount_saved_searches) && $document->sis_tow_amount_saved_searches) ? $amount_debated : 0;

        // Get amount of views.
        tow_statistics_dataset_extract($node->nid);
        global $_tow_dataset_statistics;
        $document->sis_tow_count_views = $_tow_dataset_statistics['count'];
        $document->sis_tow_count_views = (isset($document->sis_tow_count_views) && $document->sis_tow_count_views) ? $document->sis_tow_count_views : 0;

        // Get teaser.
        $document->ss_picture = $node->picture;
        $alter = array(
            'html' => TRUE,
            'ellipsis' => TRUE,
            'word_boundary' => TRUE,
            'max_length' => 255,
        );
        $document->ss_teaser = views_trim_text($alter, $node->body);

        //Build request to show private datasets with granted access
        if ($node->field_access_type[0]['value'] == 2) {
            $result = db_query("

      SELECT
        tar.uid,
        tar.nid,
        tar.status

        FROM
          {tow_access_requests} tar

        JOIN
          {node} n

        ON
          tar.nid = n.nid

        WHERE
          tar.nid = %d

        AND
          tar.status = 1

    ", $node->nid);

            $users = array();
            while ($row = db_fetch_object($result)) {
                $users[] = $row->uid;
            }
            $document->im_tow_user_access_private = $users;
        }
    }

    // Record processing.
    elseif ($node->type == 'record') {

        // Add dataset_nid field and table_nid field to be indexed by apachesolr.
        $document->sis_tow_dataset_nid = db_result(db_query("

      SELECT
        ctt.field_dataset_value AS dataset_nid

        FROM
            {content_type_table} ctt
          LEFT JOIN
            {content_type_record} ctr
            ON
              ctr.field_table_value = ctt.nid

        WHERE
          ctr.nid = %d

    ", $node->nid));
        $table_nid = $node->field_table[0]['value'];
        $document->ss_tow_field_table = $table_nid;

        // Add values to doc.
        foreach (tow_apachesolr_table_fields($table_nid) as $field) {
            $value = $node->{'field_' . $field['type']}[$field['delta']]['value'];
            if (isset($value)) {
                _tow_apachesolr_add_value_to_doc(&$document, $field['type'], $field['delta'], $value);
            }
        }
    }

    // Linked record processing.
    elseif ($node->type == 'linked_record') {

        // Put tables.
        $tables = array();
        foreach ($node->field_tables as $table) {
            $tables[] = $table['value'];
        }
        $sorted_tables = $tables;
        sort($sorted_tables);
        $document->ss_tow_field_table = implode('+', $sorted_tables);

        // Put records.
        $records = array();
        foreach ($sorted_tables as $key => $table_nid) {
            $record_nid = $node->field_record_nid[array_search($table_nid, $tables)]['value'];
            $record = node_load($record_nid);
            $records[$table_nid] = $record;
            $document->{'sis_tow_linked_table_nid' . '_' . $key} = $table_nid;
            $document->{'sis_tow_linked_record_nid' . '_' . $key} = (is_null($record_nid)) ? 0 : $record_nid;
            $document->{'sis_tow_l_p' . '_' . $key} = (is_null($record_nid)) ? 0 : $record->field_position[0]['value'];
        }
        foreach (tow_apachesolr_table_fields($tables) as $field) {
            foreach ($field['table'] as $table) {
                $record = $records[$table['nid']];
                if (!empty($record)) {
                    $value = $record->{'field_' . $field['type']}[$table['delta']]['value'];

                    // Add values to docs.
                    if (isset($value)) {
                        _tow_apachesolr_add_value_to_doc(&$document, $field['type'], $field['delta'], $value);
                    }
                    break;
                }
            }
        }
    } elseif ($node->type == 'saved_search') {
        $ss = node_load($node->nid);
        $document->sis_tow_dataset_nid = db_result(db_query("

      SELECT
        ss.field_dataset_nid_value

        FROM
          {content_type_saved_search} ss

        WHERE
          ss.nid = %d

    ", $node->nid));
        $dataset = node_load($document->sis_tow_dataset_nid);
        $document->ss_tow_dataset_title = $dataset->title;
        $document->ss_tow_dataset_path = 'node/' . $dataset->nid;
        $document->ss_tow_table_nid = db_result(db_query("

      SELECT
        ss.field_table_nid_value

        FROM
          {content_type_saved_search} ss

        WHERE
          ss.nid = %d

    ", $node->nid));
        $document->ss_tow_search_constraints = db_result(db_query("

      SELECT
        ss.field_search_constraints_value

        FROM
          {content_type_saved_search} ss

        WHERE
          ss.nid = %d

    ", $node->nid));
        $document->ss_tow_saved_search_path = 'search_dataset/' . (($node->field_table_nid[0]['value'] == 0) ? $node->field_dataset_nid[0]['value'] : $node->field_table_nid[0]['value']) . (($node->field_search_constraints[0]['value'] == '') ? '' : '#?filters=' . $node->field_search_constraints[0]['value']);
        $document->ss_tow_sc_hash = db_result(db_query("

      SELECT
        ss.field_sc_hash_value

        FROM
          {content_type_saved_search} ss

        WHERE
          ss.nid = %d

    ", $node->nid));
        $document->ss_body = $ss->body;
        $document->ss_teaser = ($ss->teaser == '' || $ss->teaser == NULL) ? node_teaser($ss->body) : $ss->teaser;
    }
}

/* * **************************************************************************************************
 * Hook handlers
 * ************************************************************************************************** */

/**
 * Part of hook_apachesolr_modify_query() implementation
 * for the case of outer search (over all datasets).
 */
function tow_apachesolr_dataset_modify_query(&$query, &$params) {

    $categories_vid = _tow_apachesolr_taxonomy_get_vid('Dataset tags');
    $tags_vid = _tow_apachesolr_taxonomy_get_vid('Dataset categories');

    // Extend params.
    $params['fl'] .= ',sis_tow_amount_tables';
    $params['fl'] .= ',sis_tow_amount_records';
    $params['fl'] .= ',sis_tow_amount_saved_searches';
    $params['fl'] .= ',sis_tow_amount_debated';
    $params['fl'] .= ',sis_tow_count_views';
    $params['fl'] .= ',sis_cck_field_access_type';
    $params['fl'] .= ',im_vid_' . $categories_vid;
    $params['fl'] .= ',im_vid_' . $tags_vid;
    $params['fl'] .= ',ss_teaser';
    $params['fl'] .= ',ss_picture';
    $params['fl'] .= ',im_tow_user_access_private';

    // Add filters.
    $query->add_filter('type', 'dataset');

    global $_tow_user;
    $is_admin = (in_array('administrator', $_tow_user->roles) || ($_tow_user->uid == 1));
    if (!$is_admin) {
        $subquery = apachesolr_drupal_query();
        $subquery->add_filter('uid', $_tow_user->uid);
        $subquery->add_filter('sis_cck_field_access_type', 0);
        $subquery->add_filter('sis_cck_field_access_type', 1);
        $subquery->add_filter('im_tow_user_access_private', $_tow_user->uid);
        $query->add_subquery($subquery, 'OR');
    }

    $params['fl'] .= ',tid';
    $params['stats'] = 'true';
    $params['hl'] = 'false';
    $params['rows'] = 30;
    $params['facet.limit'] = 999999;
    $params['facet.missing'] = 'true';
    $params['facet.field'][] = '{!ex=' . 'im_vid_' . $categories_vid . '}' . 'im_vid_' . $categories_vid;
    $params['facet.field'][] = '{!ex=' . 'im_vid_' . $tags_vid . '}' . 'im_vid_' . $tags_vid;
    $params['facet.field'][] = '{!ex=' . 'sis_cck_field_access_type' . '}' . 'sis_cck_field_access_type';

    $current_tab = tow_search_outer_get_homepage_tab();
    switch ($current_tab) {

        // Sort by time created descending for the newest datasets.
        case 'newest':
            $query->set_solrsort('created', 'desc');
            break;

        // Sort by #views for the most popular datasets.
        case 'popular':
            $query->set_available_sort('sis_tow_count_views', array('title' => t('Most viewed'), 'default' => 'asc'));
            $query->set_solrsort('sis_tow_count_views', 'desc');
            break;
        
        // Sort by (# searches + # comments + # search votes) for the most debated datasets.
        case 'debated':
            $query->set_available_sort('sis_tow_amount_debated', array('title' => t('Most debated'), 'default' => 'asc'));
            $query->set_solrsort('sis_tow_amount_debated', 'desc');
            break;

        // Do nothing by default.
        default:
            break;
    }
}

/**
 * Part of hook_apachesolr_modify_query() implementation
 * for the case of outer search (by user).
 */
function tow_apachesolr_dataset_user_modify_query(&$query, &$params) {
    $viewed_user = user_load(arg(1));

    $categories_vid = _tow_apachesolr_taxonomy_get_vid('Dataset tags');
    $tags_vid = _tow_apachesolr_taxonomy_get_vid('Dataset categories');

    // Extend params.
    $params['fl'] .= ',sis_tow_amount_tables';
    $params['fl'] .= ',sis_tow_amount_records';
    $params['fl'] .= ',sis_tow_amount_saved_searches';
    $params['fl'] .= ',sis_tow_count_views';
    $params['fl'] .= ',sis_cck_field_access_type';
    $params['fl'] .= ',im_vid_' . $categories_vid;
    $params['fl'] .= ',im_vid_' . $tags_vid;
    $params['fl'] .= ',ss_teaser';
    $params['fl'] .= ',im_tow_user_access_private';

    // Add filters.
    $query->add_filter('type', 'dataset');
    $query->add_filter('uid', $viewed_user->uid);

    global $user;
    $is_admin = (in_array('administrator', $user->roles) || ($user->uid == 1));
    if (($user->uid != $viewed_user->uid) && !$is_admin) {
        $subquery = apachesolr_drupal_query();
        $subquery->add_filter('sis_cck_field_access_type', 0);
        $subquery->add_filter('sis_cck_field_access_type', 1);
        $subquery->add_filter('im_tow_user_access_private', $user->uid);
        $query->add_subquery($subquery, 'OR');
    }

    $query->set_solrsort('created', 'desc');
}

/**
 * Part of hook_apachesolr_process_results() implementation
 * for the case of outer search (over all datasets).
 */
function tow_apachesolr_dataset_process_results(&$results) {
    $current_tab = tow_search_outer_get_homepage_tab();

    foreach ($results as $key => $result) {

        // Add tables amount, records amount and post date.
        $results[$key]['amount_tables'] = $result['node']->sis_tow_amount_tables . t(' tables');
        $results[$key]['amount_records'] = $result['node']->sis_tow_amount_records . t(' records');
        $results[$key]['posted_date'] = t('posted ') . date('M d, Y', $result['node']->created);

        if ($current_tab != 'user') {

            // User settings.
            $account = new stdClass;
            $account->uid = $result['node']->uid;
            $account->name = $result['node']->name;
            $account->picture = $result['node']->ss_picture;

            // User points.
            $results[$key]['points'] = userpoints_get_current_points($result['node']->uid, 'all');


            // User picture.
            $results[$key]['picture'] = theme('user_picture', $account);
        } else {
            unset($results[$key]['user']);
        }

        // Teaser.
        $results[$key]['teaser'] = $result['node']->ss_teaser;

        // Get term ID.
        $tid = $result['fields']['im_vid_' . _tow_apachesolr_taxonomy_get_vid('Dataset categories')]['0'];

        // Add taxonomy.
        if (isset($tid)) {
            $results[$key]['category'] = l(taxonomy_get_term($tid)->name, '', array('query' => array('filters' => 'tid:' . $tid)));
        }
        if (isset($result['fields']['im_vid_' . _tow_apachesolr_taxonomy_get_vid('Dataset tags')]) && !is_null($result['fields']['im_vid_' . _tow_apachesolr_taxonomy_get_vid('Dataset tags')])) {
            foreach ($result['fields']['im_vid_' . _tow_apachesolr_taxonomy_get_vid('Dataset tags')] as $tid) {
                $results[$key]['tags'][] = l(taxonomy_get_term($tid)->name, '', array('query' => array('filters' => 'tid:' . $tid)));
            }
        }

        // Add access type.
        $results[$key]['access'] = tow_access_get_access_status($result['node']->sis_cck_field_access_type);

        // Add total votes.
        $total_votes = _votingapi_get_standard_results('node', $result['node']->nid);
        $results[$key]['votes'] = isset($total_votes['vote']['points']['sum']) ? $total_votes['vote']['points']['sum'] : 0;

        // Add total views.
        tow_statistics_dataset_extract($result['node']->nid);
        global $_tow_dataset_statistics;
        $results[$key]['views'] = $_tow_dataset_statistics['count'];


        // Add total saved searches.
        $results[$key]['ssearches'] = $result['node']->sis_tow_amount_saved_searches;
    }
}

/**
 * Part of hook_apachesolr_modify_query() implementation
 * for the case of inner search (within dataset).
 */
function tow_apachesolr_record_modify_query(&$query, &$params) {
    $nid = tow_search_inner_get_nid();

    $params['fl'] .= ',ss_tow_field_table';
    if (is_numeric($nid)) {
        $query->add_filter('type', 'record');
    } else {
        $query->add_filter('type', 'linked_record');

        // Check the usage of field collapsing.
        $nid_collapsed = explode('+', $nid);
        sort($nid_collapsed);
        $nid_full = array_keys(tow_search_inner_linked_build_table_tablesets($nid_collapsed[0]));
        sort($nid_full);
        $nid_diff = array_diff($nid_full, $nid_collapsed);
        $fc = (empty($nid_diff)) ? FALSE : TRUE;

        // Set grouping on field collapsing.
        if ($fc) {
            $nid = implode('+', $nid_full);
            $list = array();
            $subquery = apachesolr_drupal_query();
            foreach ($nid_collapsed as $n) {
                $k = array_search($n, $nid_full);
                $list[] = 'sis_tow_l_p_' . $k;
                $params['fl'] .= ',sis_tow_l_p_' . $k;
                $subquery->add_filter('sis_tow_l_p_' . $k, '([* TO *] NOT 0)');
            }
            $query->add_subquery($subquery, 'OR');
            
            $params['group'] = 'true';
            $params['group.func'] = _tow_apachesolr_group_function($list);
            # $params['group.field'] = 'ss_tow_field_table';
            $params['group.facet'] = 'true';
            
        }
    }

    $table_fields = tow_apachesolr_table_fields($nid, TRUE);

    $params['rows'] = 10000000;
    $query->add_filter('ss_tow_field_table', $nid);

    $params['stats'] = 'true';
    $params['facet.limit'] = 999999;
    $params['facet.missing'] = 'true';
    $params['hl'] = 'false';
    foreach ($table_fields as $index => $field) {
        $params['fl'] .= ',' . $field['name'];
        $params['facet.field'][] = '{!ex=' . $field['name'] . '}' . $field['name'];

        // Add stats.
        if ($field['type'] != 'bool' && $field['type'] != 'text' && $field['type'] != 'char') {
            $params['stats.field'][] = $field['name'];
        }

        $field_filters = $query->get_filters($field['name']);
        $subquery = apachesolr_drupal_query();
        foreach ($field_filters as $field_filter) {
            $query->remove_filter($field_filter['#name'], $field_filter['#value'], $field_filter['#exclude']);
            if ($field['type'] == 'enum') {

                // Wrap in quotes.
                $field_filter['#value'] = '"' . $field_filter['#value'] . '"';
            }
            if ($field['type'] == 'text' || $field['type'] == 'char') {

                // Wrap in quotes.
                $field_filter['#value'] = preg_replace('/--/', '\\--', $field_filter['#value']);
                $field_filter['#value'] = preg_replace('/(?<!\\\\)\+/', '--', $field_filter['#value']);
            }
            $subquery->add_filter($field['name'], $field_filter['#value'], $field_filter['#exclude']);
        }
        $query->add_subquery($subquery, 'OR');
    }
}

/**
 * Part of hook_apachesolr_process_results() implementation
 * for the case of inner search (within dataset).
 */
function tow_apachesolr_record_process_results(&$results) {

    $response = apachesolr_static_response_cache(NULL, 'tow_inner_search');
    if (isset($results[0])) {
        $table_nid = $results[0]['node']->ss_tow_field_table;
    } else {
        return NULL;
    }

    $fields = tow_apachesolr_table_fields($table_nid);
    $rows = array();
    $headers = array();

    $nid = tow_search_inner_get_nid();
    $selected_fields = (isset($_GET['selected_fields'])) ? json_decode($_GET['selected_fields'], TRUE) : array();

    // Leave only the selected fields if field selection is provided.
    if (!empty($selected_fields)) {
        foreach ($fields as $key => $field) {
            if (!in_array($field['title'] . '_' . $field['type'], $selected_fields)) {
                unset($fields[$key]);
            }
        }
    } else {
        if (!is_numeric($nid)) {

            // Check the usage of field collapsing.
            $nid_collapsed = explode('+', $nid);
            sort($nid_collapsed);
            $nid_full = array_keys(tow_search_inner_linked_build_table_tablesets($nid_collapsed[0]));
            sort($nid_full);
            $nid_diff = array_diff($nid_full, $nid_collapsed);

            // Unset unnecessary fields.
            if (!empty($nid_diff)) {
                foreach ($fields as $key => $field) {
                    $unset = TRUE;
                    foreach ($field['table'] as $ft) {
                        if (!in_array($ft['nid'], $nid_diff)) {
                            $unset = FALSE;
                            break;
                        }
                    }
                    if ($unset) {
                        unset($fields[$key]);
                    }
                }
            }
        }
    }

    foreach ($results as $key => $result) {
        $row = array();
        foreach ($fields as $field_key => $field) {
            $value = $result['node']->$field['name'];

            switch ($field['type']) {
                case 'bool':
                    $value = _tow_apachesolr_solr_to_bool($value);
                    break;

                case 'date':
                    _tow_apachesolr_solr_to_date($value);
                    break;

                case 'time':
                    _tow_apachesolr_solr_to_time($value);
                    break;

                case 'datetime':
                    _tow_apachesolr_solr_to_datetime($value);
                    break;

                case 'timestamp':
                    _tow_apachesolr_solr_to_timestamp($value);
                    break;

                case 'enum':
                    $value = $field['enum_options'][$value];

                default:
                    break;
            }

            $row[$field_key] = '<span>' . $value . '</span>';
            $headers[$field_key] = array('sTitle' => $field['title']);
        }
        $rows[$key] = array_values($row);
        $rows[$key]['nid'] = $result['node']->nid;
    }

    $results['headers'] = array_values($headers);
    $results['rows'] = $rows;
}

/**
 * Part of hook_apachesolr_modify_query() implementation
 * for the saved searches (by user).
 */
function tow_apachesolr_saved_search_user_modify_query(&$query, &$params) {
    $user = user_load(arg(1));

    // Extend params.
    $params['fl'] .= ',sis_tow_dataset_nid';
    $params['fl'] .= ',ss_tow_dataset_title';
    $params['fl'] .= ',ss_tow_dataset_path';
    $params['fl'] .= ',ss_tow_saved_search_path';
    $params['fl'] .= ',ss_body';
    $params['fl'] .= ',ss_teaser';

    // Add filters.
    $query->add_filter('type', 'saved_search');
    $query->add_filter('uid', $user->uid);

    $query->set_solrsort('created', 'desc');
}

/**
 * Part of hook_apachesolr_process_results() implementation
 * for the case of saved searches.
 */
function tow_apachesolr_saved_search_process_results(&$results) {
    global $base_url;

    $current_tab = tow_search_outer_get_homepage_tab();

    foreach ($results as $key => $result) {

        // Add post date.
        $results[$key]['posted_date'] = t('posted ') . _tow_dataset_ago($result['node']->created);

        if ($current_tab != 'user') {

            // User settings.
            $account = new stdClass;
            $account->uid = $result['node']->uid;
            $account->name = $result['node']->name;
            $account->picture = $result['node']->ss_picture;

            // User points.
            $results[$key]['points'] = userpoints_get_current_points($result['node']->uid, 'all');


            // User picture.
            $results[$key]['picture'] = theme('user_picture', $account);
        } else {
            unset($results[$key]['user']);
        }

        // Teaser.
        $results[$key]['teaser'] = $result['node']->ss_teaser;

        // Link.
        $results[$key]['link'] = $base_url . '/' . $result['node']->ss_tow_saved_search_path;

        // Dataset.
        $results[$key]['dataset_title'] = $result['node']->ss_tow_dataset_title;
        $results[$key]['dataset_link'] = $base_url . '/' . $result['node']->ss_tow_dataset_path;

        // Add total votes.
        $total_votes = _votingapi_get_standard_results('node', $result['node']->nid);
        $results[$key]['votes'] = isset($total_votes['vote']['points']['sum']) ? $total_votes['vote']['points']['sum'] : 0;
    }
}

/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Adds facet field.
 */
function tow_apachesolr_add_facet_field(&$f_fields, $field) {
    $f_fields[] = $field;
    if ($field['type'] == 'text' || $field['type'] == 'char') {
        $field['name'] = 'f_' . $field['name'];
        $f_fields[] = $field;

        $field['name'] = 'sis_tow_field_' . $field['type'] . '_' . $field['delta'] . '_words';
        $f_fields[] = $field;

        $field['name'] = 'sis_tow_field_' . $field['type'] . '_' . $field['delta'] . '_length';
        $f_fields[] = $field;
    }
}

/**
 * Get facets.
 */
function tow_apachesolr_facets() {
    $facets['sis_tow_amount_tables'] = array(
        'info' => t('Amount of tables'),
        'facet_field' => 'sis_tow_amount_tables',
    );

    $facets['sis_tow_amount_records'] = array(
        'info' => t('Amount of rows'),
        'facet_field' => 'sis_tow_amount_records',
    );

    $facets['sis_tow_amount_saved_searches'] = array(
        'info' => t('Amount of saved searches'),
        'facet_field' => 'sis_tow_amount_saved_searches',
    );

    return $facets;
}

/**
 * Function for retrieving number of results for facetted-like menus.
 * Uses analyse of single solr request.
 *
 * @param $facet_field
 *   Facet field
 * @param $values
 *   Array of options of drop-down menu in form $key => $value, where $key is value of filter.
 *   Array should has accending sorting on keys.
 * @param $is_range
 *   If TRUE, search is done through range [value TO *], strict value is searched otherwise.
 */
function tow_apachesolr_facet_menu($facet_field, $values, $is_range = FALSE) {
    $saved_response = apachesolr_static_response_cache();
    $saved_query = apachesolr_current_query();
    $query = clone $saved_query;

    $do_new_search = TRUE;
    if ($facet_field == 'im_vid_' . _tow_apachesolr_taxonomy_get_vid('Dataset categories')) {
        foreach (tow_search_outer_filter_taxonomy() as $tid => $term) {
            if ($query->has_filter('tid', $tid)) {
                $query->remove_filter('tid', $tid);
                $do_new_search = TRUE;
            }
        }
    } else {
        foreach ($query->get_filters() as $set_filter) {
            if ($set_filter['#name'] == $facet_field) {
                $query->remove_filter($facet_field);
                $do_new_search = TRUE;
            }
        }
    }

    if ($do_new_search) {
        $params = apachesolr_search_basic_params($query);
        apachesolr_search_add_facet_params($params, $query);
        apachesolr_search_add_boost_params($params, $query, apachesolr_get_solr());

        $params['facet.limit'] = 999999;
        list($final_query, $response) = apachesolr_do_query('apachesolr_search', $query, $params);
        if (empty($response)) {
            return;
        }

        apachesolr_static_response_cache($saved_response);
        apachesolr_current_query($saved_query);
    } else {
        $response = clone $saved_response;
    }

    if ($is_range) {
        $sorted_keys = array_flip($values);

        $facet_counts = (array) $response->facet_counts->facet_fields->$facet_field;
        if (!empty($facet_counts)) {
            $current_key = array_pop($sorted_keys);

            krsort($facet_counts);
            $counts = 0;

            foreach ($facet_counts as $facet_value => $facet_count) {
                while ($current_key > $facet_value) {
                    $values[$current_key] .= ' (' . $counts . ')';
                    $current_key = array_pop($sorted_keys);
                }
                $counts += $facet_count;
            }

            while (!empty($sorted_keys)) {
                $values[$current_key] .= ' (' . $counts . ')';
                $current_key = array_pop($sorted_keys);
            }

            $values[$current_key] .= ' (' . $counts . ')';
        } else {
            foreach ($values as $key => $value) {
                $values[$key] .= ' (0)';
            }
        }
    } else {
        foreach ($values as $key => $value) {
            if ($key === 'any') {
                $num = $response->response->numFound;
            } else {
                $num = $response->facet_counts->facet_fields->$facet_field->$key;
            }

            if (!isset($num)) {
                $num = 0;
            }
            $values[$key] .= ' (' . $num . ')';
        }
    }

    return $values;
}

/**
 * Returns number of results.
 */
function tow_apachesolr_facet_menu_item_count($facet_field, $value) {
    $saved_response = apachesolr_static_response_cache();
    $saved_query = apachesolr_current_query();
    $query = clone $saved_query;

    $query->remove_filter($facet_field);
    $query->add_filter($facet_field, $value);

    $params = apachesolr_search_basic_params($query);
    apachesolr_search_add_facet_params($params, $query);
    apachesolr_search_add_boost_params($params, $query, apachesolr_get_solr());
    list($final_query, $response) = apachesolr_do_query('apachesolr_search', $query, $params);
    if (empty($response)) {
        return;
    }

    apachesolr_static_response_cache($saved_response);
    apachesolr_current_query($saved_query);

    return $response->response->numFound;
}

/**
 * Function for retrieving number of results for facetted-like menus for range of values.
 * Uses solr requests for each facet value.
 */
function tow_apachesolr_facet_menu_solr_range($facet_field, $values) {
    foreach ($values as $key => $value) {
        $num = tow_apachesolr_facet_menu_item_count($facet_field, '[' . $key . ' TO *]');
        if (!isset($num)) {
            $num = 0;
        }
        $values[$key] .= ' (' . $num . ')';
    }
    return $values;
}

/**
 * Returns prefix of ApacheSolr dynamic single-valued field
 * corresponding to ToW field of table record.
 * */
function tow_apachesolr_field_type($type) {
    switch ($type) {
        case 'int':
        case 'timestamp':
        case 'enum':
            return 'sis'; //integer

        case 'code':
            return 'ss'; //string

        case 'float':
            return 'fs'; //sfloat

        case 'bool':
            return 'bs'; //boolean
            break;

        case 'date':
        case 'datetime':
        case 'time':
            return 'ds'; //date

        case 'char':
        case 'text':
        default:
            return 'sts'; //text (no break)
    }
}

/**
 * Returns the name of the field.
 */
function tow_apachesolr_field_name($field_type, $delta) {
    return tow_apachesolr_field_type($field_type) . '_tow_field_' . $field_type . '_' . $delta;
}

/**
 * Returns array of fields of table with given nid, name, type, title, delta.
 */
function tow_apachesolr_table_fields($nid, $add_facet_string_fields = FALSE) {
    static $current_nid = NULL;
    static $fields = array();
    static $f_fields = array();

    if ($nid != $current_nid || empty($fields)) {
        $fields = array();
        $f_fields = array();
        $current_nid = $nid;
        if (is_numeric($nid)) {
            list($fields, $f_fields) = _tow_apachesolr_separate_table_fields($nid);
        }
        // NID is not numeric - linked table.
        else {
            if (!is_array($nid)) {
                $nid = str_replace(' ', '+', $nid);
                $nid = explode('+', $nid);
            }
            sort($nid);
            $deltas = array();
            $added_indexes = array();
            $key = 0;
            foreach ($nid as $table_nid) {
                list($table_fields, $table_f_fields) = _tow_apachesolr_separate_table_fields($table_nid);
                foreach ($table_fields as $field) {
                    $add_to_list = TRUE;
                    if ($field['index']) {
                        if (!isset($added_indexes[$field['type']])) {
                            $added_indexes[$field['type']] = array('titles' => array(), 'keys' => array());
                        }
                        $duplicate = array_search($field['title'], $added_indexes[$field['type']]['titles']);
                        if ($duplicate === FALSE) {
                            $added_indexes[$field['type']]['titles'][] = $field['title'];
                            $added_indexes[$field['type']]['keys'][] = $key;
                        } else {

                            // Key of added index field.
                            $key_ = $added_indexes[$field['type']]['keys'][$duplicate];
                            $add_to_list = FALSE;
                            $fields[$key_]['table'][] = array(
                                'nid' => $table_nid,
                                'delta' => $field['delta']
                            );
                        }
                    }
                    if ($add_to_list) {

                        // Field is either not index or is not index duplicate.
                        if (isset($deltas[$field['type']])) {
                            $deltas[$field['type']]++;
                        } else {
                            $deltas[$field['type']] = 0;
                        }
                        $field['table'] = array();
                        $field['table'][0] = array(
                            'nid' => $table_nid,
                            'delta' => $field['delta']
                        );
                        $field['delta'] = $deltas[$field['type']];
                        $field['name'] = tow_apachesolr_field_name($field['type'], $field['delta']);
                        $fields[] = $field;
                        tow_apachesolr_add_facet_field($f_fields, $field);
                        $key++;
                    }
                }
            }
        }
    }

    return $add_facet_string_fields ? $f_fields : $fields;
}

/* * **************************************************************************************************
 * Helper functions
 * ************************************************************************************************** */

/**
 * Helper functions to prepare info for the solr indexing.
 */
function _tow_apachesolr_add_value_to_doc(&$document, $field_type, $delta, $value) {
    switch ($field_type) {

        case 'time':
            _tow_apachesolr_time_to_solr($value);
            break;

        case 'date':
            _tow_apachesolr_date_to_solr($value);
            break;

        case 'datetime':
            _tow_apachesolr_datetime_to_solr($value);
            break;

        case 'char':
        case 'text':
        case 'code':
            $words_field_name = 'sis_tow_field_' . $field_type . '_' . $delta . '_words';
            $length_field_name = 'sis_tow_field_' . $field_type . '_' . $delta . '_length';
            $words = str_word_count($value);
            $length = strlen($value);
            $document->$words_field_name = $words;
            $document->$length_field_name = $length;
            break;

        default:
            break;
    }

    $apachesolr_field_name = tow_apachesolr_field_name($field_type, $delta);
    $document->$apachesolr_field_name = $value;
}

/**
 * Helper function converting date from usual format yyyy-mm-dd
 * to solr datetime format yyyy-mm-ddThh:mm:ssZ.
 */
function _tow_apachesolr_date_to_solr(&$value) {
    $value = date('Y-m-d', strtotime($value)) . 'T00:00:00Z';
}

/**
 * Helper function converting datetime from usual format yyyy-mm-dd hh:mm:ss
 * to solr datetime format yyyy-mm-ddThh:mm:ssZ.
 */
function _tow_apachesolr_datetime_to_solr(&$value) {
    $int_time = strtotime($value);
    $value = date('Y-m-d', $int_time) . 'T' . date('H:i:s', $int_time) . 'Z';
}

/**
 * Returns a grouping function string representation for a given subtableset.
 */
function _tow_apachesolr_group_function($list = array()) {

    // Define the base and degree.
    $degree = 0;
    $base = variable_get('tow_max_records_in_table', 920);

    $result = '0';
    if (!isset($list) || (count($list) == 0)) {
        $result = '0';
    } else {
        while (!empty($list)) {
            $field = array_shift($list);
            $result = 'add(product(' . $field . ',pow(' . $base . ',' . $degree . ')),' . $result . ')';
            $degree++;
        }
    }

    return $result;
}

/**
 * Returns array of fields of separate table with given nid, name, type, title, delta.
 */
function _tow_apachesolr_separate_table_fields($nid) {
    $fields = array();
    $f_fields = array();
    $table_node = node_load($nid, NULL, TRUE);

    if ($table_node->type == 'table') {
        $index = 0;
        $weights_array = unserialize($table_node->field_weights[0]['value']);
        $indexes_array = unserialize($table_node->field_indexes[0]['value']);

        foreach (tow_possible_fields() as $field_type) {
            $field_title_name = 'field_title_' . $field_type;

            foreach ($table_node->$field_title_name as $delta => $field_title) {
                if ($field_title['value'] != NULL) {
                    if ($field_type == 'enum') {
                        $enum_options = unserialize($table_node->field_enum_options[$delta]['value']);
                    } else {
                        $enum_options = NULL;
                    }
                    $x = $weights_array[$index];
                    $is_index = $indexes_array[$x];
                    $field = array(
                        'name' => tow_apachesolr_field_name($field_type, $delta),
                        'type' => $field_type,
                        'title' => $field_title['value'],
                        'delta' => $delta,
                        'enum_options' => $enum_options,
                        'index' => $is_index,
                        'table_nid' => $table_node->nid,
                        'unit' => $table_node->field_units[$x]['value'],
                    );
                    $fields[$weights_array[$index]] = $field;
                    tow_apachesolr_add_facet_field($f_fields, $field);
                    $index++;
                }
            }
        }
        ksort($fields);
    } elseif ($table_node->type != 'dataset') {
        drupal_not_found();
        die();
    }

    return array($fields, $f_fields);
}

/**
 * Helper function converting timestamp from solr datetime format yyyy-mm-ddThh:mm:ssZ
 * to usual int timestamp.
 */
function _tow_apachesolr_solr_to_bool(&$value) {
    if ($value == '1') {
        return 'true';
    }
    if ($value == '0') {
        return 'false';
    } else {
        return NULL;
    }
}

/**
 * Helper function converting date from solr datetime format yyyy-mm-ddThh:mm:ssZ
 * to usual format yyyy-mm-dd.
 */
function _tow_apachesolr_solr_to_date(&$value) {
    $value = str_replace('T', ' ', $value);
    $value = str_replace('Z', '', $value);
    $value = date('Y-m-d', strtotime($value));
}

/**
 * Helper function converting datetime from solr datetime format yyyy-mm-ddThh:mm:ssZ
 * to usual format yyyy-mm-dd hh:mm:ss.
 */
function _tow_apachesolr_solr_to_datetime(&$value) {
    $value = str_replace('T', ' ', $value);
    $value = str_replace('Z', '', $value);
    $value = date('Y-m-d H:i:s', strtotime($value));
}

/**
 * Helper function converting time from solr datetime format yyyy-mm-ddThh:mm:ssZ
 * to usual format hh:mm:ss.
 */
function _tow_apachesolr_solr_to_time(&$value) {
    $value = str_replace('T', ' ', $value);
    $value = str_replace('Z', '', $value);
    $value = date('H:i:s', strtotime($value));
}

/**
 * Helper function converting timestamp from solr datetime format yyyy-mm-ddThh:mm:ssZ
 * to usual int timestamp.
 */
function _tow_apachesolr_solr_to_timestamp(&$value) {
    $value = gmdate('Y-m-d H:i:s', $value);
}

/**
 * Returns vocabulary ID by name.
 */
function _tow_apachesolr_taxonomy_get_vid($vocabulary_name) {
    $query = db_query("SELECT v.vid AS vid FROM {vocabulary} v WHERE v.name = '%s'", $vocabulary_name);

    $vocabularies = array();
    while ($record = db_fetch_object($query)) {
        $vocabularies[] = $record->vid;
    }

    return (empty($vocabularies) ? 0 : $vocabularies[0]);
}

/**
 * Helper function converting time from usual format hh:mm:ss
 * to solr datetime format yyyy-mm-ddThh:mm:ssZ.
 */
function _tow_apachesolr_time_to_solr(&$value) {
    $value = '0001-01-01T' . date('H:i:s', strtotime($value)) . 'Z';
}

/**
 * Helper function converting timestamp from timestamp
 * to solr datetime format yyyy-mm-ddThh:mm:ssZ.
 */
function _tow_apachesolr_timestamp_to_solr(&$value) {
    $value = gmdate('Y-m-d', $value) . 'T' . gmdate('H:i:s', $value) . 'Z';
}
