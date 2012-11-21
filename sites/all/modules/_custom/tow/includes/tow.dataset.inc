<?php

/**
 * @file
 *
 * Dataset API.
 */
/* * **************************************************************************************************
 * Hook handlers
 * ************************************************************************************************** */

/**
 * Returns "Dataset badge" block.
 */
function tow_dataset_badge_block() {

    // No block in case of not a dataset.
    global $_tow_dataset;
    if (!$_tow_dataset) {
        return;
    }

    // Return if there is no info on tables count.
    global $_tow_dataset_count_tables;
    if (!isset($_tow_dataset_count_tables)) {
        return;
    }

    // The same with the records.
    global $_tow_dataset_count_records;
    if (!isset($_tow_dataset_count_records)) {
        $_tow_dataset_count_records = 0;
    }

    // Prepare theming parametres.
    $upoints = userpoints_get_current_points($_tow_dataset->uid, 'all');
    $posted_by = t('Posted by ') . l($_tow_dataset->name, 'user/' . $_tow_dataset->uid) . ' (' . $upoints . ')';
    $count = $_tow_dataset_count_tables . t(' tables') . ' ' . $_tow_dataset_count_records . t(' rows');
    $added = t('Added ') . _tow_dataset_ago($_tow_dataset->created);
    $updated = t('Updated ') . _tow_dataset_ago($_tow_dataset->changed);
    $access = t('Access is <span>') . tow_access_get_access_status($_tow_dataset->field_access_type[0]['value']) . '</span>';
    global $_tow_dataset_statistics;
    $visits = $_tow_dataset_statistics['count'] . ' ' . t('views');
    global $user;
    if ($user->uid == $_tow_dataset->uid) {
        $extrainfo = t('You are an administrator of this dataset');
    } elseif ($_tow_dataset->field_access_type[0]['value'] == 1) {
        $extrainfo = t('Request access to this dataset');
    }

    // Save the content of the block.
    $block['content'] = '';
    $block['content'] .= theme('tow_theme_dataset_badge', $posted_by, $count, NULL, $added, $updated, $access, $visits);

    // Params for the user picture.
    $account = new stdClass;
    $account->uid = $_tow_dataset->uid;
    $account->name = $_tow_dataset->name;
    $account->picture = $_tow_dataset->picture;

    // Add the picture.
    $block['content'] .= theme('user_picture', $account) . '<div class="user-dataset">' . $extrainfo . '</div>';

    return $block;
}

/**
 * Returns "Share" block.
 */
function tow_dataset_share_block() {

    // No block in case of not a dataset.
    global $_tow_dataset;
    if (!$_tow_dataset) {
        return;
    }

    // Return if there is no info on tables count.
    global $_tow_dataset_count_tables;
    if (!isset($_tow_dataset_count_tables)) {
        return;
    }

    // The same with the records.
    global $_tow_dataset_count_records;
    if (!isset($_tow_dataset_count_records)) {
        $_tow_dataset_count_records = 0;
    }

    // Prepare theming parametres.
    $path = isset($_GET['q']) ? $_GET['q'] : '<front>';
    $link = url($path, array('absolute' => TRUE));
    $permalink = '<div class="permalink">' . t('Permalink ') . '</div>' . '<input value="' . $link . '"></input>';
    $embedding = t('Embedding options are available from ') . l('search', 'search_dataset/' . $_tow_dataset->nid) . ' and ' . l('browse', 'node/' . $_tow_dataset->nid) . '.';

    // Save title and content of the block.
    $block['title'] = 'Share';
    $block['content'] = theme('tow_theme_dataset_share');
    $block['content'] .= '<p>' . $permalink . '</p>';
    $block['content'] .= '<p>' . $embedding . '</p>';

    return $block;
}

/**
 * Returns "Dataset votes" block.
 */
function tow_dataset_votes_block() {

    // No block in case of not a dataset.
    global $_tow_dataset;
    $viewed_dataset = node_load(arg(1));
    if ($viewed_dataset->type != 'dataset') {
        return;
    }

    // Save the content of the block.
    $block['content'] = '';
    //Output voting widget
    $tag = variable_get('vud_tag', 'vote');
    $widget = variable_get('vud_node_widget', 'plain');
    $block['content'] .= theme('vud_widget', $viewed_dataset->nid, 'node', $tag, $widget);
    //Output bookmark star
    $flag = flag_get_flag('bookmarks');
    if ($flag) {
        $block['content'] .= '<div class="starflag">' . flag_create_link('bookmarks', $_tow_dataset->nid) . '<b>' . $flag->get_count($_tow_dataset->nid) . '</b></div>';
    } else {
        $block['content'] .= '<div class="starflag">' . flag_create_link('bookmarks', $_tow_dataset->nid) . '<b>0</b></div>';
    }

    return $block;
}

