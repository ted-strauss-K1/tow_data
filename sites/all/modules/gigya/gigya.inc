<?php
// $Id: gigya.inc,v 1.13 2011/01/24 08:53:39 gambaweb Exp $

/**
 * @file
 */

/**
 * Undocumented Function!
 */
function gigya_execute_replacement(&$value, $key, $params) {
  extract($params);

  switch ($type) {
    case 'node':
      $full_object = node_load($object->nid);
      break;
    case 'comment':
      $full_object = _comment_load($object->cid);
      break;
    case 'user':
      $full_object = user_load($object->uid);
      break;
  }

  if (is_string($value)) {
    if (module_exists('token')) {
      foreach ($token_types as $token_type) {
        if ($type == 'comment' && $token_type == 'node') {
          $the_object = node_load($object->nid);
        }
        else {
          $the_object = $full_object;
        }
        $value = token_replace($value, $token_type, $the_object);
      }
    }
    $value = strtr($value, $replacements);
  }
}

/**
 * Callback for array_walk.
 *
 * Receives form items and assesses whether or not they're input fields.
 * If so, then it tries to set the default value based on the
 * profile mappings.
 *
 * @see gigya_form_alter() in gigya.module.
 */
function gigya_apply_profile_mapping(&$field, $key) {
  if (module_exists('token')) {
    if (is_array($field)) {
      // Check for some key elements. We do this explicitly to make sure the
      // indexes are defined and avoid PHP notices.
      $type = isset($field['#type']) ? $field['#type'] : '';
      $field_name = isset($field['#field_name']) ? $field['#field_name'] : '';
      $theme = isset($field['#theme']) ? $field['#theme'] : '';

      // Is it a standard type to replace?
      if (in_array($type, array('checkbox', 'checkboxes', 'date', 'radio', 'radios', 'select', 'textarea', 'textfield'))) {
        $field['#default_value'] = token_replace(variable_get('gigya_map_' . $key, ''), 'gigya');
      }
      // If it isn't and we have a #field_name, just go for it.
      elseif ($field_name && $type) {
        $field['#default_value']['value'] = token_replace(variable_get('gigya_map_' . $field['#field_name'], ''), 'gigya');
      }
      // If it is a fieldset or multi-value field, drill in.
      elseif ($type == 'fieldset' || $theme == 'content_multiple_values') {
        array_walk($field, 'gigya_apply_profile_mapping');
      }
    }
  }
}

/**
 * Undocumented Function!
 */
function gigya_delete_profile($uid) {
  gigya_unlink_accounts($uid, $uid);
  return TRUE;
}

/**
 * Undocumented Function!
 */
function gigya_get_loginprovider($uid) {
  return FALSE;
}

/**
 * Undocumented Function!
 */
function gigya_avatar_check($uid) {
  $bio = gigya_get_user_info($uid);
  if ($bio['thumbnailURL'])
    return $bio['thumbnailURL'];
  elseif ($bio['photoURL'])
    return $bio['photoURL'];
  else {
    return FALSE;
  }
}

/**
 * Undocumented Function!
 */
function gigya_save_userpic($uid) {
  // Get Facebook user picture's url.
  if ($avatar = gigya_avatar_check($uid)) {
    $result = drupal_http_request($avatar);

    if ($result->code != 200) {
      watchdog('gigya', 'Failed importing user avatar for @uid, code : @code',
      array('@uid' => $uid, '@code' => $result->code));
      return FALSE;
    }

    $filename = 'picture-gigya_'. $uid .'.jpg';
    $dest = file_directory_path();
    if (variable_get('user_picture_path', '')) {
      $dest .= '/' . variable_get('user_picture_path', '');
    }
    $dest .= '/' . $filename;

    $file = file_save_data($result->data, $dest, FILE_EXISTS_REPLACE);
    // Resizing
    list($width, $height) = sscanf(variable_get('user_picture_dimensions', '85x85'), '%dx%d');
    image_scale_and_crop($file, $dest, $width, $height);
    drupal_set_message('User picture saved successfully!');

    return $file;
  }
  else {
    drupal_set_message('Your social network does not have a profile picture');
    return FALSE;
  }
}

