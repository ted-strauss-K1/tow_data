<?php

/**
 * @file
 *
 * Record API.
 */
/* * **************************************************************************************************
 * Hook handlers and forms
 * ************************************************************************************************** */

/**
 * Part of hook_form_alter() implementation.
 */
function tow_record_form_alter(&$form, &$form_state) {

    global $_tow_dataset;
    global $_tow_table;
    global $_tow_record;
    $_tow_record = TRUE;

    // Remove title, menu options, revision information, comment settings, publishing options.
    unset($form['title']);
    unset($form['menu']);
    unset($form['revision_information']);
    unset($form['comment_settings']);
    unset($form['options']);
    unset($form['buttons']['preview']);

    // Hide native CCK fields.
    foreach (tow_possible_fields() as $field_type) {
        $form['field_' . $field_type]['#access'] = FALSE;
    }

    // Get table node.
    if ($_GET['table']) {
        $_tow_table = node_load($_GET['table']);
    } else {
        $_tow_table = node_load($form['#node']->field_table[0]['value']);
    }
    if (!$_tow_table) {
        drupal_access_denied(); //access denied at node/add/record page and at node/add/record?table/nid when nid doesn't belong to table
        die();
        #return;
    }

    $_tow_dataset = node_load($_tow_table->field_dataset[0]['value']);
    $_tow_dataset = (object) array('nid' => $_tow_dataset->nid, 'title' => $_tow_dataset->title);

    $form['table_nid'] = array(
        '#type' => 'value',
        '#value' => $_tow_table->nid
    );

    $form['field_table']['#access'] = FALSE;

    // Add tabular form containing CCK fields ordered by weight.
    $form['fields'] = array(
        '#theme' => 'tow_table_tabular_form',
        '#title' => 'Fields',
        '#tree' => TRUE,
    );

    $form['fields']['header'] = array(
        '#type' => 'value',
        '#value' => array('Title', 'Type', 'Value'),
    );

    $form['delta'] = array(
        '#tree' => TRUE
    );

    $weights_array = unserialize($_tow_table->field_weights[0]['value']);
    if ($weights_array) {
        $index = 0;
        foreach (tow_possible_fields() as $field_type) {
            $delta = 0;
            if (!is_array($_tow_table->{'field_title_' . $field_type})) {
                continue;
            }
            foreach ($_tow_table->{'field_title_' . $field_type} as $title_value) {
                if ($title_value['value']) {
                    $aux_array[$weights_array[$index]]['title'] = $title_value['value'];
                    $aux_array[$weights_array[$index]]['type'] = $field_type;
                    $aux_array[$weights_array[$index]]['value'] = $form['field_' . $field_type][$delta]['#default_value']['value'];
                    $aux_array[$weights_array[$index]]['delta'] = $delta;
                    $index++;
                    $delta++;
                }
            }
        }
        ksort($aux_array);

        foreach ($aux_array as $key => $value) {
            $form['fields']['data'][$key]['title'] = array(
                '#type' => 'item',
                '#value' => $value['title'],
            );
            $form['fields']['data'][$key]['type'] = array(
                '#type' => 'item',
                '#value' => $value['type'],
            );

            switch ($value['type']) {
                case 'text':
                    $input_type = 'textarea';
                    break;
                case 'bool':
                    $value['value'] = $value['value'] === NULL ? '-1' : $value['value'];
                    $input_type = 'select';
                    break;
                case 'enum':
                    $input_type = 'select';
                    break;
                case 'timestamp':
                    $value['value'] = 'NOW()';
                    $input_type = 'textfield';
                default :
                    $input_type = 'textfield';
            }

            $form['fields']['data'][$key]['value'] = array(
                '#type' => $input_type,
                '#default_value' => $value['value'],
            );

            if ($value['type'] == 'enum') {
                $form['fields']['data'][$key]['value']['#options'] = array('-1' => '-----');
                $enum_options = unserialize($_tow_table->field_enum_options[$value['delta']]['value']);
                if (is_array($enum_options)) {
                    foreach ($enum_options as $option) {
                        $form['fields']['data'][$key]['value']['#options'][] = $option;
                    }
                }
            }


            if ($value['type'] == 'bool') {
                $form['fields']['data'][$key]['value']['#options'] = array('-1' => '-----', '1' => 'true', '0' => 'false');
            }

            $form['delta'][$key] = array(
                '#type' => 'value',
                '#value' => $value['delta']
            );

            if (!$form['field_' . $value['type']][$value['delta']]) {
                $form['field_' . $value['type']][$value['delta']] = $form['field_' . $value['type']][$value['delta'] - 1];
                $form['field_' . $value['type']][$value['delta']]['_weight']['#delta'] = $value['delta'];
                $form['field_' . $value['type']][$value['delta']]['#delta'] = $value['delta'];
                $form['field_' . $value['type']][$value['delta']]['#weight'] = $value['delta'];
            }
        }
    }

    // Specify form redirection.
    $args = arg();

    // For node/[record-nid]/edit.
    if ($args[2] == 'edit') {
        $form_state['redirect'] = 'table/' . $_tow_table->nid;
    }

    // For node/record/add.
    else {
        $form['#redirect'] = 'table/' . $_tow_table->nid;
    }

    // Register validation callbacks.
    $form['#validate'][] = 'tow_record_validate_edit';

    // Register submission callbacks.
    $form['#submit'][] = 'tow_record_submit_table';
    $form['#submit'][] = 'tow_record_submit_values';
}

