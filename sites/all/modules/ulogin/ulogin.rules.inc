<?php
/**
 * @file
 * Functions for integrating the Rules module with uLogin.
 */

/**
 * Implements hook_rules_condition_info().
 * @ingroup rules
 */
function ulogin_rules_condition_info() {
  $conditions = array();
  
  $conditions['ulogin_user_created_ulogin'] = array(
    'label' => t('User created by uLogin'),
    'module' => t('uLogin'),
    'arguments' => array(
      'account' => array('label' => t('User'), 'type' => 'user'),
    ),
    'help' => t('TRUE is returned if user account was created by uLogin.'),
  );
  
  return $conditions;
}

function ulogin_user_created_ulogin($account) {
  global $ulogin_data;
  return (!empty($ulogin_data) || !empty($account->ulogin));
}