/*
 *  Gigya parameter functions below
 */

/**
 * Retrieves services available on this account.
 */
function gigya_get_capabilities() {
  $services = array(
    'login' => 'login',
    'friends' => 'friends',
  );
  return $services;
}

/**
 * Undocumented Function!
 */
function gigya_status_form($account = NULL) {
  global $user;
  drupal_add_js(drupal_get_path('module', 'gigya') . '/gigya_updatestatus.js', 'module');

  if (empty($account)) {
    global $user;
    $account = $user;
  }

  if ($capabilities = $user->gigya->getCapabilities()) {
    if (array_search('Status', $capabilities)) {
      $form = array();
      $form['status'] = array(
        '#type' => 'textfield',
        '#id' => 'gigya-textfield',
      );
      return $form;
    }
  }
}

/**
 * Gets the list of services which can be connected to.
 *
 * @return array
 *  An array of services names.
 */
function gigya_get_networks() {
  $services = array(
    'facebook' => 'Facebook',
    'myspace' => 'MySpace',
    'twitter' => 'Twitter',
    'google' => 'Google',
    'yahoo' => 'Yahoo',
    'aol' => 'AOL',
    'blogger' => 'blogger',
    'wordpress' => 'WordPress',
    'typepad' => 'typepad',
    'liveJournal' => 'liveJournal',
    'hyves' =>  'hyves' ,
    'veriSign' => 'veriSign',
    'linkedin' =>  'LinkedIn',
    'liveid' => 'Windows Live ID',
    'openid' => 'OpenID',
    'orkut' => 'Orkut',
  );
  return $services;
}

/**
 * Retrieve the human-readable display name for a service.
 */
function gigya_get_network_display_name($network) {
  $networks = gigya_get_networks();
  if (array_key_exists($network, $networks)) {
    return $networks[$network];
  }
  return $network;
}

/**
 * Undocumented Function!
 *
 * @Undocumented Function! add the ability to specify the field type.
 */
function _gigya_get_profile_fields() {
  return array(
    'nickname' => t('Nickname'),
    'photoURL' => t('Gigya Profile photo URL'),
    'thumbnailURL' => t('Thumbnail profile photo URL'),
    'profileURL' => t('Profile URL'),
    'firstName' => t('First name'),
    'lastName' => t('Last name'),
    'gender' => t('Gender'),
    'age' => t('Age'),
    'birthDay' => t('Birth day'),
    'birthMonth' => t('Birth month'),
    'birthYear' => t('Birth year'),
    'email' => t('Email'),
    'country' => t('Country'),
    'state' => t('State'),
    'city' => t('City'),
    'zip' => t('Zip'),
  );
}

/**
 * Undocumented Function!: is this used anywhere?
 */
function _gigya_get_user_capabilities($uid) {
  if (empty($uid)) {
    return;
  }
  if ($bio = gigya_get_user_info($uid)) {
    $capabilities = split(', ', $bio['capabilities']);
    array_walk($capabilities, '_gigya_trim_value');
    return $capabilities;
  }
  else {
    return FALSE;
  }
}


/**
 * Helper function for trimming.
 */
function _gigya_trim_value(&$value) {
  $value = trim($value);
}

/**************************************************
 *  Internal Gigya signature verification functions
 **************************************************/

/**
 * Wrapper around the hmacsha1 signature function.
 *
 * @param string $timestamp
 *   The current timestamp for the request being made or received.
 * @param string $uid
 *   The Gigya GID being passed to/from Gigya.
 * @param string $privatekey
 *   The private key set in the admin settings. Fetch the key from the
 *   Gigya site.
 */
function gigya_verify_signature($timestamp, $uid, $signature) {
  $secret = variable_get('gigya_SecretKey', FALSE);
  $localkey = _gigya_verify_signature($timestamp, $uid, $secret);
  if ($localkey == $signature) {
    return $uid;
  }
  else{
    return FALSE;
  }
}

/**
 * Undocumented Function!
 */
function _gigya_verify_signature($timestamp, $uid, $privatekey) {
  $ch1data = $timestamp .'_'. $uid;
  $keydata = base64_decode($privatekey);
  return _gigya_encodedata($keydata, $ch1data);
}

