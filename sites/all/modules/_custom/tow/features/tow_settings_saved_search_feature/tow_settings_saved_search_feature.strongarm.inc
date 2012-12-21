<?php

/**
 * Implementation of hook_strongarm().
 */
function tow_settings_saved_search_feature_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_anonymous_saved_search';
  $strongarm->value = 0;
  $export['comment_anonymous_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_controls_saved_search';
  $strongarm->value = '3';
  $export['comment_controls_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_mode_saved_search';
  $strongarm->value = '4';
  $export['comment_default_mode_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_order_saved_search';
  $strongarm->value = '2';
  $export['comment_default_order_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_per_page_saved_search';
  $strongarm->value = '50';
  $export['comment_default_per_page_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_form_location_saved_search';
  $strongarm->value = '1';
  $export['comment_form_location_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_preview_saved_search';
  $strongarm->value = '0';
  $export['comment_preview_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_saved_search';
  $strongarm->value = '2';
  $export['comment_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_subject_field_saved_search';
  $strongarm->value = '0';
  $export['comment_subject_field_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'content_extra_weights_saved_search';
  $strongarm->value = array(
    'title' => '-9',
    'body_field' => '-8',
    'revision_information' => '-2',
    'author' => '-1',
    'options' => '0',
    'comment_settings' => '1',
    'menu' => '-3',
    'vud_node_widget_display' => '-10',
  );
  $export['content_extra_weights_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'form_build_id_saved_search';
  $strongarm->value = 'form-3e7976093dc8498ddbe3e525e9c5d6d8';
  $export['form_build_id_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'node_options_saved_search';
  $strongarm->value = array(
    0 => 'status',
  );
  $export['node_options_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'rdf_schema_class_saved_search';
  $strongarm->value = '';
  $export['rdf_schema_class_saved_search'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'skinr_settings_saved_search';
  $strongarm->value = array(
    0 => 'comment_group',
    1 => 'node_group',
  );
  $export['skinr_settings_saved_search'] = $strongarm;

  return $export;
}
