<?php

/**
 * @file
 *
 * Table API.
 */
/* * **************************************************************************************************
 * Hook handlers and forms
 * ************************************************************************************************** */

/**
 * Returns form with 'Clone table' button.
 */
function tow_table_clone_table_form() {
    $form['clone'] = array(
        '#type' => 'submit',
        '#value' => t('Clone table')
    );
    return $form;
}

/**
 * Returns form with 'Delete table' button.
 */
function tow_table_delete_table_form() {
    $form['delete'] = array(
        '#type' => 'submit',
        '#value' => t('Delete table')
    );
    return $form;
}

/**
 * Part of hook_block() implementation.
 */
function tow_table_display_switch_block() {

    global $_tow_dataset;
    global $_tow_table;

    // Do not display this block on non-table pages.
    if (!isset($_tow_table) || !($_tow_table->nid)) {
        return;
    }

    // Define links.
    $items[] = l('Content', 'table/' . $_tow_table->nid);
    $items[] = l('Structure', 'node/' . $_tow_table->nid);
    //$items[] = l('Search', 'search_dataset/' . $_tow_table->nid);
    // Generate the content of the block.
    $block['content'] = theme_item_list($items, NULL, 'ul');

    return $block;
}

/**
 * User interface for field adding.
 */
function tow_table_field_add_form($form_state, $node, $existing_fields) {

    $form['existing_fields'] = array(
        '#type' => 'value',
        '#value' => $existing_fields
    );

    $form['node'] = array(
        '#type' => 'value',
        '#value' => $node
    );

    $form['field_add'] = array(
        '#type' => 'fieldset',
        '#title' => 'Add field',
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#tree' => TRUE,
        '#weight' => -0.5
    );

    $form['field_add']['title'] = array(
        '#type' => 'textfield',
        '#description' => t('Enter title'),
        '#size' => 5,
        '#maxlength' => 20,
    );

    $form['field_add']['type'] = array(
        '#type' => 'select',
        '#description' => t('Select type'),
        '#options' => tow_possible_fields(),
    );

    $form['field_add']['unit'] = array(
        '#type' => 'textfield',
        '#description' => t('Enter unit'),
        '#size' => 20,
        '#maxlength' => 20,
    );

    $form['field_add']['enum_options'] = array(
        '#type' => 'textarea',
        '#description' => t('If you choose enum, enter enum options here. Each option should be in single string.'),
    );

    $form['field_add']['description'] = array(
        '#type' => 'textarea',
        '#description' => t('Write a short description to help people understand this field (optional).'),
    );

    $form['field_add']['index'] = array(
        '#type' => 'checkbox',
        '#title' => t('Index')
    );

    $form['field_add']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('add field')
    );

    $form['#validate'][] = 'tow_table_field_add_form_validate';

    return $form;
}

/**
 * Part of hook_form_alter() implementation.
 */