/**
 * Wrapper for the base64 encoding of the hmacsha1 function.
 *
 * @param string $key
 *   The base64 decoded version of the private key set in the Gigya settings.
 * @param string $data
 *   The timestamp + uid being passed for comparison.
 *
 * @return The signature created by the hmacsha1 function. Should be the same
 *   as the signature retrieved from Gigya.
 */
function _gigya_encodedata($key, $data) {
  return base64_encode(_gigya_hmacsha1($key, $data));
}

/**
 *  Internal Gigya signature verification function.
 */
function _gigya_hmacsha1($key, $data) {
  $blocksize = 64;
  $hashfunc = 'sha1';
  if (drupal_strlen($key) > $blocksize)
      $key=pack('H*', $hashfunc($key));
  $key = str_pad($key, $blocksize, chr(0x00));
  $ipad = str_repeat(chr(0x36), $blocksize);
  $opad = str_repeat(chr(0x5c), $blocksize);
  $hmac = pack(
              'H*', $hashfunc(
                  ($key^$opad) . pack(
                      'H*', $hashfunc(
                          ($key^$ipad) . $data
                      )
                  )
              )
          );
  return $hmac;
}

/**
 * Very lightweight helper function to generate a TinyURL for a given post.
 */
function gigya_shorten_url($url) {
  $response = drupal_http_request('http://tinyurl.com/api-create.php?url=' . $url);
  if ($response->code == 200) {
    return $response->data;
  }
  else {
    return $url;
  }
}

/**
 * Validates the Gigya session keys.
 *
 * We use the site 'admin' username to find out the status. If it shows the
 * user logged out, thats good, if it returns an error then our keys are
 * most likely bad.
 */
function gigya_validate($update = FALSE) {

  $orig_validate_state = variable_get('gigya_validate', FALSE);

  $gigya_params = gigya_load_params();
  $gigya_params['uid'] = 'admin';
  $signature = _gigya_calc_signature(variable_get('gigya_SecretKey', FALSE), 'GET', 'getUserInfo', $gigya_params);
  $url = 'http://' . variable_get('gigya_api_url', 'socialize.api.gigya.com') . '/socialize.getUserInfo'.
    '?apiKey=' . drupal_urlencode($gigya_params['apiKey']) .
    '&timestamp=' . drupal_urlencode($gigya_params['timestamp']) .
    '&nonce=' . drupal_urlencode($gigya_params['nonce']) .
    '&sig=' . drupal_urlencode($signature) .
    // drupal_urlencode doesn't take out slashes.
    '&uid=' . urlencode($gigya_params['uid']);
  $headers = array('Content-type' => 'application/x-www-form-urlencoded');
  $results = drupal_http_request($url, $headers, 'GET');

  if ($error = _gigya_request_failure($results, TRUE)) {
    variable_set('gigya_validate', FALSE);
    if ($error == 403003) {
      if (!$update) {
        drupal_set_message(t('Your request signature is invalid. This is most likely caused by invalid site and/or private keys. Gigya is disabled until this error message is cleared.'), 'error');
      }
      return FALSE;
    }
    elseif ($error == 400006) {
      if (!$update) {
        drupal_set_message(t('Your request signature is invalid. This is most likely caused by invalid API key. Gigya is disabled until this error message is cleared.'), 'error');
      }
      return FALSE;
    }
     elseif ($error == 403007) {
      if (!$update) {
        drupal_set_message(t('Your request signature is invalid. This is most likely caused by invalid SecretKey parameter. Gigya is disabled until this error message is cleared.'), 'error');
      }
      return FALSE;
     }
    elseif  ($error == 400093) {
      if (!$update) {
        drupal_set_message(t('Your request signature is invalid. This is most likely caused by invalid API key. Gigya is disabled until this error message is cleared.'), 'error');
      }
      return FALSE;
    }
    //this ignores "Unauthorized user" and "Invalid uid parameter" when trying to validate the keys when gigya will fix the API
    elseif  ($error == 400095 || $error == 403005) {
      if (!$update && variable_get('need_validation_msg', FALSE) && variable_get('gigya_validate', FALSE)) {
        drupal_set_message(t('Gigya validated properly. This site is authorized to use Gigya services'));
        variable_set('need_validation_msg', TRUE);
        variable_set('gigya_validate', TRUE);
        return TRUE;
      }
    }

  }

  if (!$orig_validate_state || variable_get('need_validation_msg', FALSE)) {
    drupal_set_message(t('Gigya validated properly. This site is authorized to use Gigya services'));
    variable_set('need_validation_msg', TRUE);
  }

  variable_set('gigya_validate', TRUE);

  return TRUE;
}

