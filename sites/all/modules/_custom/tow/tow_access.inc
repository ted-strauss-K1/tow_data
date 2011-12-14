<?php
// $Id: tow_access.inc,v 1.0.0.0 2011/02/28 23:42:15 tow Exp $
/**
 * @file
 * Access functions
 */

/* 
 * access types:
 * 0 open
 * 1 by request
 * 2 private
 */

/*
 * access statuses:
 * -1 denied
 *  0 pending
 *  1 granted
 */

/**
 * Deny access for dataset if access is forbidden
 */
function tow_access_dataset($node) {
  
  global $user;
  if ($user->uid != $node->uid) {
  	$args = arg();
    // if access is not open
    $access_type = $node->field_access_type[0]['value'];
    if ($access_type == 0) {
      if ($user->uid == 0) {
        if (tow_access_bypass_solr())
          return;
        if ($args[0] == 'node' && $args[1] == $node->nid && count($args) == 2) {
          global $grey_pma_menu;
          $grey_pma_menu = TRUE;
        }
        else {
          drupal_access_denied();
          die();
        }
      }
      return;
    }
    // if access is by request
    if ($access_type == 1) {
      // dataset summary page
      if ($args[0] == 'node' && $args[1] == $node->nid && count($args) == 2) {
        if ($user->uid) {
          $status = db_result(db_query("SELECT status FROM {tow_access_requests} WHERE nid = %d AND uid = %d", $node->nid, $user->uid));
        }
        if ($status != 1 && !user_access('edit any dataset content')) {
          if (tow_access_bypass_solr())
            return;
          global $grey_pma_menu;
          $grey_pma_menu = TRUE;
        }
        return;
      }      
    }

    if ($user->uid) {
      $status = db_result(db_query("SELECT status FROM {tow_access_requests} WHERE nid = %d AND uid = %d", $node->nid, $user->uid));
    }
    // if user has no access
    if ($status != 1) {
      if (user_access('edit any dataset content')) {
        return;
      }
      // from dataset summary perform redirect to dataset structure to avoid eternal cycling
      if (tow_access_bypass_solr())
        return;
      if ($args[0] == 'node' && $args[1] == $node->nid)
        drupal_goto('dataset/' . $node->nid);
      drupal_access_denied();
      die();
    }
  }
  
}

/**
 * Check if http request is performed from local machine
 */
function tow_access_bypass_solr() {
  static $bypass_solr;
  
  if (isset($bypass_solr)) {
    
    if ($bypass_solr)
      return TRUE;    
    else
      return FALSE;

  }
  
  if (ip_address() == $_SERVER['HTTP_HOST']) {
    $bypass_solr = TRUE;
    return TRUE;
  }
  else {
    $bypass_solr = FALSE;
    return FALSE;
  }
}

/**
 * @return form for dataset access type settings
 */
function tow_access_type_form() {
  global $dataset;
  $options = array(t('Open'), t('By request'), t('Private'));
  $current_access_type = $dataset->field_access_type[0]['value'];
  $form['description'] = array(
    '#type' => 'item',
    '#value' => t('User access set to: ') . $options[$current_access_type] . '. ' . t('Change user access:'),
  );
  $form['access_type'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => $current_access_type,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  return $form;
}

/**
 * Submission callback
 */
function tow_access_type_form_submit($form, &$form_state) {
  global $dataset;
  $dataset->field_access_type[0]['value'] = $form_state['values']['access_type'];
  node_save($dataset);
}

/**
 * Dependently on status from tow_access_requests @return access request button (nothing in that table) or message of request status 
 */
function tow_access_request_widget($nid, $uid) {
  
  $request = db_result(db_query("SELECT status FROM {tow_access_requests} WHERE nid = %d AND uid = %d LIMIT 1", $nid, $uid));
  
  if ($request === FALSE)
    return drupal_get_form('tow_access_request_form');
  
  switch ($request) {
    case 0:
      return t('Access request submitted');
    case 1:
      return t('Access granted');
    case -1:
      return t('Access denied');
  }
}

/**
 * @return access request button
 */
function tow_access_request_form() {
  global $dataset;
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Request access from ') . $dataset->name 
  );
  return $form;
}

/**
 * Submission callback
 */
function tow_access_request_form_submit($form, &$form_state) {
  global $dataset;
  global $user;
  if ($user->uid) {
    db_query("INSERT INTO {tow_access_requests} (nid, uid, status, request_timestamp) VALUES (%d, %d, 0, %d)", $dataset->nid, $user->uid, time());
  }else {
    drupal_access_denied();
    die();
  }
}

/**
 * @return table with access requests
 */
function tow_access_requests($nid) {
  $res = db_query("
    SELECT t.request_timestamp, t.status, t.uid, u.name 
    FROM {tow_access_requests} t INNER JOIN {users} u
    ON t.uid = u.uid 
    WHERE t.nid = %d
    AND u.status = 1
    ", $nid);
  while ($r = db_fetch_array($res)) {
    
    if ($r['status'] == 1) {
      $status = t('Yes');
    }else{
      $status = t('No');
    }
    
    switch ($r['status']) {
      case 1:
        $action = l('deny', 'access/deny/' . $nid . '/' . $r['uid']);
        break;
      case -1:
        $action = l('revoke', 'access/grant/' . $nid . '/' . $r['uid']);
        break;
      default:
        $action = l('grant', 'access/grant/' . $nid . '/' . $r['uid']);
        $action .= ' / ';
        $action .= l('deny', 'access/deny/' . $nid . '/' . $r['uid']);
    }     
    
    $rows[] = array(
      l($r['name'], 'user/' . $r['uid']), 
      date('d-m-Y', $r['request_timestamp']), 
      $status, 
      $action
    );
    
  }
  
  if ($rows) {
    $headers = array(t('user'), t('request date'), t('current access'), t('action'));
    return theme('table', $headers, $rows);
  }
}

/**
 * @return confirmation form for dataset access settings
 */
function tow_access_confirm($form_state, $action, $nid, $uid) {

  global $user;
  global $dataset;
  $dataset = node_load($nid);
  
  if ($user->uid == $dataset->uid || user_access('edit any dataset content')) {
    if (  ($action == 'grant' || $action == 'deny')  && is_numeric($nid) && is_numeric($uid)  ) {
      
      $requester = user_load($uid);
    
      $title = ucfirst(t($action)) . ' ' . $requester->name . ' ' . t('access') . ' ' . $dataset->title . '?';
      //drupal_set_title($title);
      
      $form['action'] = array(
        '#type' => 'value',
        '#value' => $action
      );
      
      $form['nid'] = array(
        '#type' => 'value',
        '#value' => $nid
      );
      
      $form['uid'] = array(
        '#type' => 'value',
        '#value' => $uid
      );
      
      return confirm_form($form,
        $title,  
        'node/'. $nid,
        "",
        t('Confirm'),
        t('Cancel')
      );
      
    }
  }
  drupal_access_denied();
  die();  
}

/**
 * Submission callback
 */
function tow_access_confirm_submit($form, &$form_state) {
  
  if ($form_state['values']['action'] == 'grant')
    $status = 1;
  else
    $status = -1;
    
  db_query("UPDATE {tow_access_requests} SET status = %d WHERE nid = %d AND uid = %d", $status, $form_state['values']['nid'], $form_state['values']['uid']);
  $form_state['redirect'] = 'node/' . $form_state['values']['nid'];  
}