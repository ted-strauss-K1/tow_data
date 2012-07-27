<?php

/**
 * @file
 * 
 * Invitation interface.
 */



/****************************************************************************************************
 * Forms
 ****************************************************************************************************/

/**
 * Returns form for sending invite.
 */
function tow_invite_form($form_state, $nid) {
  
  global $_tow_dataset;
  global $user;
  
  // Load dataset.
  $_tow_dataset = node_load($nid);
  
  // Page not found if not a dataset.
  if ($_tow_dataset->type !== 'dataset') {
    drupal_not_found();
    die();
  }
  
  // Deny access if user is not permitted to work with this dataset.
  if ($_tow_dataset->uid != $user->uid && !user_access('edit any dataset')) {
    drupal_access_denied();
    die();
  }
  
  // Set title.
  drupal_set_title(t('Invite people to access %title', array('%title' => $_tow_dataset->title)));
  
  // E-mail.
  $form['email'] = array(
    '#title' => 'E-mail address',
    '#description' => 'Enter e-mail address of person who will have the access',
    '#type' => 'textfield',
    '#required' => TRUE
  );
  
  // Message.
  $form['message'] = array(
    '#title' => 'Message',
    '#description' => 'Enter text of message you want to send',
    '#type' => 'textarea',
  );
  
  // Submit.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Invite')
  );
  
  // Redirect.
  $form['#redirect'] = 'node/' . $_tow_dataset->nid; 
  
  return $form;
}

/**
 * Invitation form submission callback.
 */
function tow_invite_form_submit($form, &$form_state) {
  
  global $_tow_dataset;
  global $user;
  
  // Get e-mail adress.
  $email = $form_state['values']['email'];
  
  $res = db_query("
  
    SELECT 
      u.uid AS uid, 
      u.name AS name 
      
      FROM 
        {users} u 
      
      WHERE 
        u.mail = '%s' 
      
      LIMIT 1
  
  ", $email);
   
  if ($person = db_fetch_object($res)) {
    
    // If person to be invited is our existing user.
    $exists = db_result(db_query("
    
      SELECT 
        t.nid AS nid, 
        t.uid AS uid
        
        FROM 
          {tow_access_requests} t
        
        WHERE 
            t.nid = %d 
          AND 
            t.uid = %d
            
    ", $_tow_dataset->nid, $person->uid));
    
    if (!$exists) {
      db_query("
        
        INSERT 
          INTO 
            {tow_access_requests} 
              (nid, uid, status, request_timestamp)
      
          VALUES 
            (%d, %d, 1, %d)
      
      ", $_tow_dataset->nid, $person->uid, time());
    }
    else {
     db_query("
        
        UPDATE 
        {tow_access_requests} 
        
        SET 
          status = %d, 
          request_timestamp = %d
        
        WHERE 
            nid = %d 
          AND 
            uid = %d
      
      ", 1, time(), $_tow_dataset->nid, $person->uid);
    }
    
    $link = l($_tow_dataset->title, 'node/' . $_tow_dataset->nid, array('absolute' => TRUE));
    if (tow_invite_user($email, $link, $form_state['values']['message'], $person->name)) {
      drupal_set_message('Invite has been sent');
    }
    else {
      drupal_set_message('Invite has not been sent');
    }
  }
  else {
    if (db_result(db_query_range("SELECT nid FROM {tow_invites} WHERE nid = %d AND email = '%s'", array($_tow_dataset->nid, $email), 0, 1))) {
      drupal_set_message('Invite had already been sent');
      return;
    }
    $token = user_password(32);
    $link = l($_tow_dataset->title, 'node/' . $_tow_dataset->nid, array('absolute' => TRUE));
    $tokenlink = l('link', 'user/register', array('query' => 'token=' . $token, 'absolute' => TRUE));
    if (tow_invite_user($email, $link, $form_state['values']['message'], $person, $tokenlink)) {
      db_query("INSERT INTO {tow_invites} VALUES ('%d', '%s', '%s')", $_tow_dataset->nid, $token, $email);
      drupal_set_message('Invite has been sent');
    }
    else{
      drupal_set_message('Invite has not been sent');
    }
  }
}



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Set in the db that user invited has acces to dataset.
 * Delete corresponding invite from the db.
 * $account is an object of user that has just been created.
 * tow_invite_register() has just fired so there is no need of token validation.
 */
function tow_invite_insert($account) {
  $token = $_GET['token'];
  if ($token) {
    $nid = db_result(db_query("
    
      SELECT 
        t.nid AS nid 
        
        FROM 
          {tow_invites} t 
        
        WHERE 
          t.token = '%s'
          
    ", $token));
    $exists = db_result(db_query("
    
      SELECT 
        t.nid AS nid, 
        t.uid AS uid
        
        FROM 
          {tow_access_requests} t
        
        WHERE 
            t.nid = %d 
          AND 
            t.uid = %d
            
    ", $nid, $account->uid));
    if ($exists) {
      db_query("
      
        INSERT 
          INTO 
            {tow_access_requests} 
              (nid, uid, status, request_timestamp)

          VALUES 
            (%d, %d, 1, %d)
      
      ", $nid, $account->uid, time());
    }
    else {
     db_query("
        
        UPDATE 
        {tow_access_requests} 
        
        SET 
          status = %d, 
          request_timestamp = %d
        
        WHERE 
            nid = %d 
          AND 
            uid = %d
      
      ", 1, time(), $nid, $account->uid);
    }
    db_query("
      
      DELETE 
        
        FROM 
          {tow_invites} t 
        
        WHERE 
          t.token = '%s'
          
    ", $token);
  }
}

/**
 * On user/register?token=[token] page deny access if token is not valid.
 * Do nothing if there is no ?token part of query.
 */
function tow_invite_register() {
  $token = $_GET['token'];
  if ($token) {
    if (strlen($token) != 32 || !db_result(db_query_range("SELECT nid FROM {tow_invites} WHERE token = '%s'", array($token), 0, 1))) {
      drupal_access_denied();
      die();
    }
  }
}

/**
 * Send invite.
 */
function tow_invite_user($email, $link, $user_message, $person, $tokenlink = FALSE) {
  
  global $user;
  
  // Define the body of the message.
  $body = '<p>Hello ' . $person . '</p>';
  $body .= '<p>' . $user->name . ' invites you to have access to his/her dataset ' . $link . ' on Theory on Wheels</p>';
  if ($tokenlink) {
    $body .= '<p>Since access to this dataset is restricted, to get an access you need to register on Theory on Wheels first using a special ' . $tokenlink . '</p>';
  }
  if ($user_message) {
    $body .= '<p>' . $user->name . ' included the message:</p>';
    $body .= '<pre><i>' . $user_message . '</i></pre>';
  }
  $body .= 'Good luck!';
  
  // Headers.
  $headers = array(
    'MIME-Version' => '1.0',
    'Content-Type' => 'text/html; charset=UTF-8; format=flowed; delsp=yes',
    'Content-Transfer-Encoding' => '8Bit',
    'X-Mailer' => 'Drupal',
  );
  
  // Message.
  $message = array(
    'id' => 'invite',
    'to' => $email,
    'subject' => 'Access to dataset on Theory on Wheels',
    'body' => $body,
    'headers' => $headers,
    'from' => variable_get('smtp_from', 'notify@tow.com')
  );

  // Send mail.
  return drupal_mail_send($message); 
}
