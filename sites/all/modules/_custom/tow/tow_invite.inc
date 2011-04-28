<?php
// $Id: tow_invite.inc,v 1.0.0.0  2011/02/28 15:32:21 tow Exp $

/**
 * @file
 */

function tow_invite_form($form_state, $nid) {
  
  global $dataset;
  global $user;
  
  $dataset = node_load($nid);
  
  if ($dataset->type !== 'dataset') {
    drupal_not_found();
    die();
  }
  
  if ($dataset->uid != $user->uid && !user_access('edit any dataset content')) {
    drupal_access_denied();
    die();
  }
  
  drupal_set_title(t('Invite people to access %title', array('%title' => $dataset->title)));
  
  $form['email'] = array(
    '#title' => 'E-mail address',
    '#description' => 'Enter e-mail address of person who will have the access',
    '#type' => 'textfield',
    '#required' => TRUE
  );
  
  $form['message'] = array(
    '#title' => 'Message',
    '#description' => 'Enter text of message you want to send',
    '#type' => 'textarea',
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Invite')
  );
  
  $form['#redirect'] = 'node/' . $dataset->nid; 
  
  return $form;
}

function tow_invite_form_submit($form, &$form_state) {
  
  global $dataset;
  global $user;
  
  $email = $form_state['values']['email'];
  
  $res = db_query("SELECT uid, name FROM {users} WHERE mail = '%s' LIMIT 1", $email);
   
  if ($person = db_fetch_object($res)) {
    // If person to be invited is our existing user
    db_query("
      INSERT INTO {tow_access_requests} (nid, uid, status, request_timestamp)
      VALUES (%d, %d, 1, %d)
      ON DUPLICATE KEY UPDATE status = 1
      ", $dataset->nid, $person->uid, time());
    $link = l($dataset->title, 'node/' . $dataset->nid, array('absolute' => TRUE));
    tow_invite_user($email, $link, $form_state['values']['message'], $person->name);
  }else{
    if (db_result(db_query("SELECT nid FROM {tow_invites} WHERE nid = %d AND email = '%s' LIMIT 1", $dataset->nid, $email))) {
      drupal_set_message('Invite had already been sent');
      return;
    }
    $token = user_password(32);
    $link = l($dataset->title, 'user/register', array('query' => 'token=' . $token, 'absolute' => TRUE));
    if (tow_invite_user($email, $link, $form_state['values']['message'], $person)) {
      db_query("INSERT INTO {tow_invites} VALUES (%d, '%s', '%s')", $dataset->nid, $token, $email);
      drupal_set_message('Invite has been sent');
    }else{
      drupal_set_message('Invite has not been sent');
    }
  }
  
}

function tow_invite_user($email, $link, $user_message, $person) {
  
  global $user;
    
  $body = '<p>Hello ' . $person . '</p>';
  $body .= '<p>' . $user->name . ' invites you to have access to his/her dataset ' . $link . ' on Theory on Wheels</p>';
  if ($user_message) {
    $body .= '<p>' . $user->name . ' included the message:</p>';
    $body .= '<pre><i>' . $user_message . '</i></pre>';
  }
  $body .= 'Good luck!';
  
  $headers = array(
    'MIME-Version' => '1.0',
    'Content-Type' => 'text/html; charset=UTF-8; format=flowed; delsp=yes',
    'Content-Transfer-Encoding' => '8Bit',
    'X-Mailer' => 'Drupal',
  );
  
  $message = array(
    'id' => 'invite',
    'to' => $email,
    'subject' => 'Access to dataset on Theory on Wheels',
    'body' => $body,
    'headers' => $headers,
    'from' => variable_get('smtp_from', 'develop@una.by') //TODO: change e-mail address
  );

  return drupal_mail_send($message); 
    
}