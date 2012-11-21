<?php

/**
 * Implementation of hook_strongarm().
 */
function tow_settings_record_feature_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_anonymous_record';
  $strongarm->value = 0;
  $export['comment_anonymous_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_controls_record';
  $strongarm->value = '3';
  $export['comment_controls_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_mode_record';
  $strongarm->value = '4';
  $export['comment_default_mode_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_order_record';
  $strongarm->value = '1';
  $export['comment_default_order_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_per_page_record';
  $strongarm->value = '50';
  $export['comment_default_per_page_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_form_location_record';
  $strongarm->value = '0';
  $export['comment_form_location_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_preview_record';
  $strongarm->value = '1';
  $export['comment_preview_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_record';
  $strongarm->value = '0';
  $export['comment_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_subject_field_record';
  $strongarm->value = '1';
  $export['comment_subject_field_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'content_extra_weights_record';
  $strongarm->value = array(
    'title' => '-5',
    'body_field' => '0',
    'revision_information' => '-2',
    'author' => '-3',
    'options' => '-1',
    'comment_settings' => '0',
    'menu' => '-4',
  );
  $export['content_extra_weights_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'form_build_id_record';
  $strongarm->value = 'form-fc7eae505222ccbdaeee523e7eb5504e';
  $export['form_build_id_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'node_options_record';
  $strongarm->value = array(
    0 => 'status',
  );
  $export['node_options_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'rdf_schema_class_record';
  $strongarm->value = '';
  $export['rdf_schema_class_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'skinr_settings_record';
  $strongarm->value = array(
    0 => 'comment_group',
    1 => 'node_group',
  );
  $export['skinr_settings_record'] = $strongarm;

  return $export;
}
