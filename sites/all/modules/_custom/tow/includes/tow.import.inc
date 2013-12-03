<?php

/**
 * @file
 *
 * Functions for import.
 */
/* * **************************************************************************************************
 * Hook handlers
 * ************************************************************************************************** */

/**
 * Part of impelementation of hook_cron(). Clears old data from database table tow_import_progress.
 */
function tow_import_cron() {
    db_query("DELETE FROM {tow_import_progress} WHERE %d > (timestamp + 3600)", time());
}

/* * **************************************************************************************************
 * Forms
 * ************************************************************************************************** */

/**
 * Import form.
 */
function tow_import_form($form_state, $nid) {

    // Load dataset.
    global $_tow_dataset;
    $_tow_dataset = node_load($nid);
    if ($_tow_dataset->type !== 'dataset') {
        drupal_not_found();
        die();
    }

    // Add javascript sources.
    drupal_add_js('sites/all/libraries/jquery.filedrop/jquery.filedrop.js');
    drupal_add_js(drupal_get_path('module', 'tow') . '/js/import.js');

    // Set title.
    drupal_set_title($_tow_dataset->title);

    // Necessary parameters.
    $form['#attributes'] = array('enctype' => "multipart/form-data");
    $directory_path = file_directory_path();
    $d = file_check_directory($directory_path, FILE_CREATE_DIRECTORY, 'file_directory_path');

    // Dataset NID.
    $form['dataset'] = array(
        '#type' => 'hidden',
        '#value' => $nid,
    );

    // Hash.
    $form['hash'] = array(
        '#type' => 'hidden',
        '#value' => (isset($form_state['values']['hash']) ? $form_state['values']['hash'] : (isset($form_state['post']['hash']) ? $form_state['post']['hash'] : 0)),
    );

    // Way.
    $form['to'] = array(
        '#type' => 'item',
        '#tree' => TRUE,
        '#title' => t('Import files from your data to...'),
    );
    $form['to']['way'] = array(
        '#type' => 'radios',
        '#default_value' => 0,
        '#options' => array(t('Create new table in this dataset'), t('Insert now data into a table')),
    );
    $form['to']['table'] = array(
        '#type' => 'select',
        '#options' => tow_table_list($nid)
    );

    // Guide.
    $form['guide'] = array('#type' => 'item', '#value' => l('Learn about importing data', 'import_learn'));

    // File.
    $form['file'] = array(
        '#type' => 'item',
        '#title' => t('Choose files to import (only XLS, TXT & CSV formats accepted)'),
    );
    $form['file']['upload'] = array(
        '#type' => 'file',
        '#size' => 1,
        '#description' => t('or drag your file here')
    );
    $form['file']['or_url'] = array(
        '#type' => 'item',
        '#value' => t('or URL, where we can grab it')
    );
    $form['file']['url'] = array(
        '#type' => 'textfield'
    );
    $form['file']['get_it'] = array(
        '#type' => 'item',
        '#prefix' => '<div class="get-it">',
        '#suffix' => '</div>',
        '#value' => t('Get it!')
    );

    // Make rows.
    $rows = array(
        'Uploading',
        'Validating files',
        'Creating content',
    );
    foreach ($rows as $i => $row) {
        $rows[$i] = array(
            'class' => $i + 1,
            'data' => array(
                $i + 1 . '.',
                t($row),
                array('class' => 'dots', 'data' => NULL),
                array('class' => 'status', 'data' => NULL),
            ),
        );
    }
    $messages = array('Warnings', 'Keys');
    foreach ($messages as $message) {
        $i++;
        $rows[] = array(
            'class' => t(strtolower($message)) . ' ' . ($i + 1) . ' hidden',
            'data' => array(
                t($message) . ':',
                array('class' => 'message', 'data' => NULL)
            ),
        );
    }
    $rows[] = array(
        'class' => '6 hidden',
        'data' => array(
            '4. ' . t('Done'),
            array('class' => 'message', 'data' => NULL),
            array('class' => 'status', 'data' => NULL),
        ),
    );
    $rows[] = array(
        'class' => '7 hidden',
        'data' => array(l('New import', 'import/' . $_tow_dataset->nid) . t(' or back to your ') . l('dataset', 'node/' . $_tow_dataset->nid)),
    );

    // Progress.
    $form['progress'] = array(
        '#prefix' => '<div class="progress-bar hidden"><label class="progress-label">' . t('Progress') . '</label>',
        '#value' => theme('table', array(), $rows, array('class' => 'progress-table')),
        '#suffix' => '</div>'
    );

    // Submission.
    $form['submit'] = array('#type' => 'submit', '#value' => 'Submit', '#attributes' => array('class' => 'hidden'));

    return $form;
}

/**
 * Upload file and initialize progress.
 */
function tow_import_form_submit($form, &$form_state) {
    $file = file_save_upload('upload');
    if ($file) {
        $hash = $form_state['values']['hash'];
        $form_state['rebuild'] = TRUE;
        $table_nid = ($form_state['values']['to']['way'] == '0') ? 'false' : $form_state['values']['to']['table'];
        $dataset_nid = $form_state['values']['dataset'];
        $to = tow_import_check_request($hash, $dataset_nid, $table_nid);
        $filename = $file->filename;
        $copied = file_copy($file, 'import/' . $hash);

        tow_import_progress_init($hash, $copied, $filename, $to);
    }
}

/* * **************************************************************************************************
 * Progress
 * ************************************************************************************************** */

/**
 * Import progress.
 */
