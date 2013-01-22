<?php

/**
 * @file
 *
 * Field API.
 */
/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 *  Submits field descriptions into the database after table node was created.
 */
function tow_field_descriptions_submit($table_nid) {
    global $_tow_descriptions;
    if (is_array($_tow_descriptions)) {
        foreach ($_tow_descriptions as $index => $description) {
            _tow_field_description_save($table_nid, $index, $description);
        }
    }
    unset($_tow_descriptions);
}

/* * **************************************************************************************************
 * Helper functions
 * ************************************************************************************************** */

/**
 * Saves field description in the database.
 */
function _tow_field_description_save($table_nid, $index, $description, $is_new = TRUE) {
    if (!$is_new) {

        // Update.
        if ($description !== '') {
            db_query("

        UPDATE
          {tow_field_info}
            SET
              description = '%s'

              WHERE
                  nid = %d
                AND
                  ind = %d

      ", $description, $table_nid, $index);
        }

        // Delete.
        else {
            db_query("

        DELETE
          FROM
            {tow_field_info}

          WHERE
              nid =  %d
            AND
              ind = %d

      ", $table_nid, $index);
        }
    } else {

        // Insert.
        if ($description !== '')
            db_query("

        INSERT INTO
          {tow_field_info}
            (nid, ind, description)

            VALUES
              ('%d', '%d', '%s')

      ", $table_nid, $index, $description);
    }
}

/**
 * Checks whether value is empty and changes it by NULL placeholder if so.
 */
function _tow_field_insert_fake($value) {
    $placeholder = 'iWy2KupFUYKV4c9wmSrR';
    return (empty($value) && (string) $value !== '0' && $value !== FALSE) ? $placeholder : $value;
}

/**
 * Checks whether unit string is NULL placeholder and changes it by NULL if so.
 */
function _tow_field_remove_fake($value) {
    $placeholder = 'iWy2KupFUYKV4c9wmSrR';
    return ($value === $placeholder) ? NULL : $value;
}

/* * **************************************************************************************************
 * Forms
 * ************************************************************************************************** */

/**
 * Confirmation form for field deletion.
 */
function tow_field_delete_confirm($form_state, $table_nid, $type, $delta, $index) {

    global $user;
    global $_tow_table;
    global $_tow_dataset;

    // Error 403 if no access to table.
    $_tow_table = node_load($table_nid);
    if ($user->uid != $_tow_table->uid && !user_access('edit any table')) {
        drupal_access_denied();
        die();
    }

    // Error 404 if the field is missing.
    if (!$_tow_table->{'field_title_' . $type}[$delta]['value']) {
        drupal_not_found();
        die();
    }

    // Load parent dataset.
    $_tow_dataset = node_load($_tow_table->field_dataset[0]['value']);

    // Form.
    $form['type'] = array(
        '#type' => 'value',
        '#value' => $type
    );

    $form['delta'] = array(
        '#type' => 'value',
        '#value' => $delta
    );

    $form['field_index'] = array(
        '#type' => 'value',
        '#value' => $index
    );

    return confirm_form($form, t('Are you sure you want to delete field %field?', array('%field' => $_tow_table->{'field_title_' . $type}[$delta]['value'])), 'node/' . $table_nid, t('Deletion of field may cause loss of data'), t('Confirm'), t('Cancel'));
}

/**
 * Shows field description.
 */
function tow_field_description_callback($field_index, $table_nid) {

    // CHANGED
    $dataset_nid_res = db_query("SELECT field_dataset_value FROM {content_type_table} WHERE nid = %d", $table_nid);
    $dataset_nid = db_fetch_array($dataset_nid_res);
    $dataset_nid = $dataset_nid['field_dataset_value'];
    $dataset = node_load($dataset_nid);
    // END OF CHANGED

    $description_res = db_query("SELECT description FROM {tow_field_info} WHERE nid = %d AND ind = %d", $table_nid, $field_index);
    $description = '';
    while ($res = db_fetch_array($description_res)) {
        $description = $res['description'];
    }

    if ($description !== '') {
        $output = "<div>" . $description . "</div>";
    } else {
        $output = "<div>" . t("The owner hasn't provided any description for this field yet.") . "</div>";
    }

    return $output;
}

/**
 * Interface for field editing.
 */
function tow_field_edit_form($form_state, $table_nid, $type, $delta, $index) {

    global $user;
    global $_tow_table;
    global $_tow_dataset;

    // Error 403 if no access to table.
    $_tow_table = node_load($table_nid);
    if ($user->uid != $_tow_table->uid && !user_access('edit any table')) {
        drupal_access_denied();
        die();
    }

    // Error 404 if the field is missing.
    if (!$_tow_table->{'field_title_' . $type}[$delta]['value']) {
        drupal_not_found();
        die();
    }

    // Load parent dataset.
    $_tow_dataset = node_load($_tow_table->field_dataset[0]['value']);

    // Form.
    $form['current_type'] = array(
        '#type' => 'value',
        '#value' => $type
    );

    $form['delta'] = array(
        '#type' => 'value',
        '#value' => $delta
    );

    $form['field_index'] = array(
        '#type' => 'value',
        '#value' => $index
    );

    $form['title'] = array(
        '#type' => 'textfield',
        '#title' => t('Field title'),
        '#default_value' => $_tow_table->{'field_title_' . $type}[$delta]['value'],
    );

    $possible_fields = tow_possible_fields();

    $form['type'] = array(
        '#type' => 'select',
        '#title' => t('Type'),
        '#options' => $possible_fields,
        '#default_value' => array_search($type, $possible_fields)
    );

    $form['unit'] = array(
        '#type' => 'textfield',
        '#title' => t('Unit'),
        '#default_value' => _tow_field_remove_fake($_tow_table->field_units[$index]['value']),
        '#autocomplete_path' => 'tow/tags/autocomplete/Units',
    );

    // Read field description from database.
    $description_res = db_query("SELECT description FROM {tow_field_info} WHERE nid = %d AND ind = %d", $table_nid, $index);
    $description = '';
    while ($res = db_fetch_array($description_res)) {
        $description = $res['description'];
    }

    $form['current_description'] = array(
        '#type' => 'value',
        '#value' => $description
    );

    $form['description'] = array(
        '#type' => 'textarea',
        '#title' => t('Description'),
        '#default_value' => $description
    );

    $index_array = unserialize($_tow_table->field_indexes[0]['value']);
    $form['index'] = array(
        '#type' => 'checkbox',
        '#title' => t('Index'),
        '#default_value' => $index_array[$index]
    );

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save')
    );

    $form['#redirect'] = 'node/' . $table_nid;

    return $form;
}

/* * **************************************************************************************************
 * Callback functions
 * ************************************************************************************************** */

/**
 * Submission callback for deletion form.
 */
function tow_field_delete_confirm_submit($form, &$form_state) {

    global $_tow_table;

    $type = $form_state['values']['type'];
    $delta = $form_state['values']['delta'];
    $field_index = $form_state['values']['field_index'];

    // Delete field from the database.
    db_query("

    DELETE
      FROM
        {content_field_%s} f

      WHERE
          f.nid IN (

            SELECT
              r.nid AS nid

              FROM
                {content_type_record} r

              WHERE
                r.field_table_value = %d
          )
        AND
          f.delta = %d

  ", $type, $_tow_table->nid, $delta);

    unset($_tow_table->{'field_title_' . $type}[$delta]);
    if ($type = 'enum') {
        unset($_tow_table->field_enum_options[$delta]);
    }

    // Remove weight.
    $weights = unserialize($_tow_table->field_weights[0]['value']);
    $current_weight = $weights[$field_index];
    unset($weights[$field_index]);
    foreach ($weights as $i => $weight) {
        if ($weight > $current_weight) {
            $weights[$i]--;
        }
    }
    $_tow_table->field_weights[0]['value'] = serialize(array_values($weights));

    // Modify indexes array.
    $indexes = unserialize($_tow_table->field_indexes[0]['value']);
    unset($indexes[$field_index]);
    $_tow_table->field_indexes[0]['value'] = serialize($indexes);

    // Modify unit values.
    unset($_tow_table->field_units[$field_index]);

    // Save table.
    try {
        node_save($_tow_table);
    } catch (Exception $e) {
        watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
    }

    drupal_set_message('The field has been deleted');
    $form_state['redirect'] = 'node/' . $_tow_table->nid;
}

/**
 * Submission callback for edit form.
 */
function tow_field_edit_form_submit($form, &$form_state) {
    global $_tow_table;

    $table_nid = $_tow_table->nid;
    $field_type = $form_state['values']['current_type'];
    $delta = $form_state['values']['delta'];

    // Update index array of the table.
    $index_array = unserialize($_tow_table->field_indexes[0]['value']);
    $index_array[$form_state['values']['field_index']] = $form_state['values']['index'];
    $new_indexes = serialize($index_array);
    if ($new_indexes != $_tow_table->field_indexes[0]['value']) {
        $node_save = TRUE;
        $_tow_table->field_indexes[0]['value'] = $new_indexes;
    }

    // Update unit values of the table.
    $new_units_array = $_tow_table->field_units;
    $new_units_array[$form_state['values']['field_index']]['value'] = _tow_field_insert_fake($form_state['values']['unit']);
    if ($new_units_array != $_tow_table->field_units) {
        $node_save = TRUE;
        $_tow_table->field_units = $new_units_array;
        
        $voc_id = _tow_apachesolr_taxonomy_get_vid('Units');
        foreach ($new_units_array as $index => $unit) {
            $units_tag_object = _tow_taxonomy_get_term($unit['value'], $voc_id);
            if (!$units_tag_object) {
                _tow_add_taxonomy_term($unit['value'], $voc_id);
            }
        }
    }

    // Check if should save.
    if ($_tow_table->{'field_title_' . $field_type}[$delta]['value'] != $form_state['values']['title']) {
        $_tow_table->{'field_title_' . $field_type}[$delta]['value'] = $form_state['values']['title'];
        $node_save = TRUE;
    }

    // Save if necessary.
    if ($node_save) {
        try {
            node_save($_tow_table);
        } catch (Exception $e) {
            watchdog('ToW', $e->getMessage(), array(), WATCHDOG_ERROR);
        }
    }

    // Save description.
    $is_new = ($form_state['values']['current_description'] === '');
    if ($form_state['values']['current_description'] !== trim($form_state['values']['description'])) {
        _tow_field_description_save($table_nid, $form_state['values']['field_index'], trim($form_state['values']['description']), $is_new);
    }
}