/* * **************************************************************************************************
 * Callback functions
 * ************************************************************************************************** */

/**
 * Sets NID of table.
 */
function tow_record_submit_table($form, &$form_state) {
    $form_state['values']['field_table'][0]['value'] = $form_state['values']['table_nid'];
}

/**
 * Sets corresponding CCK fields values to be equal to values entered by user.
 */
function tow_record_submit_values($form, &$form_state) {
    if (!is_array($form_state['values']['fields']['data'])) {
        return;
    }
    foreach ($form_state['values']['fields']['data'] as $key => $value) {
        $type = $form['fields']['data'][$key]['type']['#value'];
        if ($type == 'bool') {
            $value['value'] = _tow_bool_to_int($value['value']);
        }
        if ($type == 'timestamp') {
            $value['value'] = time();
        }
        $delta = $form_state['values']['delta'][$key];
        $form_state['values']['field_' . $type][$delta]['_weight'] = $delta;
        $form_state['values']['field_' . $type][$delta]['value'] = _tow_field_insert_fake($value['value']);
    }
}

/**
 * Performs form validation during record editing/creation.
 */
function tow_record_validate_edit(&$form, &$form_state) {

    $error = FALSE;
    $err_field_names = array();
    $err_msgs = array();

    foreach ($form_state['values']['fields']['data'] as $key => $value_array) {
        $type = $form['fields']['data'][$key]['type']['#value'];

        switch ($type) {
            case 'int':
                $value = trim($value_array['value']);
                $form_state['values']['fields']['data'][$key]['value'] = $value;
                if (($value != '') && (!is_numeric($value) || !((int) $value == $value))) {
                    $error = TRUE;
                    $err_field_names[] = "fields][data][$key][value";
                    $err_msgs[] = t('Specified value is not integer.');
                }
                break;

            case 'float':
                $value = trim($value_array['value']);
                $form_state['values']['fields']['data'][$key]['value'] = $value;
                if (($value != '') && (!is_numeric($value))) {
                    $error = TRUE;
                    $err_field_names[] = "fields][data][$key][value";
                    $err_msgs[] = t('Specified value is not float.');
                }
                break;

            case 'char':
                break;

            case 'text':
                break;

            case 'enum':
                break;

            case 'bool':
                break;

            case 'time':
                $value = trim($value_array['value']);
                if (!preg_match("/^-?(([0-7]?[0-9]?[0-9])|([8][0-2][0-9])|([8][3][8])):([0-5]?[0-9])(:([0-5]?[0-9]))?$/", $value)) {
                    $error = TRUE;
                    $err_field_names[] = "fields][data][$key][value";
                    $err_msgs[] = t('Time must be entered in "HHH:MM:SS" or "HHH:MM" format.');
                }
                break;

            case 'code':
                $value = $value_array['value'];
                if (strlen($value) > 3) {
                    $error = TRUE;
                    $err_field_names[] = "fields][data][$key][value";
                    $err_msgs[] = t('Code field must not contain more than 3 symbols.');
                }
                break;
        }
    }

    // Showing error messages.
    _tow_show_validation_error($error, $err_field_names, $err_msgs);
}

/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Formats node content to be displayed on the record view page, which indexed by apachesolr.
 */
function tow_record_format_for_search_indexing(&$node) {

    $table_nid = $node->field_table[0]['value'];

    // Adding enum field value.
    unset($node->content['field_enum']);
    foreach ($node->field_enum as $delta => $value) {
        $enum_options = unserialize(db_result(db_query("

      SELECT
        t.field_enum_options_value AS field_enum_options_value

        FROM
          {content_field_enum_options} t

        WHERE
            t.nid = %d
          AND
            t.vid = %d
          AND
            t.delta = %d

    ", $table_nid, $table_nid, $delta)));
        $node->content[] = array('#value' => $enum_options[$value['value']] . ' ');
    }

    // Formating boolean field values.
    unset($node->content['field_bool']);
    foreach ($node->field_bool as $delta => $value) {
        if (isset($value['value'])) {
            $node->content[] = array('#value' => _tow_format_bool($value['value']) . ' ');
        }
    }

    // Substitute table_nid by table title.
    unset($node->content['field_table']);
    $node->content[] = array('#value' => db_result(db_query("

    SELECT
      n.title AS title

      FROM
        {node} n

      WHERE
          n.nid = %d
        AND
          n.vid = %d

  ", $table_nid, $table_nid)));
}

/**
 * Substitutes NULL placeholders by NULLs in node CCK fields.
 */
function tow_record_set_empty(&$node) {
    $types = content_types($node->type);
    foreach ($types['fields'] as $field) {
        if (isset($node->$field['field_name'])) {
            foreach ($node->$field['field_name'] as &$instance) {
                $instance['value'] = _tow_field_remove_fake($instance['value']);
            }
        }
    }
}
