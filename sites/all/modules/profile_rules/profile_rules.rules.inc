<?php
//$id$
/**
 * Rules Conditions and Actions for Profile fields.
 */

/**
 * Implementation of hook_condition_info
 */
function profile_rules_rules_condition_info() {
  return array(
    'profile_rules_condition' => array(
      'label' => t('Compare profile field'),
      'arguments' => array(
        'user' => array(
          'type' => 'user',
          'label' => t('User to grab profile fields from'),
        ),
      ),
      'module' => 'Profile',
      'eval input' => array('profile_field_value'),
    ),
  );
}

/**
 * Profile rules condition callback
 */
function profile_rules_condition(&$user, &$settings) {
  $type = db_result(db_query("SELECT type FROM {profile_fields} WHERE name='%s'", $settings['profile_field']));
  if ($type != 'date')
    return profile_rules_get_profile_field_value($user->uid, $settings['profile_field']) == $settings['profile_field_value'];
  $compare_date = date_parse($settings['profile_field_value']);
  $user_date = unserialize(profile_rules_get_profile_field_value($user->uid, $settings['profile_field']));
  return $user_date['year'] == $compare_date['year'] && $user_date['month'] == $compare_date['month'] && $user_date['day'] == $compare_date['day'];
}

/**
 * Profile rules condition form callback //PD remove & before settings and before form
 */
function profile_rules_condition_form($settings, $form) {
  $form['settings']['profile_field'] = array(
    '#type' => 'select',
    '#title' => t('Profile Field'),
    '#description' => t('Select the profile field to compare value to.'),
    '#options' => profile_rules_get_profile_fields(),
    '#default_value' => $settings['profile_field'],
  );

  $form['settings']['profile_field_value'] = array(
    '#type' => 'textarea',
    '#title' => t('Profile Field Value'),
    '#description' => t('Enter the value to compare to the profile field value. For date fields use the format y-m-d, eg 2010-7-23 (as a string if using PHP processing).'),
    '#default_value' => $settings['profile_field_value'],
  );
}


/**
* Implementation of hook_rules_action_info().
*/
function profile_rules_rules_action_info() {
  return array(
    'profile_rules_set_profile_field_action' => array(
      'label' => t('Set profile field of a user'),
      'module' => 'Profile',
      'arguments' => array(
        'user' => array(
          'type' => 'user',
          'label' => t('User to set profile fields for.'),
        ),
      ),
      'eval input' => array('profile_field_value'),
    ),
  );
}

/**
 * Profile rules action callback
 */
function profile_rules_set_profile_field_action(&$account, &$settings) {
  $value = $settings['profile_field_value'];
  $type = db_result(db_query("SELECT type FROM {profile_fields} WHERE name='%s'", $settings['profile_field']));
  if ($type == 'date') {
    $value_date = date_parse($value);
    $value = array();
    foreach (array('year', 'month', 'day') as $el)
      $value[$el] = $value_date[$el];
    $value = serialize($value);
  }
  profile_rules_set_profile_field_value($account->uid, $settings['profile_field'], $value);
}

/**
 * Profile rules action settings form callback
 */
function profile_rules_set_profile_field_action_form(&$settings, &$form) {
  $form['settings']['profile_field'] = array(
    '#type' => 'select',
    '#title' => t('Profile Field'),
    '#description' => t('Select the profile field to set the value of.'),
    '#options' => profile_rules_get_profile_fields(),
    '#default_value' => $settings['profile_field'],
  );
  $form['settings']['profile_field_value'] = array(
    '#type' => 'textarea',
    '#title' => t('Profile Field Value'),
    '#description' => t('Enter the value to set the profile field to. For date fields use the format y-m-d, eg 2010-7-23 (as a string if using PHP processing).'),
    '#default_value' => $settings['profile_field_value'],
  );
  return $form;
}



/**
 * Get list of defined profile fields.
 */
function profile_rules_get_profile_fields() {
  $fields = array();
  $res = db_query('SELECT * FROM {profile_fields}');
  while ($field = db_fetch_object($res))
    $fields[$field->name] = $field->title;
  return $fields;
}


/*
 * Get the profile field value for the given user and field (by name).
 */ 
function profile_rules_get_profile_field_value($uid, $field) {
  $fid = db_result(db_query("SELECT fid, type FROM {profile_fields} WHERE name='%s'", $field));
  return db_result(db_query("SELECT value FROM {profile_values} WHERE fid=%d AND uid=%d", $fid, $uid));
}

/*
 * Set the profile field value for the given user and field (by name).
 */
function profile_rules_set_profile_field_value($uid, $field, $value) {
  $fid = db_result(db_query("SELECT fid FROM {profile_fields} WHERE name='%s'", $field));
  db_query("DELETE FROM {profile_values} WHERE fid=%d AND uid=%d", $fid, $uid);
  db_query("INSERT INTO {profile_values} (fid, uid, value) VALUES (%d, %d, '%s')", $fid, $uid, $value);
}