function tow_table_form_alter(&$form, &$form_state) {

    /* DEFAULT */
    // Unset unnecessary elements.
    unset($form['menu']);
    unset($form['revision_information']);
    unset($form['comment_settings']);

    // Hide native CCK fields.
    foreach (tow_possible_fields() as $field_type) {
        $form['field_title_' . $field_type]['#access'] = FALSE;
    }
    $form['field_enum_options']['#access'] = FALSE;
    $form['field_dataset']['#access'] = FALSE;

    $form['#content_extra_fields']['body_field']['weight'] = 0;

    // Add tabular form containing.
    $form['fields'] = array(
        '#theme' => 'tow_table_tabular_form',
        '#title' => 'Fields',
        '#tree' => TRUE,
        '#weight' => -1,
    );

    $form['fields']['header'] = array(
        '#type' => 'value',
        '#value' => array('Title', 'Type'),
    );

    // Add array of delta.
    $form['delta'] = array(
        '#tree' => TRUE
    );

    // Get an array of fields ordered by weights.
    $weights_array = unserialize($form['#node']->field_weights[0]['value']);

    // Register common validation function.
    $form['#validate'][] = 'tow_table_validate_common';

    /* TABLE IS BEING EDITED */
    if ($weights_array) {

        global $_tow_dataset;
        global $_tow_table;

        // Load current dataset and table to global variables.
        $_tow_dataset = node_load($form['#node']->field_dataset['0']['value']);
        $_tow_dataset = (object) array('nid' => $_tow_dataset->nid, 'title' => $_tow_dataset->title);
        $_tow_table = (object) array('nid' => $form['#node']->nid, 'title' => $form['#node']->title);

        // Save field values to an auxiliary array.
        $index = 0;
        foreach (tow_possible_fields() as $field_type) {
            $delta = 0;
            if (!is_array($form['#node']->{'field_title_' . $field_type})) {
                continue;
            }
            foreach ($form['#node']->{'field_title_' . $field_type} as $title_value) {
                if ($title_value['value']) {
                    $aux_array[$weights_array[$index]]['value'] = $title_value['value'];
                    $aux_array[$weights_array[$index]]['type'] = $field_type;
                    $aux_array[$weights_array[$index]]['delta'] = $delta;
                    $index++;
                    $delta++;
                }
            }
        }

        // Sort an auxiliary array.
        ksort($aux_array);

        // Get units.
        $form['fields']['header']['#value'][] = t('Unit');
        $units_array = $form['#node']->field_units;

        // Get indexes.
        $form['fields']['header']['#value'][] = t('Index');
        $indexes_array = unserialize($form['#node']->field_indexes[0]['value']);

        // Nest arrays of delta and array of fields.
        foreach ($aux_array as $key => $value) {

            $form['delta'][$key] = array(
                '#type' => 'value',
                '#value' => $value['delta']
            );

            // Title: editable.
            $form['fields']['data'][$key]['title'] = array(
                '#type' => 'textfield',
                '#default_value' => $value['value'],
                '#size' => 5,
            );

            // Type: noneditable, for information.
            $form['fields']['data'][$key]['type'] = array(
                '#type' => 'item',
                '#value' => $value['type'],
            );

            // Unit: editable.
            $form['fields']['data'][$key]['unit'] = array(
                '#type' => 'textfield',
                '#default_value' => _tow_field_remove_fake($units_array[$key]['value']),
                '#size' => 5,
            );

            // Index: Yes / No.
            $form['fields']['data'][$key]['index'] = array(
                '#type' => 'checkbox',
                '#default_value' => $indexes_array[$key],
            );

            // Get enum options for enum fields.
            if ($value['type'] == 'enum') {
                foreach ($form['#node']->field_title_enum as $enum_delta => $enum_value) {
                    if ($enum_value['value'] == $value['value']) {
                        break;
                    }
                }
                $enum_options = unserialize($form['#node']->field_enum_options[$enum_delta]['value']);
                if ($enum_options) {
                    $enum_options_string = implode(' | ', $enum_options);
                    $form['fields']['data'][$key]['type']['#value'] .= ' : ' . $enum_options_string;
                }
            }
        }

        ## TODO: interface for field deleting
        // Add interface for field adding.
        $form['field_add'] = array(
            '#type' => 'fieldset',
            '#title' => 'Add field',
            '#collapsible' => TRUE,
            '#collapsed' => FALSE,
            '#tree' => TRUE,
            '#weight' => -0.5
        );

        $form['field_add']['title'] = array(
            '#type' => 'textfield',
            '#description' => t('Enter title'),
            '#size' => 5,
            '#maxlength' => 20,
        );

        $form['field_add']['type'] = array(
            '#type' => 'select',
            '#description' => t('Select type'),
            '#options' => tow_possible_fields(),
        );

        $form['field_add']['unit'] = array(
            '#type' => 'textfield',
            '#description' => t('Enter unit'),
            '#size' => 20,
            '#maxlength' => 20,
        );

        $form['field_add']['enum_options'] = array(
            '#type' => 'textarea',
            '#description' => t('If you choose enum, enter enum options here. Each option should be in single string.'),
        );

        $form['field_add']['description'] = array(
            '#type' => 'textarea',
            '#description' => t('Write a short description to help people understand this field (optional).'),
        );

        $form['field_add']['index'] = array(
            '#type' => 'checkbox',
            '#title' => t('Index')
        );

        // Register additional validation function for table edit case.
        $form['#validate'][] = 'tow_table_validate_edit';

        // Add interface for actions: add record, truncate, browse.
        $form['action_links'] = array(
            '#type' => 'item',
            '#value' => _tow_table_actions_list($form['#node']->nid, 'browse'),
            '#weight' => -0.3
        );

        ## TODO: user friendly interface for weights changing
        // Register Submission callbacks.
        $form['#submit'][] = 'tow_table_submit_fields';
        $form['#submit'][] = 'tow_table_submit_weights';
        $form['#submit'][] = 'tow_table_submit_field_add';
        $form['#submit'][] = 'tow_table_submit_indexes';
        $form['#submit'][] = 'tow_table_submit_units';
    }

    /* TABLE IS BEING CREATED */ else {

        // Get dataset value.
        if ($_SESSION['table_definition']) {
            $form['title']['#default_value'] = $_SESSION['table_definition']['table_name'];
            $form['field_dataset'][0]['#default_value']['value'] = $_SESSION['table_definition']['dataset'];
            $count_fields = $_SESSION['table_definition']['count_fields'];
            unset($_SESSION['table_definition']);
        } else {
            $count_fields = 1;
            if (is_numeric($_GET['dataset']))
                $form['field_dataset'][0]['#default_value']['value'] = $_GET['dataset'];
            $form['#validate'][] = 'tow_table_dont_validate_cancel';
        }

        // Load dataset.
        global $_tow_dataset;
        $_tow_dataset = node_load($form['field_dataset'][0]['#default_value']['value']);
        $_tow_dataset = (object) array('nid' => $_tow_dataset->nid, 'title' => $_tow_dataset->title);

        // Submit value.
        $form['buttons']['submit']['#value'] = t('Create table');

        // Field headers.
        $form['fields']['header']['#value'][] = 'Unit';
        $form['fields']['header']['#value'][] = 'Enum options';
        $form['fields']['header']['#value'][] = 'Description';
        $form['fields']['header']['#value'][] = 'Index';

        // Elemnts for the fields' defenition.
        for ($i = 0; $i < $count_fields; $i++) {

            // Title.
            $form['fields']['data'][$i]['title'] = array(
                '#type' => 'textfield',
                '#size' => 8,
            );

            // Type.
            $form['fields']['data'][$i]['type'] = array(
                '#type' => 'select',
                '#options' => tow_possible_fields(),
            );

            // Unit: editable.
            $form['fields']['data'][$i]['unit'] = array(
                '#type' => 'textfield',
                '#size' => 8,
            );

            // Enum options.
            $form['fields']['data'][$i]['enum_options'] = array(
                '#type' => 'textarea',
                '#rows' => 2,
            );

            // Description.
            $form['fields']['data'][$i]['description'] = array(
                '#type' => 'textarea',
                '#rows' => 2,
            );

            // Index.
            $form['fields']['data'][$i]['index'] = array(
                '#type' => 'checkbox',
            );
        }

        $field_types = tow_possible_fields();

        // Field params.
        for ($i = 2; $i < $count_fields; $i++) {
            foreach ($field_types as $field_type) {
                $form['field_title_' . $field_type][$i] = $form['field_title_' . $field_type][0];
                $form['field_title_' . $field_type][$i]['_weight']['#delta'] = $i;
                $form['field_title_' . $field_type][$i]['#delta'] = $i;
                $form['field_title_' . $field_type][$i]['#weight'] = $i;
            }
            $form['field_enum_options'][$i] = $form['field_enum_options'][0];
            $form['field_enum_options'][$i]['_weight']['#delta'] = $i;
            $form['field_enum_options'][$i]['#delta'] = $i;
            $form['field_enum_options'][$i]['#weight'] = $i;
        }

        // Field amount.
        $form['count_fields'] = array(
            '#type' => 'value',
            '#value' => $count_fields,
        );

        $form['#submit'][] = 'tow_table_submit_create';
    }
}

/**
 * Menu callback. Page view of the table.
 */
function tow_table_page_callback($nid) {

    // Load table.
    $node = node_load($nid);

    // Error 404 case the node is not a table.
    if (!$node || $node->type != 'table') {
        drupal_not_found();
        die();
    }

    global $_tow_dataset;
    global $_tow_table;
    global $user;

    // Set global dataset and table variables.
    $_tow_dataset = node_load($node->field_dataset['0']['value']);
    $_tow_dataset = (object) array('nid' => $_tow_dataset->nid, 'title' => $_tow_dataset->title, 'uid' => $_tow_dataset->uid);
    $_tow_table = $node;

    // Generate an output of the table: title, teaser and admin tasks.
    $output = "<div>" . $node->title . "</div>";
    $output .= $node->teaser;
    if ($user->uid == $_tow_dataset->uid || user_access('edit any table')) {
        $output .= "<div>" . l('Edit', 'node/' . $node->nid . '/edit', array('query' => 'destination=table/' . $node->nid)) . "</div>";
    }

    // Define display options.
    $display_options = tow_table_display_options($node);
    $display_options['paged_view'] = FALSE;
    $display_options['click_sortable'] = FALSE;
    $display_options['full_view'] = TRUE;

    // Generate an output of the table: table content.
    $output .= tow_table($node, $display_options);
    $output .= '<br />';

    // Generate an output of the table: additional actions.
    if ($user->uid == $_tow_dataset->uid) {
        $output .= _tow_table_actions_list($node->nid, 'edit');
    }

    return $output;
}

