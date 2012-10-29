<?php
/**
 * Implements hook_token_list().
 */
function ulogin_token_list($type = 'all') {
  $tokens = array();
  
  if ($type == 'user' || $type == 'all') {
    $tokens['user']['ulogin_network'] = t('Network providing authentication.');
    $tokens['user']['ulogin_uid'] = t('UID') . ' ' . t('provided by uLogin.');
    $tokens['user']['ulogin_identity'] = t('Identity') . ' ' . t('provided by uLogin.');
    foreach (_ulogin_fields_list() as $key => $value) {
      $tokens['user']['ulogin_' . $key] = $value . ' ' . t('provided by uLogin.');
    }
  }
  
  return $tokens;
}

/**
 * Implements hook_token_values().
 */
function ulogin_token_values($type, $object = NULL, $options = array()) {
  global $ulogin_data;
  $values = array();
  
  if ($type == 'user' && !empty($ulogin_data)) { //for just created user
    $data = $ulogin_data;
  }
  elseif ($type == 'user' && !empty($object->ulogin)) { //for existing user
    $data = $object->ulogin;
  }
  
  if (!empty($data)) {
    $values['ulogin_network'] = $data['network'];
    $values['ulogin_uid'] = $data['uid'];
    $values['ulogin_identity'] = $data['identity'];
    $values['ulogin_email'] = $data['email'];
    $values['ulogin_nickname'] = $data['nickname'];
    $values['ulogin_first_name'] = $data['first_name'];
    $values['ulogin_last_name'] = $data['last_name'];
    $values['ulogin_bdate'] = $data['bdate'];
    $values['ulogin_sex'] = $data['sex'];
    $values['ulogin_photo'] = $data['photo'];
    $values['ulogin_photo_big'] = $data['photo_big'];
    $values['ulogin_country'] = $data['country'];
    $values['ulogin_city'] = $data['city'];
    $values['ulogin_phone'] = $data['phone'];
  }
  
  return $values;
}