function tow_import_progress($hash, $stage, $action = 'increase', $total_stages = 10) {

    static $state;
    static $update_step;

    switch ($action) {

        // Fail.
        case 'fail':
            $state = -1;
            $update_step = 0;
            break;


        // Finish.
        case 'finish':
            if ($state >= 0) {
                $state = 10;
                $update_step = 0;
            }
            break;

        // Increase.
        case 'increase':
            $state += $update_step;
            break;

        // Message.
        case 'message':
            tow_import_progress_set($hash, $stage, $total_stages);
            return;

        // Set.
        case 'set':
            if ($state >= 0) {
                $state = 0;
                $update_step = 10 / $total_stages;
            }
            break;
    }
    tow_import_progress_set($hash, $stage, round($state));
}

/**
 * Progress delete stage actions.
 */
function tow_import_progress_delete($hash) {
    global $user;
    db_query("DELETE FROM {tow_import_progress} WHERE sid = '%s' AND hash = %d", $user->sid, $hash);
}

/**
 * Initialize progress.
 */
function tow_import_progress_init($hash, $copied, $filename, $to) {
    global $user;

    // Write a record about the progress to the database.
    db_query("INSERT INTO {tow_import_progress} (sid, hash, stage1, stage2, stage3, stage4, stage5, stage6, timestamp) VALUES('%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', %d)", $user->sid, $hash, '', '', '', '', '', '', time());

    // If file was copied start import.
    if ($copied) {
        tow_import_progress_set($hash, 1, 10);

        // Possible file processors.
        $phpexcel = 'phpexcel';
        $phpexcelreader = 'phpexcelreader';

        // Uncomment the method you want to use.
        tow_import_file_process_test($hash, $filename, $to, $phpexcel);
        # tow_import_file_process_test($hash, $filename, $to, $phpexcelreader);
    }

    // Else stop.
    else {
        tow_import_progress_set($hash, 1, -1);
        tow_import_progress_set($hash, 4, -1);
    }

    sleep(10);
}

/**
 * Set progress stage.
 */
function tow_import_progress_set($hash, $stage, $value) {
    global $user;
    db_query("UPDATE {tow_import_progress} SET stage%s = '%s' WHERE sid='%s' AND hash = %d", $stage, $value, $user->sid, $hash);
}

/* * **************************************************************************************************
 * File transfer interface
 * ************************************************************************************************** */

/**
 * Check dataset and table characteristics are set properly.
 */
function tow_import_check_request($hash, $dataset_nid, $table_nid) {

    // Check dataset and table on proper IDs and hash.
    if ((int) $hash == $hash && $hash) {
        $dataset = new stdClass;
        $dataset->nid = $dataset_nid;
        if ((int) $dataset->nid == $dataset->nid && $dataset->nid) {
            $table = new stdClass;
            $table->nid = $table_nid;
            if ($table->nid != 'false') {
                if ((int) $table->nid != $table->nid) {
                    die();
                }
            } else {
                unset($table);
            }
        } else {
            die();
        }
    } else {
        die();
    }

    // Check user acces to the dataset.
    global $user;
    $dataset->uid = db_result(db_query("SELECT uid FROM {node} WHERE nid = %d", $dataset->nid));
    if ($user->uid != $dataset->uid && !user_access('edit any table')) {
        die();
    }

    // Check table belongs to the dataset.
    if ($table) {
        $table->field_dataset_value = db_result(db_query("SELECT field_dataset_value FROM {content_type_table} WHERE nid = %d", $table->nid));
        if ($table->field_dataset_value != $dataset->nid)
            die();
    }
    else {
        $table = FALSE;
    }

    return array('dataset' => $dataset, 'table' => $table);
}

/**
 * Upload.
 */
function tow_import_upload($hash, $to) {

    // $_FILES.
    foreach ($_FILES['files'] as $key => $value) {
        $_FILES['files'][$key] = array('upload' => $value);
    }

    // Upload.
    $file = file_save_upload('upload');
    $filename = $file->filename;

    // Logging.
    $copied = file_copy($file, 'import/' . $hash);

    tow_import_progress_init($hash, $copied, $filename, $to);
}

/* * **************************************************************************************************
 * File processing
 * ************************************************************************************************** */

/**
 * Try to parse file and store info to the dataset.
 */