/**
 * Build confirmation form for table truncating.
 */
function tow_table_truncate_confirm(&$form_state, $nid) {

    $node = node_load($nid);

    global $user;
    if ($user->uid != $node->uid) {
        drupal_access_denied();
        die();
    }

    $form['nid'] = array(
        '#type' => 'value',
        '#value' => $nid,
    );

    $form['#dataset'] = $node->field_dataset[0]['value'];

    return confirm_form($form, t('Are you sure you want to truncate %title?', array('%title' => $node->title)), 'table/' . $node->nid, t('This action cannot be undone.'), t('Truncate'), t('Cancel'));
}

/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Get table data to be placed into "table" view.
 *
 * @param $node
 *   Node to be displayed.
 * @param $display_options
 *   An associative array of display options:
 *     sort_index: column index or 'nid' // default value: 'nid',
 *     sort_order: 'ASC' or 'DESC // default value: 'ASC',
 *     filter_column_index: column index // default value: 0,
 *     filter_operator_key: filtering operator // default value: 0,
 *     filter_value: value to be used during comparison // default value: 0,
 *     first_elem_no: first element to be displayed // default value: 0,
 *     elements_per_page: no of elements to display // default value: 10.
 *
 * @return
 *   An array of table rows. Each row is array of cells.
 */
function tow_table($node, $display_options = array()) {

    // Define the amount of elements per page.
    $paged_view = ($display_options['paged_view'] === TRUE);
    $full_view = ($display_options['full_view'] === TRUE);
    if (($paged_view) && (isset($display_options['limits']['total_elements']))) {
        $total_elements = $display_options['limits']['total_elements'];
    } else {
        $total_elements = 10;
    }

    $headers = $rows = array();

    // Creating query.
    $user_table = tow_table_construct_table_query($node, $display_options);

    // Set headers.
    foreach ($user_table['titles']['values'] as $key => $value) {
        $headers[$key] = array('data' => $value);
    }

    // Extend headers if necessary.
    global $user;
    $dataset = node_load($node->field_dataset['0']['value']);
    $show_actions = ($full_view && ($user->uid == $dataset->uid || user_access('edit any table')));
    if ($show_actions) {
        array_unshift($headers, array(
            'data' => 'Action',
            'datatable_options' => array('bSortable' => FALSE),
                )
        );
    }

    $query = $user_table['query'];
    $args = $user_table['arguments'];

    // Get results according to view type.
    if ($paged_view) {
        $result = pager_query($query, $total_elements, 0, NULL, $args);
    } else {
        $result = db_query($query, $args);
    }
    while ($rs = db_fetch_array($result)) {
        $row = array();
        foreach ($rs as $col_key => $value) {
        $key = substr($col_key, 4);
            $table_types_key = isset($user_table['titles']['types'][$key]) ? $user_table['titles']['types'][$key] : '';
            if ($table_types_key == 'bool') {
                $row[] = array('data' => _tow_format_bool($value));
            } elseif ($table_types_key == 'date') {
                $row[] = array('data' => substr($value, 0, 10));
            } elseif ($table_types_key == 'timestamp') {
                $row[] = array('data' => date('Y-m-d H:i:s', $value));
            } elseif ($table_types_key == 'enum') {
                $title_enum = $user_table['titles']['values'][$key];
                foreach ($node->field_title_enum as $enum_delta => $enum_value) {
                    if ($enum_value['value'] == $title_enum) {
                        unset($node->field_title_enum[$enum_delta]);
                        break;
                    }
                }
                $enum_options = unserialize($node->field_enum_options[$enum_delta]['value']);
                $row[] = array('data' => $enum_options[$value]);
            } else {
                $row[] = array('data' => $value);
            }
        }
        if ($show_actions) {
            $record_nid = $row[0]['data'];
            $row[0]['data'] = l('edit', 'node/' . $record_nid . '/edit', array('query' => 'destination=table/' . $node->nid, 'attributes' => array('class' => 'edit-row')));
            $row[0]['data'] .= ' ' . l('delete', 'node/' . $record_nid . '/delete', array('query' => 'destination=table/' . $node->nid, 'attributes' => array('class' => 'delete-row')));
        } else {
            array_shift($row);
        }
        $rows[] = array('data' => $row);
    }

    // Display parameters for dataTable theme.
    $parameters['datatable_options'] = array(
        'bLengthChange' => FALSE,
        'bPaginate' => FALSE,
        'bFilter' => FALSE,
        'bSort' => FALSE,
        'bInfo' => FALSE,
        'sScrollX' => '100%',
        'bRetrieve' => TRUE,
    );

    // Full display parameters for dataTable theme.
    $parameters_full['datatable_options'] = array(
        'bLengthChange' => FALSE,
        'bPaginate' => FALSE,
        'bFilter' => TRUE,
        'bSort' => TRUE,
        'bInfo' => FALSE,
        'sScrollX' => '100%',
        'sScrollY' => '300',
        'bScrollCollapse' => TRUE,
        'bRetrieve' => TRUE,
    );

    // Theme dataTable.
    foreach ($rows as $index => $row) {
        foreach ($row['data'] as $key => $value) {
            $rows[$index]['data'][$key]['data'] = '<span>' . $value['data'] . '</span>';
        }
    }

    if ($full_view) {
        $output = theme('datatable', $headers, $rows, $parameters_full);
    } else {
        $output = theme('datatable', $headers, $rows, $parameters);
    }

    // Additions for paged view case.
    if ($paged_view) {
        $output .= theme('pager', NULL, $total_elements);
        global $_tow_bc_attachment;
        $total = db_result(db_query("SELECT COUNT(*) FROM {content_type_record} WHERE field_table_value = %d", $node->nid));
        $_tow_bc_attachment = ' (' . $total . t(' rows; displaying ');
        $start = $_GET['page'] * $total_elements + 1;
        $_tow_bc_attachment .= $start . ' - ' . ($start + count($rows) - 1) . ')';
    }

    return $output;
}