/****************************
 * Gigya REST API functions
 ****************************/

 /*
 * Delete user from Gigya's DB
 */
function gigya_delete_account($uid) {
  if (empty($uid))
    return FALSE;

  $params = array(
    'uid' => $uid
  );

  return _gigya_api('deleteAccount',$params);
}

/*
 * Notifies Gigya that we've completed registering this user
 */
function gigya_notify_registration($old_uid, $new_uid) {
  if (empty($old_uid) || empty($new_uid))
    return FALSE;

  $params = array(
    'uid' => $old_uid,
    'siteUID' => $new_uid
  );

  return _gigya_api('notifyRegistration',$params);
}


/*
 * Sets the Gigya UID to match the Drupal UID
 */
function gigya_set_uid($old_uid, $new_uid) {
  if (empty($old_uid) || empty($new_uid))
    return FALSE;

  $params = array(
    'uid' => $old_uid,
    'siteUID' => $new_uid
  );

  return _gigya_api('setUID',$params);
}

/**
 * Fetches information about users following a given Gigya account.
 */
function gigya_get_user_info($gigya_uid) {
  if (empty($gigya_uid))
    return FALSE;

  $params = array(
    'uid' => $gigya_uid,
  );

  return _gigya_api('getUserInfo',$params);
}

/**
 * Undocumented Function!
 */
function gigya_get_friends_info($gigya_uid) {
  if (empty($gigya_uid))
    return FALSE;

  $params = array(
    'uid' => $gigya_uid,
  );

  return _gigya_api('getFriendsInfo',$params);
}

/**
 * Undocumented Function!
 */
function gigya_set_status($gigya_uid, $status) {
  if (empty($gigya_uid))
    return FALSE;

  $params = array(
    'uid' => $gigya_uid,
    'status' => $status
  );

  return _gigya_api('setStatus',$params);
}

/**
 * Undocumented Function!
 */
function gigya_publish_user_action($gigya_uid, $content) {
  if (empty($gigya_uid))
    return FALSE;

  $params = array(
    'uid' => $gigya_uid,
    'userAction' => _gigya_get_useraction_xml($content)
  );

  return _gigya_api('publishUserAction',$params);
}


/**
 * DEPRECATED - use setUID
 * Create a linkage in Gigya Socialize between the current Drupal user and gigya user (Drupal uid<->Gigya uid)
 */
function gigya_link_accounts_rest($gigya_uid,$drupalUid) {
  if (empty($gigya_uid))
    return FALSE;

  $params = array(
    'uid' => $gigya_uid,
    'siteUID' => $drupalUid
  );

  return _gigya_api('linkAccounts',$params);
}

/**
 * DEPRECATED - use deleteAccount
 * Break the linkage in Gigya Socialize between the current Drupal user and gigya user (Drupal uid != Gigya uid)
 */
function gigya_unlink_accounts($gigya_uid,$drupalUid) {
  if (empty($gigya_uid))
    return FALSE;

  $params = array(
    'uid' => $gigya_uid,
    'siteUID' => $drupalUid
  );

  return _gigya_api('unlinkAccounts',$params);
}

/*
 * Handles all calls to the Gigya API
 */
function _gigya_api($method, $params){
  $gigya_params = gigya_load_params();
  $params = array_merge($gigya_params, $params);
  $params['sig'] = _gigya_calc_signature(variable_get('gigya_SecretKey', FALSE), 'GET', $method, $params);
  $url = url("http://". variable_get('gigya_api_url', 'socialize.api.gigya.com') ."/socialize.$method",array('query' => $params));

  $headers = array('Content-type' => 'application/x-www-form-urlencoded');
  $results = drupal_http_request($url, $headers, 'GET');

  if (_gigya_request_failure($results)) {
    return FALSE;
  }

  return _gigya_convert_xml_to_array($results->data);
}