function tow_import_file_process_test($hash, $filename, $to, $processor = 'phpexcelreader') {
    watchdog('ToW Import', 'Import started: ' . date('Y-m-d H:i:s'));

    try {
        // Default column title, used in case if data file contains no column title.
        $default_column_title = "Column_";

        // File to be imported.
        $imported_file_name = 'sites/default/files/import/' . $hash;
        $info = pathinfo($filename);

        // Dataset NID.
        $dataset_nid = $to['dataset']->nid;

        // User ID.
        $uid = $to['dataset']->uid;

        if ($to['table']) {

            // Action to be done during import: create or append
            $import_action = "append";

            // Table NID.
            $table_nid = $to['table']->nid;
        } else {
            $import_action = "create";
        }

        $warning_messages = array();

        /* PROCESS */

        // Data reading and validating.
        $stage = 2;
        tow_import_progress($hash, $stage, 'set', 5);

        $function = '_tow_import_file_process_test_' . $processor;
        $data = $function($hash, $import_action, $imported_file_name, $warning_messages);

        // Extracting table name from file name.
        tow_import_progress($hash, $stage);

        if (!in_array($info['extension'], array('xls', 'xlsx'))) {
            $data['table_data'][0]['table_title'] = trim($info['filename']);
        }

        // Check whether a table with this titles already exists in the present dataset.
        tow_import_progress($hash, $stage);

        if ($import_action == 'create') {
            $existing_table_titles_res = db_query("

        SELECT
          n.title AS title

          FROM
              {node} n
            INNER JOIN
              {content_type_table} t
              ON
                  n.nid = t.nid
                AND
                  t.field_dataset_value = %d

      ", $dataset_nid);

            $existing_table_titles = array();
            while ($rs = db_fetch_array($existing_table_titles_res)) {
                $existing_table_titles[] = strtolower($rs['title']);
            }

            // Checking and changing table title.
            foreach ($data['table_data'] as &$table) {
                $table_name_index = 1;
                $table_title_unique = $table['table_title'];
                while (in_array(strtolower($table_title_unique), $existing_table_titles)) {
                    $table_title_unique = $table['table_title'] . '_' . $table_name_index;
                    $table_name_index++;
                }
                if ($table_name_index <> 1) {
                    $warning_messages[] = "Table \"" . $table['table_title'] . "\" already exists, new table title will be set to \"" . $table_title_unique . "\"";
                    $table['table_title'] = $table_title_unique;
                    $existing_table_titles[] = $table_title_unique;
                }
            }
            unset($table_title_unique, $rs, $table_name_index, $existing_table_titles_res, $existing_table_titles, $table);
        }

        $types = array_merge(tow_possible_fields(), array('null'));
        $types_id = array_flip($types);

        // Titles validation.
        tow_import_progress($hash, $stage);
        foreach ($data['table_data'] as $table_index => &$table) {

            if ((!$table['titles_cols_qty']) && ($import_action == 'create')) {

                // No titles, table to be created case.
                $warning_messages[] = "No data found for the table " . $data['table_data'][$table_index]['table_title'] . ". An empty table will be created.";
                $consider_column_titles_as_data = FALSE;
            } elseif ((!$table['rows_qty']) && ($import_action == 'append')) {

                // No data, table to be appended case.
                $warning_messages[] = "No data found in the imported file. Nothing will be appended.";
                tow_import_progress($hash, $stage, 'fail');
                $data['table_qty']--;
                unset($data['table_data'][$table_index]);
                break;
            } elseif (($table['rows_qty']) && ($import_action == 'append')) {

                // Data present, table to be appended case.
                $consider_column_titles_as_data = FALSE;
            } elseif ($table['rows_qty']) {

                // Titles present and data rows present, table to be created case.
                $consider_column_titles_as_data = FALSE;  // set to true if column names appear to be numbers or dates.
                //  Checking that column names are neither numbers nor dates.
                foreach ($table['column_titles'] as $key => &$column_title) {

                    // Trimmming current element in the array.
                    $column_title = trim($column_title);

                    // Old condition, includes taboo on column titles being a date.
                    // Works bad, because values like 'y' are treated as date.
                    if (is_numeric($column_title) || (strtolower($column_title) === "null")) {
                        $consider_column_titles_as_data = TRUE;
                        break;
                    }
                }
            } else {

                // Titles present, no data, table to be created case.
                $consider_column_titles_as_data = TRUE;
            }

            if ($consider_column_titles_as_data) {

                // Consider the first line from the file as data.
                array_unshift($table['str_value'], $table['column_titles']);

                // Increase row counter.
                $table['rows_qty']++;

                // Update column counter array...
                array_unshift($table['record_cols_qty'], $table['titles_cols_qty']);

                // ...and set new default column titles.
                unset($table['column_titles']);
                for ($col_index = 0; $col_index <= $table['titles_cols_qty'] - 1; $col_index++) {
                    $table['column_titles'][$col_index] = $default_column_title . $col_index;
                }
                $warning_messages[] = "No column titles found in data file, using default column titles";
            } else {

                // Set empty column titles to default name.
                foreach ($table['column_titles'] as $key => &$column_title) {
                    if ($column_title == '') {
                        $column_title = $default_column_title . $key;
                    }
                }
            }
        }
        // End of titles validation.
        // Data validation.
        tow_import_progress($hash, $stage);
        foreach ($data['table_data'] as $table_index => &$table) {

            // Detect type and count no of columns for each record.
            foreach ($table['str_value'] as $record_index => $record) {
                foreach ($record as $col_index => $cell) {

                    // Detect cell type, get its value of required type.
                    $res = _tow_import_detect_type($cell);
                    $data['table_data'][$table_index]['str_value'][$record_index][$col_index] = trim($cell);
                    $data['table_data'][$table_index]['value'][$record_index][$col_index] = $res['value'];
                    $data['table_data'][$table_index]['type'][$record_index][$col_index] = $res['type'];
                    $data['table_data'][$table_index]['type_id'][$record_index][$col_index] = $types_id[$res['type']];
                } // Cell loop.
            } // Row loop.
        }

        foreach ($data['table_data'] as $table_index => &$table) {

            // Data columns qty validation.
            // Create case.
            if ($import_action == 'create') {

                if ($table['rows_qty'] && (count(array_unique($table['record_cols_qty'])) - 1)) {
                    $warning_messages[] = "Records in the table " . $data['table_data'][$table_index]['table_title'] . " contain different number of columns. Table will not be created.";
                    $data['table_qty']--;
                    unset($data['table_data'][$table_index]);
                }

                // Data columns qty and titles qty validation.
                elseif ($table['rows_qty'] && ($table['record_cols_qty'][0] != $table['titles_cols_qty'])) {
                    $warning_messages[] = "The number of column titles in the table " . $data['table_data'][$table_index]['table_title'] . " does not match the number of columns in records. Table will not be created.";
                    $data['table_qty']--;
                    unset($data['table_data'][$table_index]);
                }

                // Column types validation.
                elseif ($table['rows_qty']) {
                    $invalid_types = FALSE;
                    foreach ($table['type'] as $record_index => $record_types) {
                        foreach ($record_types as $col_index => $cell_type) {
                            if (!$record_index) {

                                // First record in the table, its types will be used as first type option.
                                $table['col_type'][$col_index] = $cell_type;
                            }

                            // Determine and set as column type the 'greatest common' type for previous records
                            // and one consequent record.
                            elseif (!( $table['col_type'][$col_index] = _tow_import_common_type($cell_type, $table['col_type'][$col_index]))) {

                                // If does not exist, set error, stop iterations.
                                $invalid_types = TRUE;
                                break;
                            }
                        } // Cell loop.
                        if ($invalid_types) {
                            $warning_messages[] = "The types of values in different rows in the table " . $data['table_data'][$table_index]['table_title'] . " do not match each other. Table will not be created.";
                            unset($table);
                            $data['table_qty']--;
                            break;
                        }
                    } // Row loop.
                } else {

                    // Now rows case ( == no records in the table).
                    $data['table_data'][$table_index]['col_type'] = array();
                }
            }

            // Append case.
            else {
                $table['titles_cols_qty'] = $table['record_cols_qty'][0];

                $table_node_to_be_updated = node_load($table_nid);
                $weights_array = unserialize($table_node_to_be_updated->field_weights[0]['value']);
                $index = 0;

                foreach (tow_possible_fields() as $field_type) {
                    if (!is_array($table_node_to_be_updated->{'field_title_' . $field_type})) {
                        continue;
                    }
                    foreach ($table_node_to_be_updated->{'field_title_' . $field_type} as $title_value) {
                        if ($title_value['value']) {
                            $data['table_data'][$table_index]['col_type'][$weights_array[$index]] = $field_type;
                            $index++;
                        }
                    }
                }
                ksort($data['table_data'][$table_index]['col_type']);

                $invalid_types = FALSE;
                foreach ($table['type'] as $record_index => $record_types) {
                    foreach ($record_types as $col_index => $cell_type) {
                        if ($cell_type != $data['table_data'][$table_index]['col_type'][$col_index]) {
                            $invalid_types = TRUE;
                            break;
                        }
                    } // Cell loop.
                    if ($invalid_types) {
                        $warning_messages[] = "The types of imported data do not match that of an existing table. No data will be appended.";
                        unset($data['table_data'][$table_index]);
                        $data['table_qty']--;
                        break;
                    }
                } // Row loop.
            }
        } // Table loop.
        unset($table);
        // End of data validation.

        if ($data['table_qty']) {
            tow_import_progress($hash, $stage, 'finish');
            $progress_indicator = 1;
        } else {
            tow_import_progress($hash, $stage, 'fail');
        }

        // Creating content.
        $stage = 3;

        // Count total rows qty for progress indicator updating.
        $total_rows = 0;
        foreach ($data['table_data'] as $table) {
            $total_rows += $table['rows_qty'];
        }
        tow_import_progress($hash, $stage, 'set', $total_rows + 1);

        if ($import_action == 'create') {

            // Reading key titles from other tables.
            $existing_table_nids_res = db_query("

        SELECT
          t.nid AS nid

          FROM
            {content_type_table} t

          WHERE
            t.field_dataset_value = %d

      ", $dataset_nid);

            $column_titles_keys = array();
            while ($rs = db_fetch_array($existing_table_nids_res)) {
                $table_node = node_load($rs['nid']);
                $weights_array = unserialize($table_node->field_weights[0]['value']);

                $index = 0;
                foreach (tow_possible_fields() as $field_type) {
                    if (!is_array($table_node->{'field_title_' . $field_type})) {
                        continue;
                    }
                    foreach ($table_node->{'field_title_' . $field_type} as $title_value) {
                        if ($title_value['value']) {
                            $aux_array[$weights_array[$index]]['value'] = $title_value['value'];
                            $index++;
                        }
                    }
                }
                ksort($aux_array);
                $indexes_array = unserialize($table_node->field_indexes[0]['value']);
                foreach ($indexes_array as $key => $is_index) {
                    if ($is_index) {
                        $column_titles_keys[] = $aux_array[$key]['value'];
                    }
                }
            }
            unset($existing_table_nids_res, $rs, $table_node, $weights_array, $indexes_array);
        }

        tow_import_progress($hash, $stage);

        $created_table_titles = array();
        $created_table_nids = array();
        $found_key_titles = array();
        foreach ($data['table_data'] as $table_index => &$table) {

            if ($import_action == 'create') {

                // Fill in new table node
                $new_table = new stdClass();
                $new_table->type = 'table';
                $new_table->title = $table['table_title'];
                $new_table->uid = $uid;
                $new_table->body = "The table was imported from file \"" . $info['filename'] . '.' . $info['extension'] . '".';
                $new_table->teaser = $new_table->body;

                // Specify columns, their types and titles.
                $column_type = array();
                $indexes_array = array();

                for ($col_index = 0; $col_index <= $table['titles_cols_qty'] - 1; $col_index++) {

                    // Types of columns.
                    $column_type[$col_index]['type_id'] = $types_id[$table['col_type'][$col_index]];
                    $field_type = "field_title_" . $table['col_type'][$col_index];
                    $column_type[$col_index]['type'] = $table['col_type'][$col_index];

                    // Column titles.
                    $new_table->{$field_type}[]['value'] = $table['column_titles'][$col_index];

                    //Units
                    $new_table->field_units[]['value'] = 'iWy2KupFUYKV4c9wmSrR';

                    // Enum options if needed.
                }

                // Weights generate.
                $weights_array = array();
                foreach ($types as $type_id => $type) {
                    foreach ($table['col_type'] as $index => $col_type) {
                        if ($col_type == $type) {
                            $delta++;
                            $weights_array[] = $index;
                        }
                    }
                }

                // Searching for keys.
                $indexes_array = array();
                foreach ($table['column_titles'] as $col_index => $col_title) {
                    if (in_array($col_title, $column_titles_keys)) {
                        $indexes_array[] = 1;
                        $found_key_titles[] = $col_title;
                    } else {
                        $indexes_array[] = 0;
                    }
                }

                $new_table->field_weights[0]['value'] = serialize($weights_array);
                $new_table->field_dataset[0]['value'] = $dataset_nid;
                $new_table->field_indexes[0]['value'] = serialize($indexes_array);

                // Node save, get table nid.
                try {
                    db_query("INSERT INTO {node} (type, title, uid, created, changed) VALUES ('%s', '%s', %d, %d, %d)", $new_table->type, $new_table->title, $new_table->uid, time(), time());
                    $nid = db_last_insert_id('node', 'nid');
                    db_query("INSERT INTO {node_revisions} (nid, title, body, teaser, uid, log) VALUES (%d, '%s', '%s', '%s', %d, '%s')", $nid, $new_table->title, $new_table->body, $new_table->teaser, $new_table->uid, $new_table->title);
                    $vid = db_last_insert_id('node_revisions', 'vid');
                    db_query("UPDATE {node} SET vid = %d WHERE nid = %d", $vid, $nid);
                    $types = tow_possible_fields();
                    foreach ($types as $type) {
                        if (isset($new_table->{'field_title_' . $type})) {
                            foreach ($new_table->{'field_title_' . $type} as $delta => $value) {
                                db_query("INSERT INTO {content_field_title_$type} (nid, vid, delta, field_title_" . $type . "_value) VALUES (%d, %d, %d, '%s')", $nid, $vid, $delta, $value['value']);
                            }
                        }
                    }
                    if (isset($new_table->field_units)) {
                        foreach ($new_table->field_units as $delta => $value) {
                            db_query("INSERT INTO {content_field_units} (nid, vid, delta, field_units_value) VALUES (%d, %d, %d, '%s')", $nid, $vid, $delta, $value['value']);
                        }
                    }
                    if (is_null($new_table->field_available_positions[0]['value'])) {
                        $available_positions = array();
                        $max_rec = variable_get('tow_max_records_in_table', 920);
                        for ($i = 1; $i < $max_rec; $i++) {
                            $available_positions[] = $i;
                        }
                        $available_positions = serialize($available_positions);
                    }
                    db_query("INSERT INTO {content_type_table} (nid, vid, field_weights_value, field_indexes_value, field_dataset_value, field_available_positions_value) VALUES (%d, %d, '%s', '%s', '%s', '%s')", $nid, $vid, $new_table->field_weights[0]['value'], $new_table->field_indexes[0]['value'], $new_table->field_dataset[0]['value'], $available_positions);
                    $new_table->nid = $nid;
                } catch (Exception $e) {
                    watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
                }
                $table_nid = $new_table->nid;

                if ($table_nid) {
                    $created_table_nids[] = $table_nid;
                    $created_table_titles[] = $new_table->title;
                }
            }

            $now = time();
            $available_positions = unserialize($available_positions);
            $content_type_record = array(
                'sql' => "INSERT INTO {content_type_record} (nid, vid, field_table_value, field_position_value) VALUES ",
            );
            $types = tow_possible_fields();
            foreach ($types as $type) {
                $fieldtypes[$type] = array(
                    'sql' => "INSERT INTO {content_field_$type} (nid, vid, delta, field_" . $type . "_value) VALUES ",
                );
            }
            $to_index = array(
                'sql' => array(
                    'mongo' => "INSERT INTO {tow_mongodb_index} (nid, changed) VALUES ($table_nid, $now), ",
                    'solr' => "INSERT INTO {apachesolr_search_node} (nid, changed) VALUES ($table_nid, $now), ",
                )
            );
            for ($record_index = 0; $record_index < $table['rows_qty']; $record_index++) {

                // Type.
                $new_record = new stdClass();
                $new_record->type = 'record';
                $new_record->uid = $uid;

                // Data.
                for ($col_index = 0; $col_index <= $table['titles_cols_qty'] - 1; $col_index++) {
                    if (isset($table['value'][$record_index][$col_index])) {
                        $value = _tow_import_reduce_type($table['col_type'][$col_index], $table['value'][$record_index][$col_index]);
                        $new_record->{"field_" . $table['col_type'][$col_index]}[$col_index]['value'] = $value;
                    } else {
                        $new_record->{"field_" . $table['col_type'][$col_index]}[$col_index]['value'] = _tow_field_insert_fake(NULL);
                    }
                }
                $new_record->field_table[0]['value'] = $table_nid;
                try {
                    db_query("INSERT INTO {node} (type, uid, created, changed) VALUES ('%s', %d, %d, %d)", $new_record->type, $new_record->uid, time(), time());
                    $nid = db_last_insert_id('node', 'nid');
                    db_query("INSERT INTO {node_revisions} (nid, body, teaser, uid, log) VALUES (%d, '%s', '%s', %d, '%s')", $nid, "$nid", "$nid", $new_table->uid, "$nid");
                    $vid = db_last_insert_id('node_revisions', 'vid');
                    db_query("UPDATE {node} SET vid = %d WHERE nid = %d", $vid, $nid);
                    foreach ($types as $type) {
                        if (isset($new_record->{'field_' . $type})) {
                            $delta = 0;
                            foreach ($new_record->{'field_' . $type} as $value) {
                                $value = _tow_field_remove_fake($value['value']);
                                if (is_null($value)) {
                                    $fieldtypes[$type]['sql'] .= "($nid, $vid, $delta, NULL), ";
                                } else {
                                    $value = db_escape_string($value);
                                    $fieldtypes[$type]['sql'] .= "($nid, $vid, $delta, '$value'), ";
                                }
                                $delta++;
                            }
                        }
                    }
                    $new_record->field_position[0]['value'] = array_shift($available_positions);
                    $pos = db_escape_string($new_record->field_position[0]['value']);
                    $content_type_record['sql'] .= "($nid, $vid, '$table_nid', '$pos'), ";
                    $to_index['sql']['mongo'] .= "($nid, $now), ";
                    $to_index['sql']['solr'] .= "($nid, $now), ";
                } catch (Exception $e) {
                    watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
                }
                tow_import_progress($hash, $stage);
            }

            db_query("UPDATE {content_type_table} SET field_available_positions_value = '%s' WHERE nid = %d", serialize($available_positions), $new_record->field_table[0]['value']);
            db_query(rtrim($content_type_record['sql'], ', '));
            foreach ($types as $type) {
                if (isset($new_record->{'field_' . $type})) {
                    db_query(rtrim($fieldtypes[$type]['sql'], ', '));
                }
            }
            db_query(rtrim($to_index['sql']['mongo'], ', '));
            db_query(rtrim($to_index['sql']['solr'], ', '));
            unset($new_table);
            unset($table_nid);
        }

        tow_import_progress($hash, $stage, 'finish');

        $stage = 4;
        if ($warning_messages) {
            foreach ($warning_messages as $value) {
                $warning .= $value . '; ';
            }
            $warning = substr($warning, 0, -2);
            tow_import_progress($hash, $stage, 'message', $warning);
        } else {
            tow_import_progress($hash, $stage, 'fail');
        }

        $stage = 5;
        if ($found_key_titles) {
            foreach ($found_key_titles as $value) {
                $found_keys .= $value . '; ';
            }
            $found_keys = substr($found_keys, 0, -2);
            tow_import_progress($hash, $stage, 'message', 'we found ' . count($found_key_titles) . ' key fields (' . $found_keys . ')');
        } elseif (($progress_indicator < 0) || ($import_action == 'append')) {
            tow_import_progress($hash, $stage, 'fail');
        } else {
            tow_import_progress($hash, $stage, 'message', "we found no key fields");
        }

        $stage = 6;
        if ($import_action == 'create') {
            if ($created_table_titles) {
                foreach ($created_table_titles as $value) {
                    $created_tables .= $value . '; ';
                    $created_tables_spaced .= $value . ' ';
                }
                foreach ($created_table_nids as $value) {
                    $created_nids_spaced .= $value . ' ';
                }
                $created_tables = substr($created_tables, 0, -2);
                tow_import_progress($hash, $stage, 'message', $created_tables . ' created./ ' . $created_tables_spaced . '/ ' . $created_nids_spaced);
            } else {
                tow_import_progress($hash, $stage, 'message', "no tables created");
            }
        } else {
            if (($progress_indicator >= 0)) {
                tow_import_progress($hash, $stage, 'message', $table_node_to_be_updated->title . ' updated./ ' . $table_node_to_be_updated->title . '/ ' . $table_node_to_be_updated->nid);
            } else {
                tow_import_progress($hash, $stage, 'message', "No data was appended.");
            }
        }
    } catch (Exeption $e) {
        tow_import_progress($hash, $stage, 'fail');
        watchdog('ToW Import', 'Import failed: ' . date('Y-m-d H:i:s'));
    }

    watchdog('ToW Import', 'Import finished: ' . date('Y-m-d H:i:s'));
}

/**
 * Parsing Excel file with PHPExcel library.
 */
function _tow_import_file_process_test_phpexcel(&$hash, &$import_action, &$imported_file_name, &$warning_messages) {
    // PHPExcel_IOFactory data reading engine.
    require_once 'sites/all/libraries/PHPExcelExtension/PHPExcel/IOFactory.php';

    // Reading files.
    // Automatically detect filetype and load it into the $obj_php_excel object.
    // Detect filetype.
    $cache_method = PHPExcel_CachedObjectStorageFactory::cache_to_phpTemp;
    $cache_settings = array();
    PHPExcel_Settings::setCacheStorageMethod($cache_method, $cache_settings);

    $input_file_type = PHPExcel_IOFactory::identify("$imported_file_name");

    // Create reader of detected type.
    $obj_reader = PHPExcel_IOFactory::createReader("$input_file_type");

    // Set properties of Reader object depending on its type.
    // Read no formatting.
    if (method_exists($obj_reader, 'setReadDataOnly')) {
        $obj_reader->setReadDataOnly(TRUE);
    }
    if (method_exists($obj_reader, 'enableMemoryOptimization()')) {
        $obj_reader->enableMemoryOptimization();
    }

    // TXT-file. Value delimiter may be tab or comma.
    if (($input_file_type == 'CSV') /* && (strtolower($info['extension']) == "txt") */) {

        // Read first line.
        $file = fopen($imported_file_name, 'r');
        $first_row = fgets($file);
        fclose($file);

        // Determine value delimiter.
        if (!strpos($first_row, ',')) {
            $obj_reader->setDelimiter("\t");
        } elseif (!strpos($first_row, "\t")) {
            $obj_reader->setDelimiter(",");
        } else {

            // Default value.
            $obj_reader->setDelimiter("\t");
        }

        // Set value enclosure.
        $obj_reader->setEnclosure('');
        $obj_php_excel = $obj_reader->load("$imported_file_name");
    } else {
        $obj_php_excel = PHPExcel_IOFactory::load("$imported_file_name");
    }

    // Working with active sheet
    $worksheet_index = 0;
    $data = array();

    foreach ($obj_php_excel->getWorksheetIterator() as $worksheet) {

        // Case append.
        if ($import_action == 'append' && $worksheet_index) {

            $warning_messages[] = '0nly first sheet will be inserted into table';
            break;
        }

        // New table titles.
        $data['table_data'][$worksheet_index]['table_title'] = trim($worksheet->getTitle());
        $data['table_data'][$worksheet_index]['record_cols_qty'] = array();
        $data['table_data'][$worksheet_index]['str_value'] = array();
        $data['table_data'][$worksheet_index]['column_titles'] = array();

        $row_index = 0;
        foreach ($worksheet->getRowIterator() as $row) {
            $cell_iterator = $row->getCellIterator();
            $cell_iterator->setIterateOnlyExistingCells(FALSE);

            // Loop through empty cells as well.
            if (($row->getRowIndex() == 1) && ($import_action == 'create')) {
                foreach ($cell_iterator as $cell) {
                    $is_date_time = PHPExcel_Shared_Date::isDateTime($cell);
                    if ($is_date_time) {
                        $timestamp = PHPExcel_Shared_Date::ExcelToPHP($cell->getValue());
                        $cellvalue = date("Y-m-d H:i:s", $timestamp - date('Z', $timestamp));
                    } else {
                        $cellvalue = $cell->getValue();
                    }
                    $data['table_data'][$worksheet_index]['column_titles'][] = $cellvalue;
                }
            } else {
                foreach ($cell_iterator as $cell) {
                    $is_date_time = PHPExcel_Shared_Date::isDateTime($cell);
                    if ($is_date_time) {
                        $timestamp = PHPExcel_Shared_Date::ExcelToPHP($cell->getValue());
                        $cellvalue = date("Y-m-d H:i:s", $timestamp - date('Z', $timestamp));
                    } else {
                        $cellvalue = $cell->getValue();
                    }
                    $data['table_data'][$worksheet_index]['str_value'][$row_index][] = $cellvalue;
                }

                $data['table_data'][$worksheet_index]['record_cols_qty'][$row_index] = count($data['table_data'][$worksheet_index]['str_value'][$row_index]);
                $row_index++;
            } // Cell loop.
        } // Row loop.
        $data['table_data'][$worksheet_index]['rows_qty'] = $row_index;
        if ((count($data['table_data'][$worksheet_index]['column_titles']) == 1) && ($data['table_data'][$worksheet_index]['column_titles'][0] == 'null')) {
            $data['table_data'][$worksheet_index]['titles_cols_qty'] = 0;
            unset($data['table_data'][$worksheet_index]['column_titles'][0]);
        } else {
            $data['table_data'][$worksheet_index]['titles_cols_qty'] = count($data['table_data'][$worksheet_index]['column_titles']);
        }

        $worksheet_index++;
    } // Worksheet loop.
    $data['table_qty'] = $worksheet_index;

    $obj_php_excel->disconnectWorksheets();
    unset($obj_php_excel, $obj_reader);
    file_delete('sites/default/files/import/' . $hash);

    return $data;
}

/**
 * Parsing Excel file with PHPExcelReader library.
 */
function _tow_import_file_process_test_phpexcelreader(&$hash, &$import_action, &$imported_file_name, &$warning_messages) {

    // Include PHPExcelReader library.
    require_once 'sites/all/libraries/PHPExcelReader/excel_reader2.php';

    // Reading files.
    // Automatically detect filetype and load it into the $obj_php_excel object.
    // Detect filetype.
    $php_excel_reader = new Spreadsheet_Excel_Reader("$imported_file_name");

    $data = array();
    foreach ($php_excel_reader->boundsheets as $worksheet_index => $worksheet) {
        if ($import_action == 'append' && $worksheet_index) {
            $warning_messages[] = '0nly first sheet will be inserted into table';
            break;
        }

        // New table titles.
        $data['table_data'][$worksheet_index]['table_title'] = trim($worksheet['name']);
        $data['table_data'][$worksheet_index]['record_cols_qty'] = array();
        $data['table_data'][$worksheet_index]['str_value'] = array();
        $data['table_data'][$worksheet_index]['column_titles'] = array();

        $row_index = 0;
        $data['table_qty'] = 0;
        foreach ($php_excel_reader->sheets[$worksheet_index]['cells'] as $row => $cell) {
            foreach ($cell as $cell_index => $cellvalue) {
                if (($row == 1) && ($import_action == 'create')) {
                    $data['table_data'][$worksheet_index]['column_titles'][] = $cellvalue;
                } else {
                    $data['table_data'][$worksheet_index]['value'][$row_index][] = $cellvalue;
                    $data['table_data'][$worksheet_index]['str_value'][$row_index][] = $cellvalue;
                    $data['table_data'][$worksheet_index]['record_cols_qty'][$row_index] = count($data['table_data'][$worksheet_index]['str_value'][$row_index]);
                }
            }

            if ($row != 1) {
                $row_index++;
            }
        }
        $data['table_data'][$worksheet_index]['rows_qty'] = $row_index;
        if ((count($data['table_data'][$worksheet_index]['column_titles']) == 1) && ($data['table_data'][$worksheet_index]['column_titles'][0] == 'null')) {
            $data['table_data'][$worksheet_index]['titles_cols_qty'] = 0;
            unset($data['table_data'][$worksheet_index]['column_titles'][0]);
        } else {
            $data['table_data'][$worksheet_index]['titles_cols_qty'] = count($data['table_data'][$worksheet_index]['column_titles']);
        }
        $data['table_qty']++;
    }

    // txt-file. Value delimiter may be tab or comma.
    if (($input_file_type == 'CSV') && (strtolower($info['extension']) == "txt")) {

        # TODO: csv & txt
    }

    unset($php_excel_reader);
    file_delete('sites/default/files/import/' . $hash);

    return $data;
}

/* * **************************************************************************************************
 * Helper functions
 * ************************************************************************************************** */

/**
 * Determines the 'greatest common' type of two input types.
 */
function _tow_import_common_type($type1, $type2) {
    $in_type1 = strtolower(trim($type1));
    $in_type2 = strtolower(trim($type2));
    if ($in_type1 === $in_type2) {
        return $in_type1;
    };

    $common_types = array(
        'bool' => array(
            'int' => 'int',
            'float' => 'float',
            'null' => 'bool',
            'date' => 'text',
            'time' => 'text',
            'datetime' => 'text',
            'text' => 'text',
        ),
        'int' => array(
            'bool' => 'int',
            'float' => 'float',
            'null' => 'int',
            'date' => 'text',
            'time' => 'text',
            'datetime' => 'text',
            'text' => 'text',
        ),
        'float' => array(
            'bool' => 'float',
            'int' => 'float',
            'null' => 'float',
            'date' => 'text',
            'time' => 'text',
            'datetime' => 'text',
            'text' => 'text',
        ),
        'null' => array(
            'bool' => 'bool',
            'int' => 'int',
            'float' => 'float',
            'date' => 'date',
            'time' => 'time',
            'datetime' => 'datetime',
            'text' => 'text',
        ),
        'date' => array(
            'bool' => 'text',
            'int' => 'text',
            'float' => 'text',
            'null' => 'date',
            'time' => 'datetime',
            'datetime' => 'datetime',
            'text' => 'text',
        ),
        'time' => array(
            'bool' => 'text',
            'int' => 'text',
            'float' => 'text',
            'null' => 'date',
            'date' => 'datetime',
            'datetime' => 'datetime',
            'text' => 'text',
        ),
        'datetime' => array(
            'bool' => 'text',
            'int' => 'text',
            'float' => 'text',
            'null' => 'date',
            'time' => 'datetime',
            'date' => 'datetime',
            'text' => 'text',
        ),
        'text' => array(
            'bool' => 'text',
            'int' => 'text',
            'float' => 'text',
            'null' => 'text',
            'date' => 'text',
            'time' => 'text',
            'datetime' => 'text',
        ),
    );

    if (isset($common_types[$in_type1][$in_type2])) {
        return $common_types[$in_type1][$in_type2];
    } else {
        return 0;
    }
}

/**
 * Detects the type of input value.
 *
 * @param
 *   string $str_value
 *
 * @return
 *   array(
 *    'type' => 'bool', 'int', float', 'null', 'date' or 'text',
 *    'value' => formatted value
 *   )
 */
function _tow_import_detect_type($str_value) {
    $value = strtolower(trim($str_value));
    $res['value'] = $str_value;

    // Boolean.
    if (($value == 'true') || ($value == 'FALSE') || ($value == 'yes') || ($value == 'no') || ($value == '1') || ($value == '0')) {
        $res['type'] = "bool";
    }

    // Numeric: integer or float.
    elseif (is_numeric($value)) {
        if ((integer) $value == $value) {
            $res['type'] = "int";
        } else {
            $res['type'] = "float";
        }
    }

    // Date.
    elseif ($timestamp = strtotime($value)) {
        if (abs($timestamp - time()) <= (24 * 60 * 60)) {
            $res['type'] = "time";
        } elseif (date('His', $timestamp) == '000000') {
            $res['type'] = "date";
        } else {
            $res['type'] = "datetime";
        }
        $res['value'] = $value;
    }

    // Null.
    elseif (($value === "null") || ($value == '')) {
        $res['type'] = "null";
        $res['value'] = NULL;
    }

    // Text.
    else {
        $res['type'] = "text";
    }

    return $res;
}

/**
 * Reduces value to specific type.
 */
function _tow_import_reduce_type($type, $value) {
    switch ($type) {

        case 'int':
            return (integer) $value;

        case 'float':
            return (float) $value;

        case 'bool':
            $value = strtolower(trim($value));
            if ($value == 'true' || $value == 'yes' || $value == '1') {
                return 1;
            } else {
                return 0;
            }

        case 'date':
            return date("Y-m-d", strtotime($value));

        case 'time':
            return date("H:i:s", strtotime($value));

        case 'datetime':
            return date("Y-m-d H:i:s", strtotime($value));

        case 'char':
        case 'text':
        default:
            return $value;
    }
}
