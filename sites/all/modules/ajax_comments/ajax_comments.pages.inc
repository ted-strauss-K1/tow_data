<?php

/**
 * @file
 * AJAX comments form handling and callbacks.
 */

/**
 * AHAH callback.
 */
function ajax_comments_js() {
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  $form = ajax_comments_form_get_cache($form_build_id, $form_state);

  if ($form) {
    $args = $form['#parameters'];
    $form_id = array_shift($args);
  }
  // Build form from scratch (e.g., Authcache module does not cache forms within the caching system)
  else {
    $form_id = 'comment_form';
    $edit['nid'] = $_POST['nid'];
    $args = array($form_id, $edit);

    $form_state['post'] = $_POST;

    // Use a copy of the function's arguments for manipulation
    $args_temp = $args;
    $args_temp[0] = &$form_state;
    array_unshift($args_temp, $form_id);
  
    $form = call_user_func_array('drupal_retrieve_form', $args_temp);
    $form_build_id = 'form-'. md5(uniqid(mt_rand(), true));
    $form['#build_id'] = $form_build_id;    
    drupal_prepare_form($form_id, $form, $form_state);
    $original_form = $form;
    $cacheable = FALSE; // No need to cache; amost done
    $form['#post'] = $_POST;    
  }

  $form_state['post'] = $form['#post'] = $_POST;
  $form['#programmed'] = $form['#redirect'] = FALSE;

  if ($form_state['post']['op'] == $form_state['values']['preview']) {
    $form['#after_build'] = array('comment_form_add_preview');
  }
  else {
    unset($form['#after_build']);
  }

  drupal_process_form($form_id, $form, $form_state);


  $errors = form_get_errors();

  if (!$errors) {
    // Prepare output
    if ($form_state['values']['op'] == $form_state['values']['preview']) {
      $output = '<div class="preview-item">'. $form['comment_preview']['#value'] .'</div>';
      if ($output && $form_state['values']['pid']) {
        $output = '<div class="indented">'. $output .'<div>';
      }
    }
    elseif ($form_state['values']['op'] == $form_state['values']['submit']) {
      $output = '<div class="comment-new-success">'. $form_state['storage']['ajax_comment'] .'</div>';
    }
    $form = form_builder($form_id, $form, $form_state);

    // Add Submit button if preview was required
    $node = node_load($form_state['values']['nid']);
    $op = $form_state['values']['op'];
    if (variable_get('comment_preview_'. $node->type, COMMENT_PREVIEW_REQUIRED) == COMMENT_PREVIEW_REQUIRED && ($op == t('Preview') || $op == t('Save'))) {
      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save'),
        '#weight' => 19,
        '#executes_submit_callback' => TRUE,
      );
      $form['submit']['#id'] = "ajax-comments-submit";
      $form['submit']['#submit'] = array('ajax_comments_submit');
      $form['submit']['#ahah'] = array(
        'path' => 'ajax_comments/js',
        'wrapper' => 'comment-form-content',
        'event' => 'click',
        'method' => 'before',
        'effect' => 'ajaxCommentsSubmit',
        'progress' => array('type' => '1bar', 'message' => t('Please wait...')),
      );
      $form = form_builder($form_id, $form, $form_state);
      form_set_cache($form_build_id, $form, $form_state);
    }
  }
  unset($form['#suffix']);
  unset($form['#prefix']);
  $output = theme('status_messages') . $output;


  if ($output) {
    $javascript = drupal_add_js(NULL, NULL);
    if (isset($javascript['setting'])) {
      $output .= '<script type="text/javascript">jQuery.extend(Drupal.settings, '. drupal_to_js(call_user_func_array('array_merge_recursive', $javascript['setting'])) .');</script>';
    }

    drupal_json(array(
      'status'   => TRUE,
      'data'     => $output,
    ));
  }
}

/**
 * Fetch a form from cache.
 */
