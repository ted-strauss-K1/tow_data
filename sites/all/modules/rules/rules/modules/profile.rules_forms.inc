<?php


/**
 * @file Rules configuration forms for the profile module
 *
 * @addtogroup rules
 * @{
 */

function rules_condition_profile_field_changed_form($settings, &$form) {
  $form['settings'] += _rules_profile_fields_form($settings);
}

function _rules_profile_fields_form($settings) {
  $fields = array();
  $result = db_query('SELECT name, title FROM {profile_fields}');
  while ($field = db_fetch_object($result)) {
    $fields[$field->name] = t($field->title);
  }

  $form['profile_field'] = array(
    '#type' => 'select',
    '#title' => t('Profile Field'),
    '#description' => t('Select the profile field to compare value to.'),
    '#options' => $fields,
    '#default_value' => isset($settings['profile_field']) ? $settings['profile_field'] : '',
    '#required' => TRUE,
  );
  return $form;
}

/**
 * @}
 */
