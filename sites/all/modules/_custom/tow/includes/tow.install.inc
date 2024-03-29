<?php

/**
 * @file
 *
 * Installation auxiliary functions.
 */
/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Creates dataset categories if necessary.
 */
function tow_install_create_dataset_categories() {

    // Get 'Dataset categories' vocabulary ID.
    $vid = db_result(db_query("

    SELECT
      v.vid AS vid

      FROM
        {vocabulary} v

      WHERE
        v.name = '%s'

  ", 'Dataset categories'));

    // If does not exist.
    if (!$vid) {
        $empty = TRUE;
        $not_created = TRUE;
    }

    // Check empty.
    else {
        $not_created = FALSE;
        $empty = (db_result(db_query("

      SELECT
        COUNT(*)

        FROM
          {term_data} td

        WHERE
          td.vid = %d

    ", $vid))) ? FALSE : TRUE;
    }

    // Needs to be filled.
    if ($empty) {

        // Needs in creation.
        if ($not_created) {
            $vocabulary = array(
                'name' => t('Dataset categories'),
                'description' => '',
                'help' => '',
                'relations' => 1,
                'hierarchy' => 0,
                'multiple' => 0,
                'required' => 0,
                'tags' => 0,
                'module' => 'tow',
                'weight' => 0,
                'nodes' => array('dataset' => 'dataset'),
            );
            taxonomy_save_vocabulary($vocabulary);
            $vid = $vocabulary['vid'];
        }

        // Create terms.
        $names = _tow_install_taxonomy_dataset_categories();
        foreach ($names as $name) {
            $term = array(
                'vid' => $vid,
                'name' => $name,
            );
            taxonomy_save_term($term);
        }
    }
}

/**
 * This function defines premie's fields.
 */
function tow_install_fields() {

    // Initialisation.
    $fields = array();
    $weight = tow_cck_init_weight();

    /* Dataset fields. */
    $format = array(1 => 'hidden', 2 => 'hidden', 3 => 'hidden', 4 => 'default', 'full' => 'hidden', 'teaser' => 'hidden', 'label' => 'hidden', 'token' => 'default');
    $exclude = array(1 => 0, 2 => 0, 3 => 0, 4 => 0, 'full' => 1, 'teaser' => 1, 'label' => 1, 'token' => 0);
    $allowed_values = '
    0|Open
    1|By request
    2|Private
  ';
    $fields[] = tow_cck_integer('field_access_type', 'dataset', t('Access Type'), $weight++, $format, $exclude, 'optionwidgets_select', 1, 0, $allowed_values);
    $format = array(1 => 'default', 2 => 'default', 3 => 'default', 4 => 'default', 'full' => 'plain', 'teaser' => 'plain', 'label' => 'hidden', 'token' => 'default');
    $exclude = array(1 => 0, 2 => 0, 3 => 0, 4 => 0, 'full' => 0, 'teaser' => 0, 'label' => 0, 'token' => 0);
    $fields[] = tow_cck_textarea('field_hash', 'dataset', t('Hash'), $weight++, $format, $exclude);

    /* Table fields. */
    $format = array(1 => 'default', 2 => 'default', 3 => 'default', 4 => 'default', 'full' => 'hidden', 'teaser' => 'hidden', 'label' => 'hidden', 'token' => 'default');
    $exclude = array(1 => 0, 2 => 0, 3 => 0, 4 => 0, 'full' => 1, 'teaser' => 1, 'label' => 1, 'token' => 0);
    $fields[] = tow_cck_textfield('field_title_int', 'table', t('Int Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_title_char', 'table', t('Char Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_title_float', 'table', t('Float Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_title_bool', 'table', t('Bool Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_title_text', 'table', t('Text Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_title_enum', 'table', t('Enum Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_title_date', 'table', t('Date Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_title_datetime', 'table', t('Datetime Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_title_time', 'table', t('Time Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_title_timestamp', 'table', t('Timestamp Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_title_code', 'table', t('Code Title'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_enum_options', 'table', t('Enum Options'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_textfield('field_weights', 'table', t('Weights'), $weight++, $format, $exclude);
    $fields[] = tow_cck_textfield('field_indexes', 'table', t('Indexes'), $weight++, $format, $exclude);
    $fields[] = tow_cck_textfield('field_units', 'table', t('Units'), $weight++, $format, $exclude, 0, 1);
    $php = '
    $dataset_nids_res = db_query(db_rewrite_sql("SELECT n.nid FROM {node} n WHERE n.type = \'dataset\'"));
    while ($dataset_nid = db_fetch_array($dataset_nids_res)) {
      $allowed_values[$dataset_nid[\'nid\']] = $dataset_nid[\'nid\'];
    }
    return $allowed_values;
  ';
    $fields[] = tow_cck_integer('field_dataset', 'table', t('Dataset'), $weight++, $format, $exclude, 'number', 1, 0, '', $php);
    $fields[] = tow_cck_textarea('field_available_positions', 'table', t('Available Positions'), $weight++, $format, $exclude);

    /* Record fields. */
    $format = array(1 => 'default', 2 => 'default', 3 => 'default', 4 => 'default', 'full' => 'us_0', 'teaser' => 'hidden', 'label' => 'hidden', 'token' => 'default');
    $exclude = array(1 => 0, 2 => 0, 3 => 0, 4 => 0, 'full' => 0, 'teaser' => 1, 'label' => 1, 'token' => 0);
    $fields[] = tow_cck_integer('field_int', 'record', t('Int'), $weight++, $format, $exclude, 'number', 0, 1);
    $format['full'] = 'plain';
    $fields[] = tow_cck_textfield('field_char', 'record', t('Char'), $weight++, $format, $exclude, 0, 1);
    $format['full'] = 'us_2';
    $fields[] = tow_cck_decimal('field_float', 'record', t('Float'), $weight++, $format, $exclude, 'number', 0, 1);
    $format['full'] = 'unformatted';
    $fields[] = tow_cck_integer('field_bool', 'record', t('Bool'), $weight++, $format, $exclude, 'number', 0, 1);
    $format['full'] = 'plain';
    $fields[] = tow_cck_textarea('field_text', 'record', t('Text'), $weight++, $format, $exclude, 0, 1);
    $format['full'] = 'unformatted';
    $fields[] = tow_cck_integer('field_enum', 'record', t('Enum'), $weight++, $format, $exclude, 'number', 0, 1);
    $format['full'] = 'default';
    $fields[] = tow_cck_date('field_date', 'record', t('Date'), $weight++, $format, $exclude, 0, 1);
    $format['full'] = 'default';
    $fields[] = tow_cck_datetime('field_datetime', 'record', t('Datetime'), $weight++, $format, $exclude, 0, 1);
    $format['full'] = 'plain';
    $fields[] = tow_cck_textfield('field_time', 'record', t('Time'), $weight++, $format, $exclude, 0, 1);
    $format['full'] = 'unformatted';
    $fields[] = tow_cck_integer('field_timestamp', 'record', t('Timestamp'), $weight++, $format, $exclude, 'number', 0, 1);
    $format['full'] = 'plain';
    $fields[] = tow_cck_textfield('field_code', 'record', t('Code'), $weight++, $format, $exclude, 0, 1);
    $fields[] = tow_cck_integer('field_table', 'record', t('Table'), $weight++, $format, $exclude, 'number', 1);
    $fields[] = tow_cck_integer('field_position', 'record', t('Position'), $weight++, $format, $exclude, 'number', 1);

    /* Linked Record fields. */
    $format = array(1 => 'default', 2 => 'default', 3 => 'default', 4 => 'default', 'full' => 'hidden', 'teaser' => 'hidden', 'label' => 'hidden', 'token' => 'default');
    $exclude = array(1 => 0, 2 => 0, 3 => 0, 4 => 0, 'full' => 0, 'teaser' => 1, 'label' => 1, 'token' => 0);
    $fields[] = tow_cck_integer('field_record_nid', 'linked_record', t('Record NID'), $weight++, $format, $exclude, 'number', 1, 1);
    $fields[] = tow_cck_textfield('field_tables', 'linked_record', t('Linked Tables'), $weight++, $format, $exclude, 1, 1);

    /* Saved Search fields. */
    $format = array(1 => 'default', 2 => 'default', 3 => 'default', 4 => 'default', 'full' => 'hidden', 'teaser' => 'hidden', 'label' => 'hidden', 'token' => 'default');
    $exclude = array(1 => 0, 2 => 0, 3 => 0, 4 => 0, 'full' => 1, 'teaser' => 1, 'label' => 1, 'token' => 0);
    $fields[] = tow_cck_integer('field_dataset_nid', 'saved_search', t('Dataset NID'), $weight++, $format, $exclude, 'number', 1);
    $fields[] = tow_cck_textfield('field_table_nid', 'saved_search', t('Table NID'), $weight++, $format, $exclude);
    $fields[] = tow_cck_textarea('field_search_constraints', 'saved_search', t('Search Constraints'), $weight++, $format, $exclude);
    $fields[] = tow_cck_textarea('field_selected_fields', 'saved_search', t('Selected Fields'), $weight++, $format, $exclude);
    $fields[] = tow_cck_textfield('field_sc_hash', 'saved_search', t('SC Hash'), $weight++, $format, $exclude);

    /* Document fields. */
    $fields[] = tow_cck_integer('field_doc_dataset_nid', 'document', t('Dataset NID'), $weight++, $format, $exclude, 'number', 1);
    $format = array(1 => 'image_plain', 2 => 'image_plain', 3 => 'image_plain', 4 => 'image_plain', 'full' => 'image_plain', 'teaser' => 'image_plain', 'label' => 'hidden', 'token' => 'image_plain');
    $exclude = array(1 => 0, 2 => 0, 3 => 0, 4 => 0, 'full' => 0, 'teaser' => 0, 'token' => 0);
    $fields[] = tow_cck_image('field_doc_image', 'document', t('Image'), $weight++, $format, $exclude);


    return $fields;
}

/* * **************************************************************************************************
 * Auxiliary functions
 * ************************************************************************************************** */

/**
 * Returns possible dataset categories.
 */
function _tow_install_taxonomy_dataset_categories() {

    $categories = array(
        'Social sciences',
        'Anthropology',
        'Archaeology',
        'Area studies',
        'Cultural and ethnic studies',
        'Economics',
        'Gender and sexuality studies',
        'Geography',
        'Political science',
        'Psychology',
        'Sociology',
        'Natural sciences',
        'Space sciences',
        'Earth sciences',
        'Life sciences',
        'Chemistry',
        'Physics',
        'Formal sciences',
        'Computer sciences',
        'Logic',
        'Mathematics',
        'Statistics',
        'Systems science',
        'Professions and Applied sciences',
        'Agriculture',
        'Architecture and design',
        'Business',
        'Divinity',
        'Education',
        'Engineering',
        'Environmental studies and Forestry',
        'Family and consumer science',
        'Health sciences',
        'Human physical performance and recreation',
        'Journalism, media studies and communication',
        'Law',
        'Library and museum studies',
        'Military sciences',
        'Public administration',
        'Social work',
        'Transportation',
        'Humanities',
        'History',
        'Languages and linguistics',
        'Literature',
        'Performing arts',
        'Philosophy',
        'Religion',
        'Visual arts',
        'Other'
    );

    return $categories;
}