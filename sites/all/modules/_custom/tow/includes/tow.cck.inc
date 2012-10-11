<?php

/**
 * @file
 * 
 * Functions for programmatical creation of the CCK fields.
 */



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Initializes weight.
 */
function tow_cck_init_weight() {
  $weight = db_fetch_object(db_query("

    SELECT 
      MAX({content_node_field_instance}.weight) AS weight 

      FROM 
        {content_node_field_instance} 

  "))->weight;
  $weight++;
  
  return $weight;
}

/**
 * CCK Integer field.
 */
function tow_cck_integer($name, $type, $title, $weight, $format, $exclude, $widget, $required = 0, $multiple = 0, $allowed_values = '', $php = '') {
  $field = array(
    'field_name' => $name,
    'type_name' => $type,
    'display_settings' => array(
      'weight' => $weight,
      'parent' => '',
      2 => array(
        'format' => $format[2],
        'exclude' => $exclude[2],
      ),
      3 => array(
        'format' => $format[3],
        'exclude' => $exclude[3],
      ),
      'label' => array(
        'format' => $format['label'],
        'exclude' => $exclude['label'],
      ),
      'teaser' => array(
        'format' => $format['teaser'],
        'exclude' => $exclude['teaser'],
      ),
      'full' => array(
        'format' => $format['full'],
        'exclude' => $exclude['full'],
      ),
      4 => array(
        'format' => $format[4],
        'exclude' => $exclude[4],
      ),
      'token' => array(
        'format' => $format['token'],
        'exclude' => $exclude['token'],
      ),
    ),
    'widget_active' => '1',
    'type' => 'number_integer',
    'required' => $required,
    'multiple' => $multiple,
    'db_storage' => '1',
    'module' => 'number',
    'active' => '1',
    'locked' => '0',
    'columns' => array(
      'value' => array(
        'type' => 'int',
        'not null' => FALSE,
        'sortable' => TRUE,
      ),
    ),
    'prefix' => '',
    'suffix' => '',
    'allowed_values' => $allowed_values,
    'allowed_values_php' => $php,
    'widget' => array(
      'default_value' => array(
        0 => array(
          'value' => '0',
        ),
      ),
      'default_value_php' => NULL,
      'label' => $title,
      'weight' => $weight,
      'description' => '',
      'type' => $widget,
    ),
  );
  
  return $field;
}

/**
 * CCK Textarea field.
 */
function tow_cck_textarea($name, $type, $title, $weight, $format, $exclude, $required = 0, $multiple = 0, $allowed_values = '', $php = '') {
  $field = array(
    'field_name' => $name,
    'type_name' => $type,
    'display_settings' => array(
      'weight' => $weight,
      'parent' => '',
      2 => array(
        'format' => $format[2],
        'exclude' => $exclude[2],
      ),
      3 => array(
        'format' => $format[3],
        'exclude' => $exclude[3],
      ),
      'label' => array(
        'format' => $format['label'],
        'exclude' => $exclude['label'],
      ),
      'teaser' => array(
        'format' => $format['teaser'],
        'exclude' => $exclude['teaser'],
      ),
      'full' => array(
        'format' => $format['full'],
        'exclude' => $exclude['full'],
      ),
      4 => array(
        'format' => $format[4],
        'exclude' => $exclude[4],
      ),
      'token' => array(
        'format' => $format['token'],
        'exclude' => $exclude['token'],
      ),
    ),
    'widget_active' => '1',
    'type' => 'text',
    'required' => $required,
    'multiple' => $multiple,
    'db_storage' => '1',
    'module' => 'text',
    'active' => '1',
    'locked' => '0',
    'columns' => array(
      'value' => array(
        'type' => 'text',
        'size' => 'big',
        'not null' => FALSE,
        'sortable' => TRUE,
        'views' => TRUE,
      ),
    ),
    'text_processing' => '',
    'max_length' => '',
    'allowed_values' => $allowed_values,
    'allowed_values_php' => $php,
    'widget' => array(
      'rows' => NULL,
      'size' => NULL,
      'label' => $title,
      'weight' => $weight,
      'description' => '',
      'type' => 'text_textarea',
      'module' => 'text',
    ),
  );
  
  return $field;
}

/**
 * CCK Text field.
 */
function tow_cck_textfield($name, $type, $title, $weight, $format, $exclude, $required = 0, $multiple = 0, $allowed_values = '', $php = '') {
  $field = array(
    'field_name' => $name,
    'type_name' => $type,
    'display_settings' => array(
      'weight' => $weight,
      'parent' => '',
      2 => array(
        'format' => $format[2],
        'exclude' => $exclude[2],
      ),
      3 => array(
        'format' => $format[3],
        'exclude' => $exclude[3],
      ),
      'label' => array(
        'format' => $format['label'],
        'exclude' => $exclude['label'],
      ),
      'teaser' => array(
        'format' => $format['teaser'],
        'exclude' => $exclude['teaser'],
      ),
      'full' => array(
        'format' => $format['full'],
        'exclude' => $exclude['full'],
      ),
      4 => array(
        'format' => $format[4],
        'exclude' => $exclude[4],
      ),
      'token' => array(
        'format' => $format['token'],
        'exclude' => $exclude['token'],
      ),
    ),
    'widget_active' => '1',
    'type' => 'text',
    'required' => $required,
    'multiple' => $multiple,
    'db_storage' => '0',
    'module' => 'text',
    'active' => '1',
    'locked' => '0',
    'columns' => array(
      'value' => array(
        'type' => 'varchar',
        'length' => '1024',
        'not null' => FALSE,
        'sortable' => TRUE,
        'views' => TRUE,
      ),
    ),
    'text_processing' => '0',
    'max_length' => '1024',
    'allowed_values' => $allowed_values,
    'allowed_values_php' => $php,
    'widget' => array(
      'rows' => 5,
      'size' => '60',
      'default_value' => array(
        0 => array(
          'value' => '',
          '_error_element' => 'default_value_widget][field_title_int][0][value',
        ),
      ),
      'default_value_php' => NULL,
      'label' => $title,
      'weight' => $weight,
      'description' => '',
      'type' => 'text_textfield',
      'module' => 'text',
    ),
  );
  
  return $field;
}

/**
 * CCK Decimal field.
 */
function tow_cck_decimal($name, $type, $title, $weight, $format, $exclude, $widget, $required = 0, $multiple = 0, $allowed_values = '', $php = '') {
  $field = array(
    'field_name' => $name,
    'type_name' => $type,
    'display_settings' => array(
      'weight' => $weight,
      'parent' => '',
      2 => array(
        'format' => $format[2],
        'exclude' => $exclude[2],
      ),
      3 => array(
        'format' => $format[3],
        'exclude' => $exclude[3],
      ),
      'label' => array(
        'format' => $format['label'],
        'exclude' => $exclude['label'],
      ),
      'teaser' => array(
        'format' => $format['teaser'],
        'exclude' => $exclude['teaser'],
      ),
      'full' => array(
        'format' => $format['full'],
        'exclude' => $exclude['full'],
      ),
      4 => array(
        'format' => $format[4],
        'exclude' => $exclude[4],
      ),
      'token' => array(
        'format' => $format['token'],
        'exclude' => $exclude['token'],
      ),
    ),
    'widget_active' => '1',
    'type' => 'number_decimal',
    'required' => $required,
    'multiple' => $multiple,
    'db_storage' => '0',
    'module' => 'number',
    'active' => '1',
    'locked' => '0',
    'columns' => array(
      'value' => array(
        'type' => 'numeric',
        'precision' => '10',
        'scale' => '2',
        'not null' => FALSE,
        'sortable' => TRUE,
      ),
    ),
    'prefix' => '',
    'suffix' => '',
    'min' => '',
    'max' => '',
    'allowed_values' => $allowed_values,
    'allowed_values_php' => $php,
    'precision' => '10',
    'scale' => '2',
    'decimal' => '.',
    'widget' => array(
      'default_value' => array(
        0 => array(
          'value' => '',
          '_error_element' => 'default_value_widget][field_float][0][value',
        ),
      ),
      'default_value_php' => NULL,
      'label' => $title,
      'weight' => $weight,
      'description' => '',
      'type' => $widget,
      'module' => 'number',
    ),
  );

  return $field;
}

/**
 * CCK Date field.
 */
function tow_cck_date($name, $type, $title, $weight, $format, $exclude, $required = 0, $multiple = 0) {
  $field = array(
    'field_name' => $name,
    'type_name' => $type,
    'display_settings' => array(
      'weight' => $weight,
      'parent' => '',
      2 => array(
        'format' => $format[2],
        'exclude' => $exclude[2],
      ),
      3 => array(
        'format' => $format[3],
        'exclude' => $exclude[3],
      ),
      'label' => array(
        'format' => $format['label'],
        'exclude' => $exclude['label'],
      ),
      'teaser' => array(
        'format' => $format['teaser'],
        'exclude' => $exclude['teaser'],
      ),
      'full' => array(
        'format' => $format['full'],
        'exclude' => $exclude['full'],
      ),
      4 => array(
        'format' => $format[4],
        'exclude' => $exclude[4],
      ),
      'token' => array(
        'format' => $format['token'],
        'exclude' => $exclude['token'],
      ),
    ),
    'widget_active' => '1',
    'type' => 'date',
    'required' => $required,
    'multiple' => $multiple,
    'db_storage' => '0',
    'module' => 'date',
    'active' => '1',
    'locked' => '0',
    'columns' => array(
      'value' => array(
        'type' => 'varchar',
        'length' => 20,
        'not null' => FALSE,
        'sortable' => TRUE,
        'views' => TRUE,
      ),
    ),
    'granularity' => array(
      'year' => 'year',
      'month' => 'month',
      'day' => 'day',
    ),
    'timezone_db' => '',
    'tz_handling' => 'none',
    'todate' => '',
    'repeat' => 0,
    'repeat_collapsed' => '',
    'default_format' => 'medium',
    'widget' => array(
      'default_value' => 'blank',
      'default_value_code' => '',
      'default_value2' => 'same',
      'default_value_code2' => '',
      'input_format' => 'm/d/Y - H:i:s',
      'input_format_custom' => '',
      'increment' => 1,
      'text_parts' => array(),
      'year_range' => '-3:+3',
      'label_position' => 'above',
      'label' => $title,
      'weight' => $weight,
      'description' => '',
      'type' => 'date_text',
      'module' => 'date',
    ),
  );
 
  return $field;
}

/**
 * CCK Datetime field.
 */
function tow_cck_datetime($name, $type, $title, $weight, $format, $exclude, $required = 0, $multiple = 0) {
  $field = array(
    'field_name' => $name,
    'type_name' => $type,
    'display_settings' => array(
      'weight' => $weight,
      'parent' => '',
      2 => array(
        'format' => $format[2],
        'exclude' => $exclude[2],
      ),
      3 => array(
        'format' => $format[3],
        'exclude' => $exclude[3],
      ),
      'label' => array(
        'format' => $format['label'],
        'exclude' => $exclude['label'],
      ),
      'teaser' => array(
        'format' => $format['teaser'],
        'exclude' => $exclude['teaser'],
      ),
      'full' => array(
        'format' => $format['full'],
        'exclude' => $exclude['full'],
      ),
      4 => array(
        'format' => $format[4],
        'exclude' => $exclude[4],
      ),
      'token' => array(
        'format' => $format['token'],
        'exclude' => $exclude['token'],
      ),
    ),
    'widget_active' => '1',
    'type' => 'datetime',
    'required' => $required,
    'multiple' => $multiple,
    'db_storage' => '0',
    'module' => 'date',
    'active' => '1',
    'locked' => '0',
    'columns' => array(
      'value' => array(
        'type' => 'datetime',
        'not null' => FALSE,
        'sortable' => TRUE,
        'views' => TRUE,
      ),
    ),
    'granularity' => array(
      'year' => 'year',
      'month' => 'month',
      'day' => 'day',
      'hour' => 'hour',
      'minute' => 'minute',
      'second' => 'second',
    ),
    'timezone_db' => 'UTC',
    'tz_handling' => 'site',
    'todate' => '',
    'repeat' => 0,
    'repeat_collapsed' => '',
    'default_format' => 'medium',
    'widget' => array(
      'default_value' => 'blank',
      'default_value_code' => '',
      'default_value2' => 'same',
      'default_value_code2' => '',
      'input_format' => 'm/d/Y - H:i:s',
      'input_format_custom' => '',
      'increment' => 1,
      'text_parts' => array(),
      'year_range' => '-3:+3',
      'label_position' => 'above',
      'label' => $title,
      'weight' => $weight,
      'description' => '',
      'type' => 'date_text',
      'module' => 'date',
    ),
  );
  
  return $field;
}

/**
 * CCK Image field.
 */
function tow_cck_image($name, $type, $title, $weight, $format, $exclude, $required = 0, $multiple = 1) {
  module_load_include('inc', 'imagefield', 'imagefield_widget');
  module_load_include('inc', 'filefield', 'filefield_widget');

  $field = array(
    'field_name' => $name,
    'type_name' => $type,
    'display_settings' => array(
      'weight' => $weight,
      'parent' => '',
      'label' => array(
        'format' => $format['label'],
      ),
      'teaser' => array(
        'format' => $format['teaser'],
        'exclude' => $exclude['teaser'],
      ),
      'full' => array(
        'format' => $format['full'],
        'exclude' => $exclude['full'],
      ),
      4 => array(
        'format' => $format[4],
        'exclude' => $exclude[4],
      ),
      2 => array(
        'format' => $format[2],
        'exclude' => $exclude[2],
      ),
      3 => array(
        'format' => $format[3],
        'exclude' => $exclude[3],
      ),
      'token' => array(
        'format' => $format['token'],
        'exclude' => $exclude['token'],
      ),
    ),
    'widget_active' => '1',
    'type' => 'filefield',
    'required' => $required,
    'multiple' => $multiple,
    'db_storage' => '1',
    'module' => 'filefield',
    'active' => '1',
    'locked' => '0',
    'columns' => array(
      'fid' => array(
        'type' => 'int',
        'not null' => FALSE,
        'views' => TRUE,
      ),
      'list' => array(
        'type' => 'int',
        'size' => 'tiny',
        'not null' => FALSE,
        'views' => TRUE,
      ),
      'data' => array(
        'type' => 'text',
        'serialize' => TRUE,
        'views' => TRUE,
      ),
    ),
    'list_field' => '0',
    'list_default' => 1,
    'description_field' => '0',
    'widget' => array(
      'file_extensions' => 'png gif jpg jpeg',
      'file_path' => '',
      'progress_indicator' => 'bar',
      'max_filesize_per_file' => '',
      'max_filesize_per_node' => '',
      'max_resolution' => '0',
      'min_resolution' => '0',
      'alt' => '',
      'custom_alt' => 0,
      'title' => '',
      'custom_title' => 0,
      'title_type' => 'textfield',
      'default_image' => NULL,
      'use_default_image' => 0,
      'label' => $title,
      'weight' => $weight,
      'description' => '',
      'type' => 'imagefield_widget',
      'module' => 'imagefield',
    ),
  );
  
  if (module_exists('imagefield_tokens')) {
    $field['alt'] = '[title]';
    $field['title'] = '[title]';
  }

  return $field;
}
