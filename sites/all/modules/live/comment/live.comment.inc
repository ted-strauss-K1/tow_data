<?php

/**
 * Construct and Output comment preview securely.
 */
function live_comment_preview() {
  global $user;

  $format = !empty($_POST['format']) ? $_POST['format'] : FILTER_FORMAT_DEFAULT;
  $body = !empty($_POST['body']) ? $_POST['body'] : '';
  $access = user_access('use live comment preview');
  $token = $_POST['token'] != "" ? $_POST['token'] : "";
  $skip_anonymous = $user->uid == 0 ? TRUE : FALSE;
  $token_value = !empty($_POST['token_value']) ? $_POST['token_value'] : '';
  $valid_token = drupal_valid_token($token, $token_value, $skip_anonymous);

  if (!filter_access($format) || $body == "" || !$access || !$valid_token) {
    // SECURITY CHECK!
    // Deny access:
    // 1) If the current user is not allowed to use specified input format; or
    // 2) If the comment body is empty; or
    // 3) If the form submission is faked i.e. form token validation fails (to prevent CSRF attacks. NOTE: check_markup(..) should only be used after validating token); or
    // 4) If user does not have the permission.
    drupal_access_denied();
    return;
  }

  if (user_is_anonymous()) {
    $comment->uid = 0;
    $username = NULL;
  }
  else {
    if ($_POST['uid'] == $user->uid) {
      $comment->uid = $user->uid;
      $username = $user->name;
    }
    else {
      $comment->uid = $_POST['uid'];
      $account = user_load($comment->uid);
      $username = $account->name;
    }
  }

  $comment->name = !empty($_POST['name']) ? $_POST['name'] : $username;
  $comment->homepage = !empty($_POST['homepage']) ? $_POST['homepage'] : '';

  $comment->comment = check_markup($body, $format);
  $comment->subject = !empty($_POST['subject']) ? $_POST['subject'] : trim(truncate_utf8(decode_entities(strip_tags($comment->comment)), 29, TRUE));

  $comment->timestamp = time();

  $output = '<h2>'. t('Comment Preview') .'</h2><div class="preview">';
  $output .= theme('comment', $comment, array());
  $output .= '</div>';

  // Avoid debug information(devel.module) from being added to the preview.
  $GLOBALS['devel_shutdown'] = FALSE;

  exit(drupal_json(array('html' => $output)));
}
