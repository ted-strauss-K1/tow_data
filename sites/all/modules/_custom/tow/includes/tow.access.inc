<?php

/**
 * @file
 *
 * Access functions for Theory on Wheels.
 * 
 *  access types:
 *    0 - open
 *    1 - by request
 *    2 - private
 *
 *  access statuses:
 *    -1 - denied
 *     0 - pending
 *     1 - granted
 */



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Check if http request is performed from the local machine.
 */
function tow_access_bypass_solr() {
  static $bypass_solr;
  
  // Check if bypass is already set and return if so.
  if (!isset($bypass_solr)) {
    
    // Check if it's the local machine and return the value.
    $bypass_solr = (ip_address() == '127.0.0.1') ? TRUE : FALSE;
  }
  
  return ($bypass_solr) ? TRUE : FALSE;
}

/**
 * Deny access for dataset if access is forbidden.
 */
function tow_access_dataset($node) {
  global $user;
  
  if ($user->uid != $node->uid) {
    $args = arg();
    
    // Get access type.
    $access_type = $node->field_access_type[0]['value'];
    
    // If access is open.
    if ($access_type == 0) {
      if ($user->uid == 0) {
        if (tow_access_bypass_solr()) {
          return;
        }
        if ($args[0] == 'node' && $args[1] == $node->nid && count($args) == 2) {
          global $_tow_grey_pma_menu;
          $_tow_grey_pma_menu = TRUE;
        }
        else {
          drupal_access_denied();
          die();
        }
      }
      return;
    }
    
    // If access is by request.
    if ($access_type == 1) {
      
      // Dataset summary page.
      if ($args[0] == 'node' && $args[1] == $node->nid && count($args) == 2) {
        if ($user->uid) {
          $status = db_result(db_query("
          
            SELECT 
              tar.status AS status 
              
              FROM 
                {tow_access_requests} tar 

              WHERE 
                  tar.nid = %d 
                AND 
                  tar.uid = %d
          
          ", $node->nid, $user->uid));
        }
        if ($status != 1 && !user_access('edit any dataset')) {
          if (tow_access_bypass_solr()) {
            return;
          }
          global $_tow_grey_pma_menu;
          $_tow_grey_pma_menu = TRUE;
        }
        return;
      }
    }

    // Get  status.
    $status = 0;
    if ($user->uid) {
      $status = db_result(db_query("
      
        SELECT 
          tar.status 
          
          FROM 
            {tow_access_requests} tar 
          
          WHERE 
              tar.nid = %d 
            AND 
              tar.uid = %d
      
      ", $node->nid, $user->uid));
    }
    
    // If user has no access.
    if ($status != 1) {
      if (user_access('edit any dataset', $user)) {
        return;
      }
      
      // From dataset summary perform redirect to dataset structure to avoid eternal cycling.
      if (tow_access_bypass_solr()) {
        return;
      }
      if ($args[0] == 'node' && $args[1] == $node->nid) {
        drupal_goto('dataset/' . $node->nid);
      }
      
      drupal_access_denied();
      die();
    }
  }
  
}

/**
 * Returns access status string ('open', 'by request', 'private') corresponding to number notation.
 */
function tow_access_get_access_status($code) {
  switch ($code) {  
    case 0: 
      return 'open';
    break;
    case 1: 
      return 'by request';
    break;
    case 2: 
      return 'private';
    break;
    default:
      return FALSE;
    break;
  }
}

/**
 * Dependently on status from tow_access_requests return access request button (nothing in that table) or message of request status. 
 */
function tow_access_request_widget($nid, $uid) {
  
  // Get status.
  $request = db_result(db_query("
  
    SELECT 
      tar.status AS status 
      
      FROM 
        {tow_access_requests} tar 
      
      WHERE 
          tar.nid = %d 
        AND 
          tar.uid = %d 
          
      LIMIT 1
  
  ", $nid, $uid));
  
  // Return an access request button if $request is FALSE.
  if ($request === FALSE) {
    return drupal_get_form('tow_access_request_form');
  }
  
  // Return a message according to request's value.
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
 * Returns table with access requests.
 */
function tow_access_requests($nid) {
  
  $res = db_query("
    
    SELECT 
      t.request_timestamp AS request_timestamp, 
      t.status AS status, 
      t.uid AS uid, 
      u.name AS name 
    
      FROM 
          {tow_access_requests} t 
        INNER JOIN 
          {users} u
          ON 
            t.uid = u.uid 
    
      WHERE 
          t.nid = %d
        AND 
          u.status = 1

  ", $nid);
  while ($r = db_fetch_array($res)) {
    
    // Save status.
    if ($r['status'] == 1) {
      $status = t('Yes');
    }
    else {
      $status = t('No');
    }
    
    // Form links.
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
    
    // Set a new row.
    $rows[] = array(
      l($r['name'], 'user/' . $r['uid']), 
      date('d-m-Y', $r['request_timestamp']), 
      $status, 
      $action,
    );
    
  }
  
  // Return a themed table on existing rows...
  if ($rows) {
  $title = 'List of access requests and accepted invitations<br>';
    $headers = array(t('user'), t('request/invitation date'), t('current access'), t('action'));
    return $title . theme('table', $headers, $rows);
  }
  
  // ...return the following text otherwise.
  return 'There are no access requests and accepted invitations for this dataset.';
}



/****************************************************************************************************
 * Forms
 ****************************************************************************************************/

/**
 * Returns confirmation form for dataset access settings.
 */
function tow_access_confirm($form_state, $action, $nid, $uid) {

  global $user;
  global $_tow_dataset;
  $_tow_dataset = node_load($nid);
  
  // Do this only for permited users...
  if ($user->uid == $_tow_dataset->uid || user_access('edit any dataset')) {
    
    // Grant or Deny actions.
    if (($action == 'grant' || $action == 'deny') && is_numeric($nid) && is_numeric($uid)) {
      
      // Params.
      $requester = user_load($uid);
      $title = ucfirst(t($action)) . ' ' . $requester->name . ' ' . t('access') . ' ' . $_tow_dataset->title . '?';
      
      // Form.
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
      
      // Add confirmation elements.
      $description = '';
      return confirm_form($form, filter_xss($title), 'node/'. $nid, filter_xss($description), t('Confirm'), t('Cancel'));
    }
    
    // Open action.
    if (($action == 'open')  && is_numeric($nid)) {
    
      // Params.
      $title = 'Open access for <i>' . $_tow_dataset->title . '</i>';
      $description = 'This will clear all information on granting and denial access to this dataset for other users.';
    
      // Form.
      $form = array();
      $form['action'] = array(
        '#type' => 'value',
        '#value' => $action
      );
      $form['nid'] = array(
        '#type' => 'value',
        '#value' => $nid
      );

      // Add confirmation elements.
      return confirm_form($form, filter_xss($title), 'node/'. $nid, filter_xss($description), t('Confirm'), t('Cancel'));      
    }
  }
  
  // ...and deny an access otherwise.
  drupal_access_denied();
  die();  
}

/**
 * Returns form for dataset access type settings.
 */
function tow_access_type_form() {
  global $_tow_dataset;
  
  // Auxiliary variables.
  $options = array(t('Open'), t('By request'), t('Private'));
  $current_access_type = $_tow_dataset->field_access_type[0]['value'];
  
  // Form elements.
  $form['description'] = array(
    '#type' => 'item',
    '#value' => t('User access set to: ') . $options[$current_access_type] . '. ' . t('Change user access:'),
  );
  $form['access_type'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => $current_access_type,
  );
  
  return $form;
}

/**
 * Returns access request button.
 */
function tow_access_request_form() {
  global $_tow_dataset;
  
  // Form elements.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Request access from ') . $_tow_dataset->name 
  );
  
  return $form;
}



/****************************************************************************************************
 * Callback functions
 ****************************************************************************************************/

/**
 * Access confirmation form submission callback.
 */
function tow_access_confirm_submit($form, &$form_state) {
  
  // Case 'Open' action.
  if ($form_state['values']['action'] == 'open') {
    
    // Save dataset.
    global $_tow_dataset;
    $_tow_dataset->field_access_type[0]['value'] = 0;
    node_save($_tow_dataset);

    // Remove auxiliary data.
    db_query("

      DELETE 
      
        FROM 
          {tow_access_requests} tar 

        WHERE 
          tar.nid = %d

    ", $form_state['values']['nid']);
  }
  
  // Case 'Grant' or 'Deny' actions.
  else {
    
    // Get status.
    if ($form_state['values']['action'] == 'grant') {
      $status = 1;
    }
    else {
      $status = -1;
    }
    
    // Update db records.
    db_query("
      
      UPDATE 
        {tow_access_requests} 
        
        SET 
          status = %d 
        
        WHERE 
            nid = %d 
          AND 
            uid = %d
            
    ", $status, $form_state['values']['nid'], $form_state['values']['uid']);
  }
  
  // Redirect.
  $form_state['redirect'] = 'node/' . $form_state['values']['nid'];  
}

/**
 * Request form submission callback.
 */
function tow_access_request_form_submit($form, &$form_state) {
  global $_tow_dataset;
  global $user;
  
  // Write a record to the db if the user is an authentificated one.
  if ($user->uid) {
    db_query("
    
      INSERT 
        INTO 
          {tow_access_requests} 
            (nid, uid, status, request_timestamp) 
          
          VALUES 
            (%d, %d, 0, %d)
            
    ", $_tow_dataset->nid, $user->uid, time());
  }
  
  // Show error 403 otherwise.
  else {
    drupal_access_denied();
    die();
  }
}