/**
 * Undocumented Function!
 */
function _gigya_get_useraction_xml($content) {
  $template = $content['template'];
  $action = $content['action'];
  $title = htmlspecialchars($content['title']);
  $path = htmlspecialchars(url($content['path'], array('absolute' => TRUE)));
  $xml = '<userAction><template>$template</template><actionName>$action</actionName><templateFields><templateField><name>thing</name><text>$title</text><href>$path</href></templateField></templateFields></userAction>';
  return $xml;
}

/**
 * Undocumented Function!
 */
function _gigya_calc_signature($secretkey, $httpmethod, $apimethod, $parameters) {
  require_once('OAuth.inc');
  $req = OAuthRequest::from_request($httpmethod, 'http://socialize.api.gigya.com/socialize.' . $apimethod, $parameters);
  return base64_encode(hash_hmac('sha1', $req->get_signature_base_string(), base64_decode($secretkey), TRUE));
}

/**
  * Internal helper function to deal cleanly with various HTTP response codes.
  */
function _gigya_request_failure($results, $return_error = FALSE) {
  $code = $results->code;
  $error = array();
  if(variable_get('gigya_debug', 0) && isset($results->error)) {
    watchdog('gigya_debug', $results->error, NULL, WATCHDOG_WARNING);
  }
  if ($code == 200) {
    if($result = _gigya_convert_xml_to_array($results->data)){
      if(((int) $result['errorCode']) !== 0){
        $error['type'] = 'gigya';
        $error['code'] = $result['errorCode'];
        if ($error['code'] == '403005' && variable_get('gigya_validate', FALSE)) {
          //if this is a validation error and the site was validated then do nothing so we won't litter the watchdog
        }
        else {
          $error['message'] = 'Gigya API error '. $result['errorCode'] .': '. $result['errorMessage'];
          watchdog('gigya',  $error['message'], NULL, WATCHDOG_WARNING);
        }
      }
    }
    else{
      $error['type'] = 'gigya';
      $error['code'] = 0;
      $error['message'] = 'Invalid XML response from Gigya service';
      watchdog('gigya', $error['message'], NULL, WATCHDOG_WARNING);
    }
  }
  else{
    //there's an HTTP issue of some kind
    $error['type'] = 'http';
    $error['code'] = $code;
    switch ($code) {
      case 304:
        $error['message'] = 'HTTP 304: Not Modified';
        break;
      case 400:
        $error['message'] = 'HTTP 400: Bad request (you may have exceeeded the Gigya rate limit)';
        break;
      case 401:
        $error['message'] = 'HTTP 401: Not Authorized';
        break;
      case 403:
        $error['message'] = 'HTTP: 403: Forbidden';
        break;
      case 404:
        $error['message'] = 'HTTP 404: Not found';
        break;
      case 500:
        $error['message'] = 'HTTP 500: Internal Server Error';
        break;
      case 502:
        $error['message'] = 'HTTP 502: Gigya servers may be down or being upgraded.';
        break;
      case 503:
        $error['message'] = 'HTTP 503: Gigya servers may be overloaded with requests.  Please try again later.';
        break;
    }
    $error['message'] = ($error['message'] ? $error['message'] : "HTTP $code");
    watchdog('gigya', $error['message'], (array) $results, WATCHDOG_WARNING);
  }

  if(!empty($error['code'])){
    if ($return_error)
      return $error['code'];
    else
      return TRUE;
  }
  else{
    //no errors
    return FALSE;
  }
}

/**
 * Internal XML munging code
 */
function _gigya_convert_xml_to_array($data) {
  try {
    $xml = new SimpleXMLElement($data);
    return get_object_vars($xml);
  } catch (Exception $e){
    watchdog('gigya', $e->getMessage(), $data, WATCHDOG_WARNING);
    return false;
  }
}