/**
 * 'Create Table' block.
 * Part of hook_block() implementation.
 */
function tow_dataset_create_table_block() {
    global $_tow_dataset;
    global $user;

    // Return on missing dataset.
    if (!$_tow_dataset) {
        return;
    }

    // Set a creation block for table if the user is the owner of the dataset.
    if ($user->uid === $_tow_dataset->uid) {
        $args = arg();
        if ($args[0] == 'dataset' && !isset($args[2])) {
            $block['content'] = drupal_get_form('tow_dataset_create_table_form');
        }
    }

    return $block;
}

/**
 * Returns "Dataset stats" block.
 */
function tow_dataset_stats_block() {

    // Stop if it's not a dataset...
    global $_tow_dataset;
    if (!$_tow_dataset) {
        return;
    }

    // ...or there are no tables.
    global $_tow_dataset_count_tables;
    if (!isset($_tow_dataset_count_tables)) {
        return;
    }

    $block['title'] = t('Quick stats');

    // Set $_tow_dataset_statistics.
    global $_tow_dataset_statistics;
    if (!$_tow_dataset_statistics) {
        $_tow_dataset_statistics = array(
            'count' => 0,
            'timestamp' => t('never')
        );
    } else {
        $_tow_dataset_statistics['timestamp'] = _tow_dataset_ago($_tow_dataset_statistics['timestamp']);
    }

    $block['content'] = theme('table', array(), array(array(t('Views'), $_tow_dataset_statistics['count']), array(t('Last view'), $_tow_dataset_statistics['timestamp'])));

    return $block;
}

/**
 * Part of hook_form_alter() implementation.
 */
function tow_dataset_form_alter(&$form, &$form_state) {
    $form['buttons']['submit']['#value'] = t('Save and import data');
    $form['#validate'][] = 'tow_dataset_dont_validate_cancel';
    $form['#submit'][] = 'tow_dataset_submit_redirect';
    
    //Set breadcrumbs
    if (arg(1) == 'add'){
        $bc[] = l('Data', '<front>');
        $bc[] .= t('Create dataset');
    }
    drupal_set_breadcrumb($bc);
}

/* * **************************************************************************************************
 * Forms
 * ************************************************************************************************** */

/**
 * 'Create Table' form.
 */
function tow_dataset_create_table_form() {

    $form = array();

    // Fieldset wrapper.
    $form['create_table'] = array(
        '#type' => 'fieldset',
        '#title' => 'Create a new table',
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
    );

    // Table name.
    $form['create_table']['table_name'] = array(
        '#type' => 'textfield',
        '#title' => 'Name',
    );

    // Number of fields.
    $form['create_table']['count_fields'] = array(
        '#type' => 'textfield',
        '#title' => 'Number of fields',
        '#size' => 2
    );

    // Submit button.
    $form['create_table']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Go')
    );

    return $form;
}

/**
 * Returns maintenance confirmation form.
 */
function tow_dataset_maintenance_confirm(&$form_state, $nid) {
    global $_tow_dataset;
    global $user;

    // Error 404 if not a dataset.
    $_tow_dataset = node_load($nid);
    if ($_tow_dataset->type != 'dataset') {
        drupal_not_found();
        die();
    }

    // Error 403 if the user has no permission to access the dataset.
    if ($user->uid != $_tow_dataset->uid && !user_access('edit any dataset')) {
        drupal_access_denied();
        die();
    }

    $form = array();

    $form['dataset'] = array(
        '#type' => 'value',
        '#value' => $_tow_dataset,
    );

    // Messaging.
    if ($_tow_dataset->status) {
        $message = 'Are you sure you want to put %title to maintenance mode?';
    } else {
        $message = 'Now %title is in maintenance mode. Do you want to publish it?';
    }

    return confirm_form($form, t($message, array('%title' => $_tow_dataset->title)), 'node/' . $_tow_dataset->nid, '', t('Yes'), t('Cancel'));
}

/* * **************************************************************************************************
 * Callback functions
 * ************************************************************************************************** */

/**
 * Submission callback for 'Create Table' form.
 * Store values in $_SESSION.
 * Redirect to node/table/add.
 */