/**
 * Construct a query for record data extraction from the database.
 *
 * @param $node
 *   Node with data.
 * @param $display_options
 *   Array:
 *    'sort' => array('sort_index', 'sort_order'),
 *    'filter' => array('filter_column_index', 'filter_operator_key', 'filter_value'),
 *    'limits' => array('first_elem_no', 'total_elements'),
 *    'paged_view' => boolean, determines whether returned query will be used in db_query() or paged_query(),
 *    'record_nids' => array of record_nid, which are to be displayed, this overrides 'first_elem_no' option.
 *
 * @return
 *   array(
 *     'query' => query string containing replacement operators,
 *     'arguments' => array of arguments for db_query,
 *     'titles' => array of column titles,
 *     'arguments' => array of arguments for db_query() or paged_query().
 *
 * @example
 *
 *   SELECT
 *     rec.nid AS nid,
 *     int0.field_int_value AS col_0,
 *     char0.field_char_value AS col_1,
 *     char1.field_char_value AS col_2,
 *     int1.field_int_value AS col_3,
 *     char2.field_char_value AS col_4,
 *     text0.field_text_value AS col_5
 *
 *    FROM
 *        content_field_int int0
 *      LEFT JOIN
 *        {content_field_char} char0
 *        ON
 *            char0.nid = int0.nid
 *          AND
 *            char0.delta = 0
 *          AND
 *            int0.delta = 0
 *      LEFT JOIN
 *        {content_field_char} char1
 *        ON
 *            char1.nid = int0.nid
 *          AND
 *            char1.delta = 1
 *      LEFT JOIN
 *        {content_field_int} int1
 *        ON
 *            int1.nid = int0.nid
 *          AND
 *            int1.delta = 1
 *      LEFT JOIN
 *        {content_field_char} char2
 *        ON
 *            char2.nid = int0.nid
 *          AND
 *            char2.delta = 2
 *      LEFT JOIN
 *        {content_field_text} text0
 *        ON
 *            text0.nid = int0.nid
 *          AND
 *            text0.delta = 0
 *
 *    WHERE
 *        rec.nid IN (
 *          SELECT
 *            nid
 *
 *            FROM
 *              {content_type_record}
 *
 *            WHERE
 *              field_table_value = %d
 *        )
 *      AND
 *        int0.field_int_value > 5
 *
 *    ORDER BY %s %s
 *
 *    LIMIT %d OFFSET %d
 */
function tow_table_construct_table_query($node, $display_options = '') {

    // Preparing data from node for query construction.
    // Just renaming.
    $current_table = $node;

    // First returned argument is Node id
    $argmts[] = $current_table->nid;

    $aux_array = _tow_get_table_fields($current_table);

    foreach ($aux_array as $index => &$field) {
        $field['alias'] = $field['type'] . $field['delta'];
    }

    // Query construction
    // is constructed of two parts. Both parts are recorded in one foreach cycle, and then concatenated.
    // First part of query (starting from SELECT):
    // "SELECT rec.nid AS nid,".
    if ($aux_array) {
        $query .= "SELECT rec.nid AS nid, ";

        // Second part of query (starting from "FROM {content_type_record} rec\n").
        $query_part2 = "FROM {content_type_record} rec ";

        // Main cycle for data selection
        // $column_index is responsible for the actual column numeration, from 0 to maximum column number.
        // We use it because $index in $aux_array may contain any values depending on
        //   a) filter settings,
        //   b) values returned by tow_possible_fields(),
        //   c) etc.
        //
    // $column_index will be used to check the validity of $sort_index, where $sort_index is
        // the index of the column, which values will be used for sorting. $sort_index shall be less or equal
        // to the number of columns, counted from zero.
        $column_index = 0;

        foreach ($aux_array as $index => $column_info) {

            // First part of the query, fields are added one by one in each loop iteration:
            //  "int0.field_int_value AS col_0, char0.field_char_value AS col_1, char1.field_char_value AS col_2, int1.field_int_value AS col_3, char2.field_char_value AS col_4, text0.field_text_value AS col_5".
            $query = $query . $column_info['alias'] . ".field_" . $column_info['type'] . "_value AS " . "col_$column_index" . ", ";

            // Second part:
            // includes all "LEFT JOIN"s.
            if ($column_index > 0) {
                $query_part2 = $query_part2 . "LEFT JOIN {content_field_" . $column_info['type'] . "} " . $column_info['alias'] . " ";
                $query_part2 = $query_part2 . " ON " . $column_info['alias'] . ".nid = " . "rec" . ".nid ";
                $query_part2 = $query_part2 . " AND " . $column_info['alias'] . ".delta = " . $column_info['delta'] . " ";
            } else {
                $query_part2 = $query_part2 . "INNER JOIN {content_field_" . $column_info['type'] . "} " . $column_info['alias'] . " ";
                $query_part2 = $query_part2 . " ON rec.field_table_value = %d ";
                $query_part2 = $query_part2 . " AND " . $column_info['alias'] . ".nid = " . "rec" . ".nid ";
                $query_part2 = $query_part2 . " AND " . $column_info['alias'] . ".delta = " . $column_info['delta'] . " ";
            }

            // Part of query, responsible for filtering.
            if ((isset($display_options['filter'])) && ($display_options['filter']) && ($display_options['filter']['filter_column_index'] == $column_index)) {
                $filter_query_part = $column_info['alias'] . ".field_" . $column_info['type'] . "_value ";
                $filter_query_part = $filter_query_part . "%s" . " ";
                $argmts[] = tow_table_filter_translate_operator($display_options['filter']['filter_operator_key']);
                if (is_string($display_options['filter']['filter_value'])) {
                    $filter_query_part = $filter_query_part . "'%s' ";
                } else {
                    $filter_query_part = $filter_query_part . "%d ";
                }
                $argmts[] = $display_options['filter']['filter_value'];
            }

            // Recording titles into $result to be returned from the function.
            $result['titles']['values'][$column_index] = $column_info['title'];
            $result['titles']['types'][$column_index] = $column_info['type'];

            $column_index++;
        }

        // Correction of total number of columns.
        $column_index--;

        // Correction of last symbols in the first part of the query
        $query = rtrim($query, ", ");
        $query .= " ";

        // Concatenation fo two query parts (SELECT and FROM parts).
        $query = $query . $query_part2;

        // Add WHERE part if record_nids are determined.
        if ($display_options['record_nids']) {
            // WHERE rec.nid = value1 OR rec.nid = value2 ...
            // Choose only records included in this table (field_table_value == table_nid).

            $query = $query . "WHERE (";
            foreach ($display_options['record_nids'] as $record_nid) {

                $query .= "rec.nid = %d OR ";
                $argmts[] = $record_nid;
            }
            $query = substr($query, 0, -4);
            $query .= ") ";
        }
    } else {
        // This part of query is added even if the table is empty, i. e. $aux_array is not filled in remains unset.
        $query .= "SELECT rec.nid FROM {content_type_record} rec WHERE field_table_value = %d";
    }

    if ($aux_array) {

        //  Filter results.
        if (isset($display_options['filter']) && $display_options['filter']) {
            if ($display_options['record_nids']) {
                $query .= " AND " . $filter_query_part;
            } else {
                $query .= " WHERE " . $filter_query_part;
            }
        }
    }

    // Add ORDER  part
    // ORDER BY %s %s
    $query = $query . " ORDER BY ";
    $query = $query . " " . db_escape_string("%s") . " ";

    // Check if sort index is correct (not larger than the number of columns).
    if (!($display_options['sort']) || ($display_options['sort']['sort_index'] > $column_index)) {
        $argmts[] = 'nid';
    } else {
        $argmts[] = (string) $display_options['sort']['sort_index'];
    }

    $query = $query . " " . db_escape_string("%s") . " ";

    if (!isset($display_options['sort']['sort_order'])) {
        $display_options['sort']['sort_order'] = "DESC";
    }
    $argmts[] = $display_options['sort']['sort_order'];

    // Add LIMIT part
    // LIMIT %d OFFSET %d

    if (!($display_options['paged_view']) && ($display_options['limits'])) {
        $query = $query . "LIMIT " . "%d" . " OFFSET " . "%d";

        $argmts[] = $display_options['limits']['total_elements'];
        $argmts[] = $display_options['limits']['first_elem_no'];
    }

    // 8. Output
    if (!$result['titles']['values']) {
        $result['titles']['values'] = array();
    }

    $result['query'] = $query;
    $result['arguments'] = $argmts;

    return $result;
}

