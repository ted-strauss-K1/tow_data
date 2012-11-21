<?php

/**
 * Implementation of hook_strongarm().
 */
function tow_settings_linked_record_feature_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_anonymous_linked_record';
  $strongarm->value = 0;
  $export['comment_anonymous_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_controls_linked_record';
  $strongarm->value = '3';
  $export['comment_controls_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_mode_linked_record';
  $strongarm->value = '4';
  $export['comment_default_mode_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_order_linked_record';
  $strongarm->value = '1';
  $export['comment_default_order_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_per_page_linked_record';
  $strongarm->value = '50';
  $export['comment_default_per_page_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_form_location_linked_record';
  $strongarm->value = '0';
  $export['comment_form_location_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_linked_record';
  $strongarm->value = '0';
  $export['comment_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_preview_linked_record';
  $strongarm->value = '1';
  $export['comment_preview_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_subject_field_linked_record';
  $strongarm->value = '1';
  $export['comment_subject_field_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'content_extra_weights_linked_record';
  $strongarm->value = array(
    'title' => '-5',
    'body_field' => '0',
    'revision_information' => '20',
    'author' => '20',
    'options' => '25',
    'comment_settings' => '30',
    'menu' => '-2',
  );
  $export['content_extra_weights_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'form_build_id_linked_record';
  $strongarm->value = 'form-1de277b2f013cf5a722afec7306db803';
  $export['form_build_id_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'node_options_linked_record';
  $strongarm->value = array(
    0 => 'status',
  );
  $export['node_options_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'rdf_schema_class_linked_record';
  $strongarm->value = '';
  $export['rdf_schema_class_linked_record'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'skinr_settings_linked_record';
  $strongarm->value = array(
    0 => 'comment_group',
    1 => 'node_group',
  );
  $export['skinr_settings_linked_record'] = $strongarm;

  return $export;
}