function ajax_comments_form_get_cache($form_build_id, &$form_state) {
  global $user;
  if ($cached = ajax_comments_cache_get('form_'. $form_build_id, 'cache_form')) {
    $form = $cached->data;
    if ((isset($form['#cache_token']) && drupal_valid_token($form['#cache_token'])) || (!isset($form['#cache_token']) && !$user->uid)) {
      if ($cached = ajax_comments_cache_get('storage_'. $form_build_id, 'cache_form')) {
        $form_state['storage'] = $cached->data;
      }
      // Rewind form values.
      $elements = element_children($form);
      if (!empty($elements)) {
        foreach ($elements as $element) {
          if (in_array($form[$element]['#type'], array('textfield', 'textarea', 'select', 'checkbox', 'radio', 'checkboxes', 'radios'))) {
            unset($form[$element]['#value']);
          }
        }
      }
      return $form;
    }
  }
}

/**
 * This one differs from standard by loading even expired cache instances.
 */
function ajax_comments_cache_get($cid, $table = 'cache') {
  global $user;

  // Garbage collection necessary when enforcing a minimum cache lifetime
  $cache_flush = variable_get('cache_flush', 0);
  if ($cache_flush && ($cache_flush + variable_get('cache_lifetime', 0) <= time())) {
    // Reset the variable immediately to prevent a meltdown in heavy load situations.
    variable_set('cache_flush', 0);
    // Time to flush old cache data
    
    // Support third-party caching systems such as CacheRouter
    if(variable_get('cache_inc', FALSE)) { 
      cache_clear_all(NULL, $table);
    } 
    // Drupal core caching tables
    else {
      db_query("DELETE FROM {". $table ."} WHERE expire != %d AND expire <= %d", CACHE_PERMANENT, $cache_flush);  
    }
  }

  // Support third-party caching systems such as CacheRouter
  if(variable_get('cache_inc', FALSE)) { 
    $cache = cache_get($cid, $table);
    if (isset($cache->data)) {
      return $cache;  
    }
  }
  // Drupal core caching tables
  else { 
    $cache = db_fetch_object(db_query("SELECT data, created, headers, expire, serialized FROM {". $table ."} WHERE cid = '%s'", $cid));
    if (isset($cache->data)) {
      $cache->data = db_decode_blob($cache->data);
      if ($cache->serialized) {
        $cache->data = unserialize($cache->data);
      }
      return $cache;
    }    
  }
  return 0;
}

/**
 * Delivers a new comments form after comment have been submitted.
 */
function ajax_comments_js_reload_form($nid, $id = NULL, $type = NULL) {
  if ($type == 'cid') {
    $cid = $id;
    $comment = db_fetch_object(db_query('SELECT c.*, u.uid, u.name AS registered_name, u.data FROM {comments} c INNER JOIN {users} u ON c.uid = u.uid WHERE c.cid = %d', $cid));
    $comment = drupal_unpack($comment);
    $comment->name = $comment->uid ? $comment->registered_name : $comment->name;
    $edit = (array)$comment;
  }
  else {
    $edit = array('nid' => $nid);
    if (isset($id)) {
      $edit['pid'] = $id;
    }
  }
  $response = array(
    'status' => TRUE,
    'content' => drupal_get_form('comment_form', $edit, $title),
  );
  drupal_json($response);
}

/**
 * Render a new comment thread.
 */
function ajax_comments_js_load_thread($nid) {
  global $conf;
  $node = node_load($nid);
  // Hide comments form from thread.
  $conf['comment_controls_'. $node->type] = COMMENT_CONTROLS_HIDDEN;
  // Render the thread.
  $response = array(
    'status' => TRUE,
    'content' => comment_bonus_api_comment_render($node),
  );
  drupal_json($response);
}

/**
 * Comments delete callback.
 */
function ajax_comments_instant_delete($cid = 0){
  $status = FALSE;

  // Check token to avoid CSRF attack.
  if ($cid && isset($_GET['token']) && drupal_valid_token($_GET['token'], $cid)) {
    drupal_set_header('Content-type: text/javascript; charset=utf-8');
    module_load_include('inc', 'comment', 'comment.admin');
    $comment = _comment_load($cid);
    // Delete comment and its replies.
    _comment_delete_thread($comment);
    _comment_update_node_statistics($comment->nid);
    // Clear the cache so an anonymous user sees that his comment was deleted.
    cache_clear_all();
    $status = TRUE;
  }
  $response = array(
    'status' => $status,
  );
  drupal_json($response);
}
