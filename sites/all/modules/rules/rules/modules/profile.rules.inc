<?php

/**
 * @file rules integration for the profile module
 *
 * @addtogroup rules
 * @{
 */

/**
 * Implementation of hook_rules_event_info().
 */
function profile_rules_event_info() {
  return array(
    'profile_update' => array(
      'label' => t('User profile fields have been updated'),
      'module' => 'Profile',
      'arguments' => rules_events_hook_user_arguments(t('updated user')) + array(
        'account_unchanged' => array('type' => 'user', 'label' => t('unchanged user')),
      ),
    ),
  );
}

/**
 * Implementation of hook_rules_condition_info().
 */
function profile_rules_condition_info() {
  return array(
    'rules_condition_profile_field_changed' => array(
      'label' => t('Compare profile field against two users.'),
      'arguments' => array(
        'user1' => array('type' => 'user', 'label' => t('User account 1')),
        'user2' => array('type' => 'user', 'label' => t('User account 2')),
      ),
      'help' => t('Evaluates to TRUE when profile field differ between given users.'),
      'module' => 'Profile',
    ),
  );
}

/**
 * Compares for changed profile field between two given users.
 */
function rules_condition_profile_field_changed($user1, $user2, $settings) {
  $profile_field = $settings['profile_field'];
  if (isset($user1->{$profile_field}) && isset($user2->{$profile_field})) {
    $value1 = $user1->{$profile_field};
    $value2 = $user2->{$profile_field};
    return $value1 !== $value2;
  }
  else {
    // Comparing non existing fields differ always
    return TRUE;
  }
}

/**
 * @}
 */
