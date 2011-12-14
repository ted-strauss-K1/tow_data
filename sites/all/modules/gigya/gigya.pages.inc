<?php
// $Id: gigya.pages.inc,v 1.6 2010/08/19 02:17:06 azinck Exp $

/**
 * @file
 * Page display functions for for Gigya module.
 */

/**
 * Undocumented Function! WTF is this for? Should be removed.
 *
 * @param object $account
 *   A user object.
 * @param $name
 */
function gigya_connect($account, $name = NULL) {
  drupal_set_title('Edit your Social Network connections');
  return theme('gigya_connect_display');
}

/**
 * Undocumented Function!
 *
 * @return
 *   Undocumented Return!
 */
function gigya_logout_ajax() {
  global $user, $base_path;
  module_load_include('.inc', 'user', 'user.pages');
  module_load_include('inc', 'gigya');

  $dest = $_REQUEST['destination'];
  if (empty($dest)) $dest = $base_path;
  $logout_behavior = array('logoutLocation' => $dest);

  drupal_add_js(array('gigya' => $logout_behavior), 'setting');

  $behavior_js = 'Drupal.behaviors.gigyaLogout = function(){ gigya.services.socialize.logout(Drupal.settings.gigya.conf, {callback:Drupal.gigya.logoutResponse});};';
  drupal_add_js($behavior_js, 'inline');
  return '<div id="logoutMessage">' . t("Logging out...") . '</div>';
}

/**
 * Undocumented Function!
 */
function gigya_login_ajax() {
  global $user, $base_path;

  drupal_set_message('gigya_login_ajax u:' . $user->uid > 0);
  module_load_include('inc', 'gigya');

  if (!($user->uid)) {
    if ($_gigya_user = GigyaUser::userFromUrl()) {
      $dest = $base_path . variable_get('gigya_login_uiconfig_urlredirect', 'user');
      $bio = $_gigya_user->getUserInfo();
      // Check to see if the Gigya user is a Drupal user.
      if (is_numeric($_gigya_user->uid) && $account = user_load($_gigya_user->uid)) {
        if (!variable_get('user_email_verification', TRUE) || $account->login) {
          // We can suppress the notify login JS because the user is clearly
          // already logged into gigya.
          $_SESSION['gigya_suppress_notifylogin_js'] = TRUE;
          // Log the user in.
          user_external_login($account);
        }
        else {
          drupal_set_message(t('You must validate your email address for this account before logging in.'));
        }
        drupal_goto();
      }
      // Not a Drupal user, try to register them if we're allowed to.
      elseif (variable_get('user_register', 1)) {
        $form_state['redirect'] = $_REQUEST['destination'];
        $form_state['values'] = array();

        // Default values get populated during drupal_execute by hook_form_alter.
        drupal_execute('user_register', $form_state);

        if (form_get_errors()) {
          $messages = drupal_get_messages('error');
          $output = gigya_print_object('', $messages);
          // We were unable to register a valid new user.
          // Send the error messages to watchdog.
          watchdog('gigya', 'Unable to auto-create user account:' . $output);

          $dest = $_REQUEST['destination'];
          unset($_REQUEST['destination']);
          drupal_goto('socialize-register', array('signature' => $_GET['signature'], 'UID' => $_GET['UID'], 'timestamp' => $_GET['timestamp'], 'destination' => $dest));
        }

        drupal_redirect_form($form, $form_state['redirect']);
      }
      else {
        drupal_set_message(t('Only site administrators can create new user accounts.'), 'error');
      }
    }
  }
  drupal_goto();
}

/**
 * Handles AJAX requests to Gigya.
 *
 * Currently only used to set session variables for the JavaScript.
 *
 * @param $op
 *   Undocumented Parameter!
 */
function gigya_ajax($op) {
  switch ($op) {
    case 'notify-login':
      $_SESSION['gigya_suppress_notifylogin_js'] = TRUE;
      break;
    case 'link-accounts-complete':
      $_SESSION['link_accounts'] = FALSE;
      break;
    case 'logout':
      break;
  }
  exit();
}


/**
 * Undocumented Function!
 *
 * @param $account
 *   Undocumented Parameter!
 * @param $name
 *   Undocumented Parameter!
 *
 * @return
 *   Undocumented Return!
 */
function gigya_get_friends($account, $name = NULL) {
  $title = isset($account->title) ? $account->title : $account->name;
  drupal_set_title(check_plain($title));

  module_load_include('inc', 'gigya');
  $site_friends = array();
  $this_gigya_user = new GigyaUser($account->uid);
  if ($friends = $this_gigya_user->getFriends()) {
    foreach ($friends['friends']->friend as $friend) {
      $friend_arr = get_object_vars($friend);
      if ($friend_arr['isSiteUser'] === 'true')
        $site_friends[] = $friend_arr;
    }
  }

  return theme('gigya_friends', $site_friends);
}

/**
 * Wrapper for the gigya javascript userInfo function.
 *
 * Injects the gigya getUserInfo javascript function.
 *
 * @param $object
 *   Undocumented Parameter!
 * @param $name
 *   Undocumented Parameter!
 *
 * @return
 *   Undocumented Return!
 */
function gigya_get_user($object, $name = NULL) {
  $title = isset($object->title) ? $object->title : $object->name;
  drupal_set_title(check_plain($title));

  // Add the onload functions .
  $behavior_js = 'Drupal.behaviors.gigyaGetUserInfo = function(context){ gigya.services.socialize.getUserInfo(Drupal.settings.gigya.conf, {callback:Drupal.gigya.getUserInfoCallback}); }';
  drupal_add_js($behavior_js, 'inline');

  $userinfo = '<div id="userinfo"></div>';
  return $userinfo;
}

/**
 * Undocumented Function!
 *
 * @return $account
 *   Undocumented Return!
 */
function gigya_register() {
  global $user;

  // If we are already logged on, go to the user page instead.
  if ($user->uid) {
    drupal_goto('user/' . $user->uid);
  }

  if (!($_gigya_user = GigyaUser::userFromUrl())) {
    drupal_goto();
  }
  $bio = $_gigya_user->getUserInfo(TRUE);

  $registration_form = drupal_get_form('user_register');
  $link_accounts_form = drupal_get_form('gigya_link_accounts_form');
  return theme('gigya_register', $bio, $registration_form, $link_accounts_form);
}