/**
 * Checking $_GET, construct array of display options to be passed to tow_table().
 */
function tow_table_display_options($node) {

    $display_options = array();

    // Get number of fields.
    foreach (tow_possible_fields() as $field_type) {
        $count_fields += count($node->{'field_title_' . $field_type});
    }

    // Set sort parameters if possible.
    if (is_numeric($_GET['order']) && isset($_GET['sort'])) {
        if ($_GET['order'] >= 0 && $_GET['order'] < $count_fields) {
            if ($_GET['sort'] == 'asc' || $_GET['sort'] == 'desc') {
                $display_options['sort'] = array(
                    'sort_index' => $_GET['order'],
                    'sort_order' => $_GET['sort'],
                );
            }
        }
    }

    // Set filter parameters if possible.
    if (is_numeric($_GET['filter']) && is_numeric($_GET['op']) && isset($_GET['value'])) {
        if ($_GET['filter'] >= 0 && $_GET['op'] >= 0) {
            if ($_GET['filter'] < $count_fields) {
                if ($_GET['op'] < tow_table_filter_operators()) {
                    $display_options['filter'] = array(
                        'filter_column_index' => $_GET['filter'],
                        'filter_operator_key' => $_GET['op'],
                        'filter_value' => $_GET['value']
                    );
                }
            }
        }
    }

    return $display_options;
}

/**
 * Return array of filter operators.
 */
function tow_table_filter_operators() {
    return array('Is equal to', 'Is less than', 'Is greater than');
}

/**
 * Return corresponding sql operator. See function above.
 */
function tow_table_filter_translate_operator($op_key) {
    $sql_operators = array('=', '<', '>');
    return $sql_operators[$op_key];
}

/**
 * Add table structure themed as table to the node.
 */
function tow_table_structure(&$node) {

    global $_tow_dataset;
    global $_tow_table;
    global $_tow_bc_attachment;
    global $user;

    // Define global variables.
    $_tow_bc_attachment = ' (' . t('table structure') . ')';
    $_tow_dataset = node_load($node->field_dataset['0']['value']);
    $_tow_dataset = (object) array('nid' => $_tow_dataset->nid, 'title' => $_tow_dataset->title);
    $_tow_table = (object) array('nid' => $node->nid, 'title' => $node->title);

    // Admin options.
    if ($user->uid == $node->uid || user_access('edit any table')) {
        $node->structure['clone'] = drupal_get_form('tow_table_clone_table_form');
        $node->structure['delete'] = drupal_get_form('tow_table_delete_table_form');
        $node->structure['edit_description'] = l('edit description', 'node/' . $node->nid . '/edit');
    }

    // Headers.
    $headers = array(t('Action'), t('Field'), t('Type'), t('Unit'), t('Index'), t('% Blanks'));

    // Define arrays of units, weights and indexes.
    $units_array = $node->field_units;
    $weights_array = unserialize($node->field_weights[0]['value']);
    $index_array = unserialize($node->field_indexes[0]['value']);

    // If there are any fields fill rows and sort them by weights.
    if (!empty($weights_array)) {
        $index = 0;
        foreach (tow_possible_fields() as $field_type) {
            $delta = 0;
            if (!is_array($node->{'field_title_' . $field_type})) {
                continue;
            }
            foreach ($node->{'field_title_' . $field_type} as $title_value) {

                // If title value is set.
                if ($title_value['value']) {

                    // Set action.
                    if ($user->uid == $node->uid || user_access('edit any table')) {
                        $rows[$weights_array[$index]]['action'] = l('delete', 'field_delete/' . $_tow_table->nid . '/' . $field_type . '/' . $delta . '/' . $weights_array[$index]) . ' ' . l('edit', 'field_edit/' . $_tow_table->nid . '/' . $field_type . '/' . $delta . '/' . $weights_array[$index]);
                    } else {
                        $rows[$weights_array[$index]]['action'] = '';
                    }

                    // Set rows.
                    $rows[$weights_array[$index]]['value'] = $title_value['value'];
                    $rows[$weights_array[$index]]['type'] = $field_type;
                    $rows[$weights_array[$index]]['unit'] = _tow_field_remove_fake($units_array[$weights_array[$index]]['value']);
                    $rows[$weights_array[$index]]['index'] = $index_array[$weights_array[$index]] ? t('Yes') : t('No');
                    $rows[$weights_array[$index]]['blanks'] = tow_table_structure_blanks($node->nid, $delta, $field_type);

                    // Case enum.
                    if ($field_type == 'enum') {
                        $enum_options = unserialize($node->field_enum_options[$delta]['value']);
                        if ($enum_options) {
                            $enum_options_string = implode(' | ', $enum_options);
                            $rows[$weights_array[$index]]['type'] .= ' : ' . $enum_options_string;
                        }
                    }

                    // Increase delta and index.
                    $index++;
                    $delta++;
                }
            }
        }

        // Sort rows.
        ksort($rows);

        // Adding info link for each row.
        $index = 0;
        foreach ($rows as $key => $value) {
            $rows[$key]['action'] = l('info', 'field/' . $_tow_table->nid . '/' . $index++) . ' ' . $rows[$key]['action'];
        }

        // Add a themed table to the structure.
        $node->structure['structure'] = theme('table', $headers, $rows);
    } else {

        // There is no rows. Inform about this.
        $node->structure['structure'] = t('There are no rows');
        $count_fields = 0;
    }

    // Rows amount.
    $count_rows = db_result(db_query("SELECT COUNT(*) FROM {content_type_record} WHERE field_table_value = %d", $_tow_table->nid));
    if ($count_rows == 1) {
        $count_rows .= ' ' . t('row');
    } else {
        $count_rows .= ' ' . t('rows');
    }

    // Fields amount.
    $count_fields = count($rows);
    if ($count_fields == 1) {
        $count_fields .= ' ' . t('field');
    } else {
        $count_fields .= ' ' . t('fields');
    }

    // Add statistics.
    $node->structure['table_stats'] = t('This table has %rows and %fields', array('%rows' => $count_rows, '%fields' => $count_fields));
    global $_tow_table_statistics;
    if (!$_tow_table_statistics) {
        $_tow_table_statistics = 0;
    }
    $node->structure['table_stats'] .= "<br>" . t('Views') . ': ' . $_tow_table_statistics;

    // Show fields adding form for users with permission to add them.
    if ($user->uid == $node->uid || user_access('edit any table')) {
        foreach ($rows as $row) {
            $existing_fields[] = $row['value'];
        }
        $node->structure['field_add'] = drupal_get_form('tow_table_field_add_form', $node, $existing_fields);
    }
}