function tow_dataset_create_table_form_submit($form, &$form_state) {
    $args = arg();

    // $_SESSION.
    $_SESSION['table_definition'] = array(
        'dataset' => $args[1],
        'table_name' => $form_state['values']['table_name'],
        'count_fields' => $form_state['values']['count_fields']
    );

    // Redirect.
    drupal_goto('node/add/table', 'destination=dataset/' . $args[1]);
}

/**
 * Performs form validation during table adding on the dataset screen.
 */
function tow_dataset_create_table_form_validate($form, &$form_state) {
    $error = FALSE;
    $err_field_names = array();
    $err_msgs = array();
    global $_tow_dataset;
    $form_state['values']['table_name'] = trim($form_state['values']['table_name']);

    // Empty table title.
    if ($form_state['values']['table_name'] == '') {
        $error = TRUE;
        $err_field_names[] = "table_name";
        $err_msgs[] = t('Table title must neither be empty nor consist of nothing but whitespaces.');
    }

    // Unique table title.
    $table_titles_res = db_query("

    SELECT
      n.title AS title

      FROM
        {node} n

      WHERE
        n.nid IN (

          SELECT
            ctt.nid AS nid

            FROM
              {content_type_table} ctt

            WHERE
              ctt.field_dataset_value = %d
        )

  ", $_tow_dataset->nid);
    $table_titles = array();
    while ($rs = db_fetch_array($table_titles_res)) {
        $table_titles[] = strtolower($rs['title']);
    }

    if (in_array(strtolower($form_state['values']['table_name']), $table_titles)) {
        $error = TRUE;
        $err_field_names[] = "table_name";
        $err_msgs[] = t('The table with this title already exists.');
    }

    // Max and min fields check.
    if (($form_state['values']['count_fields'] > variable_get('tow_max_fields_in_table', 20)) || ($form_state['values']['count_fields'] < variable_get('tow_min_fields_in_table', 1))) {
        $error = TRUE;
        $err_field_names[] = "count_fields";
        $err_msgs[] = t("The number of fields must be greater than " . variable_get('tow_min_fields_in_table', 1) . " and less than " . variable_get('tow_max_fields_in_table', 20) . ".");
    }

    // Showing error messages.
    _tow_show_validation_error($error, $err_field_names, $err_msgs);
}

/**
 * When "Cancel" button is clicked, redirect to dataset summary or to data page without error messages.
 */
function tow_dataset_dont_validate_cancel($form, &$form_state) {
    if ($form_state['clicked_button']['#value'] == 'Cancel') {
        unset($_SESSION['messages']['error']);

        // Go to dataset summary page if the Node ID is defined.
        if ($form_state['values']['nid']) {
            drupal_goto('node/' . $form_state['values']['nid']);
        }

        // Go to data page otherwise.
        else {
            drupal_goto('data');
        }
    }
}

/**
 * Submission callback for maintenance confirmation form.
 * Publishes dataset if it is unpublished and vice-versa.
 */
function tow_dataset_maintenance_confirm_submit($form, &$form_state) {
    $dataset = $form_state['values']['dataset'];

    if ($dataset->status) {
        $dataset->status = 0;
    } else {
        $dataset->status = 1;
    }

    try {
        node_save($dataset);
    } catch (Exception $e) {
        watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
    }

    // Recursively perform the same action with tables and records.
    tow_maintanance_mode($dataset->status, $dataset->nid, 'dataset');
    $form_state['redirect'] = 'node/' . $dataset->nid;
}

function tow_dataset_submit_redirect($form, &$form_state) {
    $dataset = (object) $form_state['values'];

    try {
        node_save($dataset);
    } catch (Exception $e) {
        watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
    }

    drupal_goto('import/' . $dataset->nid);
}

/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Update dataset hash.
 */
function tow_dataset_hash_update($nid) {

    // Initialize hash.
    $hash = '';

    // Define tables, fields and aliases needed within query.
    $params = array(
        array(
            'table' => 'node_revisions',
            'field' => 'body',
            'alias' => 'table_descriptions'
        ),
        array(
            'table' => 'tow_field_info',
            'field' => 'description',
            'alias' => 'field_descriptions'
        ),
        array(
            'table' => 'node',
            'field' => 'title',
            'alias' => 'table_names'
        ),
        array(
            'table' => 'content_field_title_int',
            'field' => 'field_title_int_value',
            'alias' => 'int_fields'
        ),
        array(
            'table' => 'content_field_title_char',
            'field' => 'field_title_char_value',
            'alias' => 'char_fields'
        ),
        array(
            'table' => 'content_field_title_float',
            'field' => 'field_title_float_value',
            'alias' => 'float_fields'
        ),
        array(
            'table' => 'content_field_title_bool',
            'field' => 'field_title_bool_value',
            'alias' => 'bool_fields'
        ),
        array(
            'table' => 'content_field_title_text',
            'field' => 'field_title_text_value',
            'alias' => 'text_fields'
        ),
        array(
            'table' => 'content_field_title_enum',
            'field' => 'field_title_enum_value',
            'alias' => 'enum_fields'
        ),
        array(
            'table' => 'content_field_title_date',
            'field' => 'field_title_date_value',
            'alias' => 'date_fields'
        ),
        array(
            'table' => 'content_field_title_datetime',
            'field' => 'field_title_datetime_value',
            'alias' => 'datetime_fields'
        ),
        array(
            'table' => 'content_field_title_time',
            'field' => 'field_title_time_value',
            'alias' => 'time_fields'
        ),
        array(
            'table' => 'content_field_title_timestamp',
            'field' => 'field_title_timestamp_value',
            'alias' => 'timestamp_fields'
        ),
        array(
            'table' => 'content_field_title_code',
            'field' => 'field_title_code_value',
            'alias' => 'code_fields'
        ),
    );

    // Extend hash with the results received while requesting.
    foreach ($params as $p) {
        $query = "
      SELECT
        t1.%s AS %s

        FROM
            {%s} t1
          INNER JOIN
            {content_type_table} t2
            ON
              t1.nid = t2.nid

        WHERE
          t2.field_dataset_value = %d
    ";
        $query = db_query($query, $p['field'], $p['alias'], $p['table'], $nid);
        while ($result = db_fetch_object($query)) {
            $hash .= $result->{$p['alias']} . ' ';
        }
    }
    $hash = trim($hash);

    // Update dataset's hash.
    $query = "

    UPDATE {content_type_dataset}

      SET
        field_hash_value = '%s'

      WHERE
        nid = %d

  ";
    db_query($query, $hash, $nid);
}

/**
 * When node is viewed by user, hide dataset hash. Don't hide it for solr.
 */
function tow_dataset_hide_hash(&$node) {

    if (tow_access_bypass_solr()) {
        return;
    } else {
        unset($node->content['field_hash']);
    }
}

/**
 * Prepare two global variables for dataset badge block.
 */
function tow_dataset_prepare_badge_block($node, $count_tables) {

    global $_tow_dataset_count_tables;
    $_tow_dataset_count_tables = $count_tables;

    global $_tow_dataset_count_records;
    $_tow_dataset_count_records = db_result(db_query("

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
}

/**
 * Extracts and prepares info for dataset summary page.
 */
function tow_dataset_summary(&$node) {

    global $user;

    // For administer view only.
    if ($user->uid == $node->uid || user_access('edit any table')) {

        // Edit links and administration tasks.
        $node->summary['edit_description'] = l('edit description', 'node/' . $node->nid . '/edit');
        $node->summary['edit_categories'] = l('edit categories', 'node/' . $node->nid . '/edit');
        $node->summary['admin_tasks'] = _tow_dataset_admin_tasks($node);
    }

    // For user view if access is by request.
    elseif ($node->field_access_type[0]['value'] == 1) {
        $node->summary['access_widget'] = tow_access_request_widget($node->nid, $user->uid);
    }

    // Extract and show names of tables from the dataset.
    $table_titles_res = db_query("

    SELECT
      t.nid AS nid,
      n.title AS title

      FROM
          {content_type_table} t
        LEFT JOIN
          {node} n
          ON
            t.nid = n.nid

      WHERE
        t.field_dataset_value = %d

  ", $node->nid);

    // Originally, $_tow_grey_pma_menu flag is used to tell tow_pma_menu to make links like "import", "browse" not clicable when access is limited.
    // So here we use this flag to know if we should show links to tables and table contents.
    global $_tow_grey_pma_menu;
    if ($_tow_grey_pma_menu) {
        while ($rs = db_fetch_array($table_titles_res)) {
            $tables_list[] = $rs['title'];
        }
    } else {
        while ($rs = db_fetch_array($table_titles_res)) {
            $tables_list[] = l($rs['title'], 'table/' . $rs['nid']);
            $table_nids[] = $rs['nid'];
        }
    }

    $i = 0;
    // Create table samples.
    if (!empty($tables_list)) {

        $node->summary['tables'] = implode(', ', $tables_list);

        // Get NIDs and count of records of two random tables that contain records.
        if (!$_tow_grey_pma_menu) {
            shuffle($table_nids);
            foreach ($table_nids as $tn) {

                $count = db_result(db_query("

            SELECT
              COUNT(*) AS c

            FROM
              {content_type_record} r

            WHERE
              r.field_table_value = %d

        ", $tn));

                if ($count) {
                    $tables[] = array(
                        'nid' => $tn,
                        'count' => $count
                    );
                    $i++;
                }
                if ($i == 2) {
                    break;
                }
            }

            $node->summary['tables_stats'][] = array(
                'title' => t('Tables in this dataset'),
                'data' => tow_dataset_tables($node->nid),
            );

            // Get 2 themed tables with random LIMIT.
            if (!empty($tables)) {

                foreach ($tables as $table) {

                    $first_record_id = rand(0, $table['count'] - 3);
                    $display_options['sort'] = array(
                        'sort_index' => 'nid',
                        'sort_order' => 'ASC',
                    );
                    $display_options['limits'] = array(
                        'first_elem_no' => $first_record_id,
                        'total_elements' => 3,
                    );
                    /* $display_options += array(
                      'full_view' => TRUE,
                      'paged_view' => FALSE,
                      ); */

                    $table_node = node_load($table['nid']);

                    $node->summary['view_table'][] = array(
                        'title' => l($table_node->title, 'table/' . $table['nid']) . t(' table'),
                        'table' => tow_table($table_node, $display_options),
                    );
                }
            }
        }

        // List of saved searches.
        // Get info on the saved searches.
        $ss_results = db_query("

      SELECT
        ctss.nid AS nid,
        n.title AS title,
        ctss.field_search_constraints_value AS field_search_constraints_value

        FROM
            {content_type_saved_search} ctss
          LEFT JOIN
            {node} n
            ON
              n.nid = ctss.nid

        WHERE
          ctss.field_dataset_nid_value = %d

    ", $node->nid);

        $ss_qty = 0;
        $ss_list = array();

        while ($ss_res = db_fetch_array($ss_results)) {

            $ss_qty++;

            // Count saved searches views.
            $ss_count = db_result(db_query("

        SELECT
          tnvc.count AS count

          FROM
            {tow_node_views_counter} tnvc

          WHERE
            tnvc.nid = %d

      ", $ss_res['nid']));

            // Limit list of titles size.
            if ($ss_qty <= 4) {
                if ($ss_count) {
                    $ss_list[] = l($ss_res['title'], 'node/' . $ss_res['nid']) . ' (' . $ss_count . ')'; //'search_dataset/' . $node->nid . '#?filters=' . $ss_res['field_search_constraints_value']);
                } else {
                    $ss_list[] = l($ss_res['title'], 'node/' . $ss_res['nid']) . ' (0)';
                }
            }
        };

        // Put info into 'saved_searches' block of the summary.
        if ($ss_qty) {
            $node->summary['saved_searches']['link_to_full_list'] = l("see all " . $ss_qty, 'search_dataset/' . $node->nid);
            $node->summary['saved_searches']['short_list'] = theme_item_list($ss_list, NULL, 'ul');
        }
    }

    tow_dataset_prepare_badge_block($node, count($tables_list));
}

/**
 * Returns formatted table of dataset tables.
 */
function tow_dataset_tables($dataset_nid) {

    // Headers.
    $headers = array(
        array(
            'data' => t('Name')
        ),
        array(
            'data' => t('Records')
        ),
        array(
            'data' => t('Fields')
        ),
        array(
            'data' => t('Keys')
        ),
        array(
            'data' => t('Description')
        )
    );

    // Table nids and titles
    $table_titles_res = db_query("

    SELECT
      t.nid AS nid,
      n.title AS title

      FROM
          {content_type_table} t
        LEFT JOIN
          {node} n
          ON
            t.nid = n.nid

      WHERE
        t.field_dataset_value = %d

  ", $dataset_nid);

    // For each table...
    while ($rs = db_fetch_array($table_titles_res)) {

        $row = array();

        // Title with link.
        $row[] = array(
            'data' => l($rs['title'], 'table/' . $rs['nid'])
        );

        // Number of records.
        $row[] = array(
            'data' => db_result(db_query("

        SELECT
          COUNT(*) AS c

          FROM
            {content_type_record} r

          WHERE
            r.field_table_value = %d

      ", $rs['nid']))
        );

        // Number of fields.
        $table_node = node_load($rs['nid']);
        $table_fields = _tow_get_table_fields($table_node);
        $row[] = array(
            'data' => count($table_fields)
        );

        // Key fields.
        $key_fields = '';
        foreach ($table_fields as $table_field) {
            if ($table_field['index']) {
                $key_fields .= $table_field['title'] . ', ';
            }
        }
        $row[] = array(
            'data' => ($key_fields) ? substr($key_fields, 0, -2) : '&mdash;'
        );

        // Description fields.
        $all_fields = '';
        foreach ($table_fields as $key => $table_field) {
            $all_fields .= $table_field['title'] . ', ';
        }
        $row[] = array(
            'data' => 'Description: ' . $table_node->body . '</br>Fields: ' . substr($all_fields, 0, -2)
        );

        // Add row to the table.
        $rows[] = array('data' => $row);
    }

    $parameters['datatable_options'] = array(
        'bAutoWidth' => TRUE,
        'bLengthChange' => FALSE,
        'bPagination' => FALSE,
        'bFilter' => FALSE,
        'bSort' => FALSE,
        'bInfo' => FALSE,
        'bRetrieve' => TRUE,
    );

    foreach ($rows as $index => $row) {
        foreach ($row['data'] as $key => $value) {
            $rows[$index]['data'][$key]['data'] = '<span>' . $value['data'] . '</span>';
        }
    }

    $output = theme('datatable', $headers, $rows, $parameters);

    return $output;
}

/* * **************************************************************************************************
 * Helper functions
 * ************************************************************************************************** */

/**
 * Returns array of administration links.
 */
function _tow_dataset_admin_tasks($node) {

    $count = _tow_dataset_count_all($node->uid);

    // Maintenance mode message.
    if ($node->status) {
        $maintenance = t('Put this data set into ') . l('maintenance mode', 'dataset/' . $node->nid . '/maintenance');
    } else {
        $maintenance = t('Now this data set is in maintenance mode. ' . l('Publish it', 'dataset/' . $node->nid . '/maintenance'));
    }

    // Admin tasks links.
    $admin_tasks = array(
        'create' => l('Create', 'node/add/table', array('query' => 'dataset=' . $node->nid)) . t(' or ') . l('import', 'import/' . $node->nid) . t(' a new table'),
        'manage' => l('Manage tables', 'dataset/' . $node->nid),
        'access' => drupal_get_form('tow_access_type_form'),
    );

    if ($node->field_access_type[0]['value']) {
        $admin_tasks['invite'] = l('Invite people', 'invite/' . $node->nid) . ' ' . t('to access your data set');
        $admin_tasks['access_requests'] = tow_access_requests($node->nid);
    }

    // Add more tasks.
    $admin_tasks = array_merge($admin_tasks, array(
        'current_status' => t('Currently a normal user (free), with ') . $count['tables'] . t(' tables, ') . $count['records'] . t(' rows.'),
        'maintenence' => $maintenance,
        'delete' => l('Delete this dataset forever', 'node/' . $node->nid . '/delete'),
            ));

    return theme_item_list($admin_tasks);
}

/**
 * Returns date in "intervals ago" format (intervals are minutes, hours, days etc.) from a timestamp.
 */
function _tow_dataset_ago($timestamp) {
    $difference = time() - $timestamp;

    // Periods.
    $periods = array(
        "second",
        "minute",
        "hour",
        "day",
        "week",
        "month",
        "year",
        "decade"
    );

    // Amount of periods in a parent period.
    $lengths = array(
        "60",
        "60",
        "24",
        "7",
        "4.35",
        "12",
        "10"
    );

    // Generate the output.
    for ($j = 0; $difference >= $lengths[$j]; $j++) {
        $difference /= $lengths[$j];
    }
    $difference = round($difference);
    if ($difference != 1) {
        $periods[$j] .= "s";
    }
    $text = "$difference $periods[$j] ago";

    return $text;
}

/**
 * Count amount of tables and records the user owns.
 */
function _tow_dataset_count_all($uid) {
    $res = db_query("

      SELECT
        COUNT(*) AS c

        FROM
          {node} n

        WHERE
            n.uid = %d
          AND
            n.type = 'table'

    UNION ALL

      SELECT
        COUNT(*) AS c

        FROM
          {node} n

        WHERE
            n.uid = %d
          AND
            n.type = 'record'

  ", $uid, $uid);

    $data[] = db_fetch_array($res);
    $data[] = db_fetch_array($res);

    $count = array(
        'tables' => $data[0]['c'],
        'records' => $data[1]['c'],
    );

    return $count;
}