/**
 * Returns percentage of blank cells of a field.
 */
function tow_table_structure_blanks($table_nid, $delta, $type) {

    // Get blanks percentage.
    $blanks = round(db_result(db_query("

    SELECT
      1 - (

        SELECT
          COUNT(*)

          FROM
              {content_field_%s} x
            INNER JOIN
              {content_type_record} r
              ON
                  x.nid = r.nid
                AND
                  x.field_%s_value IS NOT NULL

          WHERE
              r.field_table_value = %d
            AND
              x.delta = %d

      ) / (

        SELECT
          COUNT(*)

          FROM
            {content_type_record} r

          WHERE

            r.field_table_value = %d

      )

  ", $type, $type, $table_nid, $delta, $table_nid)) * 100);

    // Format the output.
    if ($blanks != 0) {
        $blanks .= '%';
    }

    return $blanks;
}

/* * **************************************************************************************************
 * Helper functions
 * ************************************************************************************************** */

/**
 * Returns html list of links: add record, truncate, browse OR edit table.
 */
function _tow_table_actions_list($nid, $last_link) {
    $items = array(
        l('Add record', 'node/add/record', array('html' => TRUE, 'query' => 'table=' . $nid)),
        l('Delete all', 'table/' . $nid . '/truncate'),
    );

    if ($last_link == 'edit') {
        $items[] = l('Edit table', 'node/' . $nid . '/edit');
    } elseif ($last_link == 'browse') {
        $items[] = l('Browse table', 'table/' . $nid);
    }

    return theme_item_list($items);
}

/**
 * If $title has no digits at the end, e.g. 'mytable', return 'mytable01'.
 * If it has digits at the end, e.g. 'table05', return 'table06'.
 */
function _tow_table_numbered_title($title) {

    $len = $i = strlen($title) - 1;
    while (is_numeric($title[$i])) {
        $number .= $title[$i];
        $i--;
    }

    if (isset($number)) {
        $number = strrev($number);
        $word = substr($title, 0, 1 + $len - strlen($number));
        $number++;
        if ($number < 10)
            $number = '0' . $number;
    }
    else {
        $word = $title;
        $number = '01';
    }

    return $word . $number;
}

/* * **************************************************************************************************
 * Validation callbacks
 * ************************************************************************************************** */

/**
 * When "Cancel" button is clicked, redirect to node dataset page without error messages.
 */
function tow_table_dont_validate_cancel($form, &$form_state) {
    if ($form_state['clicked_button']['#value'] == 'Cancel') {
        unset($_SESSION['messages']['error']);
        drupal_goto('node/' . $form_state['values']['field_dataset'][0]['value']);
    }
}

/**
 * Performs form validation during adding field from node/table_id/ page.
 */
function tow_table_field_add_form_validate(&$form, &$form_state) {

    $error = FALSE;
    $err_field_names = array();
    $err_msgs = array();

    $field_types = tow_possible_fields();

    // Empty fields check.
    if ($form_state['values']['field_add']['title'] == '') {
        $error = TRUE;
        $err_field_names[] = "field_add][title";
        $err_msgs[] = t('New field title must neither be empty nor consist of nothing but whitespaces.');
    } else {

        // Unique new column name check.
        $column_names = $form_state['values']['existing_fields'];
        $no = array_search(strtolower($form_state['values']['field_add']['title']), $column_names);

        if (!($no === FALSE)) {
            $error = TRUE;
            $err_field_names[] = "field_add][title";
            $err_msgs[] = t('New field must have unique title.');
        }
    }

    // Correct number of enum options for new field check.
    $field_types = tow_possible_fields();

    if (($field_types[$form_state['values']['field_add']['type']] == 'enum') &&
            trim($form_state['values']['field_add']['enum_options']) == '') {
        $error = TRUE;
        $err_field_names[] = "field_add][enum_options";
        $err_msgs[] = t('You must specify at least one option for enum field.');
    }

    // Showing error messages.
    _tow_show_validation_error($error, $err_field_names, $err_msgs);
}

/**
 * Performs form validation during table creation or editing.
 */
function tow_table_validate_common($form, &$form_state) {

    if ($form_state['clicked_button']['#value'] == 'Cancel') {
        return;
    }

    $error = FALSE;
    $err_field_names = array();
    $err_msgs = array();

    $field_types = tow_possible_fields();

    // Unique table title.
    $table_titles_res = db_query("

    SELECT
      n.title AS title

      FROM
        {node} n

      WHERE
          n.nid IN (

            SELECT
              n.nid AS nid

              FROM
                {content_type_table} ctt

              WHERE
                ctt.field_dataset_value = %d

          )

        AND
          nid <> %d

  ", $form_state['values']['field_dataset'][0]['value'], $form_state['values']['nid']);
    $table_titles = array();
    while ($rs = db_fetch_array($table_titles_res)) {
        $table_titles[] = strtolower($rs['title']);
    }
    $form_state['values']['title'] = trim($form_state['values']['title']);
    if (in_array(strtolower($form_state['values']['title']), $table_titles)) {
        $error = TRUE;
        $err_field_names[] = "title";
        $err_msgs[] = t('The table with this title already exists.');
    }

    foreach ($form_state['values']['fields']['data'] as $key => $value) {

        // Trim field values.
        $form_state['values']['fields']['data'][$key]['title'] = trim($value['title']);
        $column_names[] = strtolower($form_state['values']['fields']['data'][$key]['title']);

        // Empty fields check.
        if ($form_state['values']['fields']['data'][$key]['title'] == '') {
            $error = TRUE;
            $err_field_names[] = "fields][data][$key][title";
            $err_msgs[] = t('Field titles must neither be empty nor consist of nothing but whitespaces.');
        }

        // Enum options quantity check.
        if (($field_types[$form_state['values']['fields']['data'][$key]['type']] == 'enum') && trim($form_state['values']['fields']['data'][$key]['enum_options']) == '') {
            $error = TRUE;
            $err_field_names[] = "fields][data][$key][enum_options";
            $err_msgs[] = t('You must specify at least one option for enum field.');
        }
    }

    // Unique field names check.
    if (count($column_names) != count(array_unique($column_names))) {
        $error = TRUE;

        // Count the frequency of field names. At least one of them will be greater than 1.
        $column_names_qty = array_count_values($column_names);

        // Remove not repeated elements from $column_names_qty array.
        $column_names_not_repeated = array_keys($column_names_qty, 1);
        foreach ($column_names_not_repeated as $column_name) {
            unset($column_names_qty[$column_name]);
        }

        foreach ($column_names_qty as $column_name => $qty) {

            // Extract numbers of fields with repeating titles.
            $repeated_field_no = array_keys($column_names, $column_name);

            // Set error for them.
            foreach ($repeated_field_no as $no) {
                $err_field_names[] = "fields][data][$no][title";
                $err_msgs[] = t('Field titles must have unique values.');
            }
        }
    }

    // Showing error messages.
    _tow_show_validation_error($error, $err_field_names, $err_msgs);
}

/**
 * Performs extra form validation during table editing from node/table_id/edit/ page connected
 * with the possibility of adding fields.
 */
function tow_table_validate_edit(&$form, &$form_state) {

    $error = FALSE;
    $err_field_names = array();
    $err_msgs = array();

    $field_types = tow_possible_fields();

    foreach ($form_state['values']['fields']['data'] as $key => $value) {
        $form_state['values']['fields']['data'][$key]['title'] = trim($value['title']);
        $column_names[] = strtolower($form_state['values']['fields']['data'][$key]['title']);
    }

    // Unique new column name check.
    $no = array_search(strtolower($form_state['values']['field_add']['title']), $column_names);

    if (!($no === FALSE) && ($column_names[$no] != '')) {
        $error = TRUE;
        $err_field_names[] = "field_add][title";
        $err_msgs[] = t('New field must have unique title.');
        $err_field_names[] = "fields][data][$no][title";
        $err_msgs[] = t('New field must have unique title.');
    }

    // Correct number of enum options for new field check.
    $field_types = tow_possible_fields();
    if (($field_types[$form_state['values']['field_add']['type']] == 'enum') && trim($form_state['values']['field_add']['enum_options']) == '') {
        $error = TRUE;
        $err_field_names[] = "field_add][enum_options";
        $err_msgs[] = t('You must specify at least one option for enum field.');
    }

    // Showing error messages
    _tow_show_validation_error($error, $err_field_names, $err_msgs);
}

/* * **************************************************************************************************
 * Submission callbacks
 * ************************************************************************************************** */

/**
 * Save node of table type similar to original.
 */
function tow_table_clone_table_form_submit($form, &$form_state) {

    // Prepare node.
    global $user;
    global $_tow_dataset;
    global $_tow_table;
    $node = node_load($_tow_table->nid);
    unset($node->nid, $node->vid, $node->created, $node->changed, $node->comment, $node->last_comment_timestamp, $node->last_comment_name, $node->comment_count);

    // Set new title.
    $title = $node->title;
    $len = $i = strlen($title) - 1;
    while (is_numeric($title[$i])) {
        $number .= $title[$i];
        $i--;
    }
    if (isset($number)) {
        $number = strrev($number);
        $word = substr($title, 0, 1 + $len - strlen($number));
    } else {
        $word = $title;
    }
    $highest_db_title = db_result(db_query("

    SELECT
      n.title

      FROM
          {node} n
        INNER JOIN
          {content_type_table} ctt
          ON
              n.nid = ctt.nid
            AND
              ctt.field_dataset_value = %d

      WHERE
        n.title LIKE '%s%'

      ORDER BY
        n.title DESC

      LIMIT 1

  ", $_tow_dataset->nid, $word));
    if ($highest_db_title == $title) {
        $number++;
        if ($number < 10)
            $number = '0' . $number;
        $node->title = $word . $number;
    }
    else {
        $node->title = _tow_table_numbered_title($highest_db_title);
    }

    // Save node.
    try {
        node_save($node);
    } catch (Exception $e) {
        watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
    }

    // Redirect.
    $nid = db_result(db_query_range(db_rewrite_sql("SELECT n.nid FROM {node} n WHERE n.uid = %d ORDER BY n.nid DESC"), array($user->uid), 0, 1));
    drupal_set_message(t('Table %title has been created', array('%title' => $node->title)));
    $form_state['redirect'] = 'node/' . $nid;
}

/**
 * Delete table node and redirect to dataset.
 */
function tow_table_delete_table_form_submit($form, &$form_state) {
    global $_tow_table;
    global $_tow_dataset;

    // Deletion.
    node_delete($_tow_table->nid);

    // Redirect.
    $form_state['redirect'] = 'dataset/' . $_tow_dataset->nid;
}

/**
 * Rebuild array of weights.
 * Add field to table node.
 */
function tow_table_field_add_form_submit($form, &$form_state) {

    // Define variables.
    $node = $form_state['values']['node'];
    $title = $form_state['values']['field_add']['title'];
    $possible_fields = tow_possible_fields();
    $type = $possible_fields[$form_state['values']['field_add']['type']];
    $unit = _tow_field_insert_fake($form_state['values']['field_add']['unit']);
    $weights = unserialize($node->field_weights[0]['value']);

    // Set weights.
    if ($weights) {
        $pos = 0;
        foreach ($possible_fields as $type_value) {
            foreach ($node->{'field_title_' . $type_value} as $field) {
                if ($field['value']) {
                    $pos++;
                }
            }
            if ($type == $type_value) {
                break;
            }
        }
        $weights_left = array_slice($weights, 0, $pos);
        $insert = array(max($weights) + 1);
        $weights_right = array_slice($weights, $pos);
        $weights = array_merge($weights_left, $insert, $weights_right);
    } else {
        $weights[] = 0;
    }
    $node->field_weights[0]['value'] = serialize($weights);

    if ($node->{'field_title_' . $type}[0]['value']) {
        $delta = count($node->{'field_title_' . $type});
    } else {
        $delta = 0;
    }

    $node->{'field_title_' . $type}[$delta] = array('value' => $title);

    if ($type == 'enum') {
        $enum_options = serialize(explode("\r\n", $form_state['values']['field_add']['enum_options']));
        $node->field_enum_options[$delta]['value'] = $enum_options;
    }

    $indexes = unserialize($node->field_indexes[0]['value']);
    $indexes[] = $form_state['values']['field_add']['index'];
    $node->field_indexes[0]['value'] = serialize($indexes);

    // Add unit to the units array.
    $node->field_units[] = array('value' => $unit);

    try {
        node_save($node);
    } catch (Exception $e) {
        watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
    }

    // Saving description.
    $description = trim($form_state['values']['field_add']['description']);
    _tow_field_description_save($node->nid, count($indexes) - 1, $description);
    drupal_set_message('Field has been added');
}

/**
 * Place fields user submitted into cck fields.
 * Save array of weights.
 */
function tow_table_submit_create($form, &$form_state) {

    // Predefine variables.
    $weights = $indexes = $units = array();
    $field_types = tow_possible_fields();

    // Save field values to form state.
    foreach ($field_types as $type_key => $type) {
        $delta = 0;
        $enum_delta = 0;
        foreach ($form_state['values']['fields']['data'] as $index => $field) {
            if ($field['type'] == $type_key) {
                $form_state['values']['field_title_' . $type][$delta]['value'] = $field['title'];
                $form_state['values']['field_title_' . $type][$delta]['_weight'] = $delta;
                if ($type == 'enum') {
                    $form_state['values']['field_enum_options'][$delta]['value'] = serialize(explode("\r\n", $field['enum_options']));
                    $form_state['values']['field_enum_options'][$delta]['_weight'] = $delta;
                    $enum_delta++;
                }
                $delta++;
                $weights[] = $index;
            }
        }
        for ($i = $form_state['values']['count_fields']; $i > $delta; $i--) {
            unset($form_state['values']['field_title_' . $type][$i]);
        }
        if ($type == 'enum') {
            for ($i = $form_state['values']['count_fields']; $i > $enum_delta; $i--) {
                unset($form_state['values']['field_enum_options'][$i]);
            }
        }
    }

    // Descriptions.
    global $_tow_descriptions;
    foreach ($form_state['values']['fields']['data'] as $field) {
        $indexes[] = $field['index'];
        $units[] = array('value' => _tow_field_insert_fake($field['unit']));

        // Preparing descriptions to be saved after node creation.
        $_tow_descriptions[] = trim($field['description']);
    }

    // Save weights, indexes and units.
    $form_state['values']['field_weights'][0]['value'] = serialize($weights);
    $form_state['values']['field_indexes'][0]['value'] = serialize($indexes);
    $form_state['values']['field_units'] = $units;
}

/**
 * Set corresponding CCK fields field values to be equal to values entered by user.
 */
function tow_table_submit_fields($form, &$form_state) {
    $form_state['values']['field_title_' . $type][$delta]['value'] = $value['title'];
}

/**
 * If user added feld, place this field title into corresponding CCK field.
 */
function tow_table_submit_field_add($form, &$form_state) {
    $title = $form_state['values']['field_add']['title'];

    if ($title) {
        $possible_fields = tow_possible_fields();
        $type = $possible_fields[$form_state['values']['field_add']['type']];
        $key = count($form_state['values']['field_title_' . $type]) - 2;
        $form_state['values']['field_title_' . $type][$key]['value'] = $title;
        if ($type == 'enum') {
            $enum_options = serialize(explode("\r\n", $form_state['values']['field_add']['enum_options']));
            $form_state['values']['field_enum_options'][$key]['value'] = $enum_options;
        }

        // Saving description.
        $description = trim($form_state['values']['field_add']['description']);
        _tow_field_description_save($form['#node']->nid, count($form_state['values']['fields']['data']), $description);
    }
}

/**
 * Save serialized array of indexes.
 */
function tow_table_submit_indexes($form, &$form_state) {

    // Data.
    foreach ($form_state['values']['fields']['data'] as $field) {
        $indexes[] = $field['index'];
    }

    // Title.
    if ($form_state['values']['field_add']['title']) {
        $indexes[] = $form_state['values']['field_add']['index'];
    }

    // Serialization.
    $form_state['values']['field_indexes'][0]['value'] = serialize($indexes);
}

/**
 * Modify the field unit values for the table
 */
function tow_table_submit_units($form, &$form_state) {

    // Get unit values.
    foreach ($form_state['values']['fields']['data'] as $field) {
        $units[] = _tow_field_insert_fake($field['unit']);
    }

    // Get title.
    if ($form_state['values']['field_add']['title']) {
        $units[] = $form_state['values']['field_add']['unit'];
    }

    // Set units.
    $units_array = array();
    foreach ($units as $index => $unit) {
        $units_array[] = array('value' => $unit);
    }
    $form_state['values']['field_units'] = $units_array;
}

/**
 * If table is being saved first time set weights to be empty array.
 * If there is a field to add, add one element to weights array (Rebuilding weights).
 */
function tow_table_submit_weights($form, &$form_state) {

    // Serialize weights.
    if (!$form_state['values']['field_weights'][0]['value']) {
        $weights = array();
        $form_state['values']['field_weights'][0]['value'] = serialize($weights);
    }

    // Rebuilding weights.
    if ($form_state['values']['field_add']['title']) {

        // Get weights.
        $weights = unserialize($form_state['values']['field_weights'][0]['value']);
        if ($weights) {

            // Get type.
            $field_types = tow_possible_fields();
            $type = $field_types[$form_state['values']['field_add']['type']];
            $pos = 0;

            // Run through field types.
            foreach ($field_types as $type_value) {

                // Find the position.
                if (!is_array($form_state['values']['field_title_' . $type_value])) {
                    continue;
                }
                foreach ($form_state['values']['field_title_' . $type_value] as $field) {
                    if (is_array($field) && $field['value']) {
                        $pos++;
                    }
                }

                // Match is found, break the loop.
                if ($type == $type_value) {
                    break;
                }
            }

            // Set weights.
            $weights_left = array_slice($weights, 0, $pos);
            $insert = array(max($weights) + 1);
            $weights_right = array_slice($weights, $pos);
            $weights = array_merge($weights_left, $insert, $weights_right);
        }

        // First entry.
        else {
            $weights[] = 0;
        }

        // Save weights.
        $form_state['values']['field_weights'][0]['value'] = serialize($weights);
    }
}

/**
 * Delete all records of a table.
 */
function tow_table_truncate_confirm_submit($form, &$form_state) {
    if ($form_state['values']['confirm']) {
        $res = db_query("SELECT nid FROM {content_type_record} WHERE field_table_value = %d", $form['nid']['#value']);
        while ($node_to_delete = db_fetch_object($res)) {
            node_delete($node_to_delete->nid);
        }
    }

    $form_state['redirect'] = 'node/' . $form['#dataset'];
}