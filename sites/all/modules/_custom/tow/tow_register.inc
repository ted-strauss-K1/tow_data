<?php
// $Id: tow_register.inc,v 1.0.0.0 2011/02/28 23:42:15 tow Exp $

/**
 * @file
 * Functions for registration form
 */

/**
 * Alter user registration form
 */
function tow_register_form_alter(&$form, &$form_state) {
  
  $help = file_get_contents(drupal_get_path('module', 'tow') . '/templates/registration_help.html');
  $help = str_replace('!datalink', l('Data', 'data'), $help);
  $help = str_replace('!feedlink', l('Feed', 'feed'), $help);
  $help = str_replace('!newslink', l('New', 'news'), $help);
  $form['user_registration_help'][0]['#value'] = $help;
  
  
  $form['name']['#title'] = t('First name');
  $form['name']['#weight'] = -4;
  
  $form['last_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Last name'),
    '#required' => TRUE,
    '#weight' => -3
  );
  
  $form['mail']['#weight'] = -2;
  
  $form['email_verify'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail verify'),
    '#required' => TRUE,
    '#weight' => -1
  );
  
  $form['pass']['#weight'] = 0;
    
  $form['affiliation'] = array(
    '#type' => 'textfield',
    '#title' => t('Affiliation'),
  );
  
  // Elements for dataset creation
  $form['dataset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Data (you can update this field later)'),
    '#collapsible' => FALSE,
    '#tree' => TRUE
  );
  
  $form['dataset']['name_guide'] = array(
    '#type' => 'item',
    '#value' => 'Name your data. Give a short title that describes your data, lab or project'
  );  
  $form['dataset']['name'] = array(
    '#type' => 'textfield'
  );
  
  $form['dataset']['description_guide'] = array(
    '#type' => 'item',
    '#value' => 'Describe your data. In 250 words or less..'
  );
  $form['dataset']['description'] = array(
    '#type' => 'textarea'
  );
  
  $form['dataset']['category_guide'] = array(
    '#type' => 'item',
    '#value' => 'Please choose..'
  );  
  $form['dataset']['category'] = array(
    '#type' => 'select',
    '#options' => tow_register_taxonomy()
  );  
  $form['dataset']['keywords'] = array(
    '#type' => 'textfield'
  );
  
  $form['dataset']['access_guide'] = array(
    '#type' => 'item',
    '#value' => 'Share data with other users? Users will be able to access and search your data'
  );
  $form['dataset']['access'] = array(
    '#type' => 'radios',
    '#options' => array(t('Open'), t('By request'), t('Private')),
    '#default_value' => 0
  );
  
  // Elements for feeds
  $form['feeds'] = array(
    '#type' => 'fieldset',
    '#title' => t('Feeds (you can update this field later)'),
    '#collapsible' => FALSE,
    '#tree' => TRUE,
    '#prefix' => '<div class="clear-block" id="feed-wrapper">',
    '#suffix' => '</div>'
  );
  
  $form['feeds']['guide'] = array(
    '#type' => 'item',
    '#value' => t('Feeds are set up by giving topic')
  );
  
  $form['feeds']['header_topic'] = array(
    '#type' => 'item',
    '#value' => t('Topic')
  );
  $form['feeds']['header_keywords'] = array(
    '#type' => 'item',
    '#value' => t('Keywords and key-phrases (comma-separated)')
  );
    
  $form['feeds']['items'] = array(
    '#prefix' => '<div id="feed-items">',
    '#suffix' => '</div>',
  );
  
  if (is_array($form_state['values']['feeds']['items'])) {
    
    foreach($form_state['values']['feeds']['items'] as $index => $item) {
      $form['feeds']['items'][$index] = _tow_register_feed_element($item);
    }
    if (count($form_state['values']['feeds']['items']) < 10)
      $form['feeds']['items'][] = _tow_register_feed_element(array());
    else
      unset($form['feeds']['add_more']);
    
  }else{
    for($i = 0; $i < 3; $i++)
      $form['feeds']['items'][] = _tow_register_feed_element(array());      
  }

  $form['feeds']['add_more'] = array(
    '#type' => 'submit',
    '#value' => t('Add more'),
    '#ahah' => array(
      'path' => 'add-more-feeds/js',
      'wrapper' => 'feed-items',
      'effect' => 'fade',
    ),
    '#weight' => 11
  );

  
  $form['terms_of_service'] = array(
    '#type' => 'checkbox',
    '#title' => t("Check here to agree on Theory on Wheels' " . l('terms of service', 'terms-of-service') . ' and ' . l('privacy', 'privacy')),
  );
  
  $form['newsletter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Check here if you would like to recieve newsletter our newsletter, sent out twice a month.'),
  );
  
  $form['#validate'][] = 'tow_register_dont_validate_cancel';
  $form['#validate'][] = 'tow_user_register_validate';

}

/**
 * @param $item array with default values
 *   'topic' => feed topic,
 *   'keywords' => feed keywords
 * @return form element with textfields for feed topic and keywords 
 */
function _tow_register_feed_element($item) {
  $element = array(
    '#tree' => TRUE,
    'topic' => array(
      '#type' => 'textfield',
      '#size' => 20
    ),
    'keywords' => array(
      '#type' => 'textfield'
    ),
  );
  
  if ($item['topic'])
    $element['topic']['#default_value'] = $item['topic'];
    
  if ($item['keywords'])
    $element['keywords']['#default_value'] = $item['keywords'];
    
  return $element;
}

/**
 * @return array of taconomy terms
 *   keys are term ids, values are term names, array is sorted alphabetically
 */
function tow_register_taxonomy() {
  $res = db_query("SELECT tid, name FROM {term_data} WHERE vid = 1 ORDER BY name ASC");
  while ($term = db_fetch_array($res))
    $terms[$term['tid']] = $term['name'];
  return $terms;
}

/**
 * Process user registration submissin
 * @param $edit array of values submitted by user into registration form
 * @param $account object of user that has just been created
 */
function tow_register(&$edit, $account) {

  if ($_POST['op'] != 'Create new account')
    return;
  
  if ($edit['dataset']['name'])
    _tow_register_create_dataset($edit['dataset'], $account->uid);
  
  _tow_register_store_feeds($edit['feeds']['items'], $account->uid);

  _tow_register_save_profile($edit['last_name'], $edit['affiliation'], $account->uid);
  
  foreach (array('email_verify', 'last_name', 'affiliation', 'dataset', 'feeds', 'terms_of_service', 'newsletter') as $item_name)
    unset($edit[$item_name]);
}

/**
 * Create dataset when user is registered
 * @param $data array of dataset fields submitted by user into registration form
 * @param $uid uid of user that has just been created
 */
function _tow_register_create_dataset($data, $uid) {
  $dataset = new stdClass;
  $dataset->type = 'dataset';
  $dataset->uid = $uid;
  $dataset->title = $data['name'];
  $dataset->status = 1;
  $dataset->body = $data['description'];
  $dataset->field_access_type[0]['value'] = $data['access'];
  
  if ($data['category'])
    $dataset->taxonomy[0] = taxonomy_get_term($data['category']);
  
  node_save($dataset);  

}

/**
 * Store feeds in the db when user is registered
 * @param $feeds array of feeds fields submitted by user into registration form
 * @param $uid uid of user that has just been created
 */
function _tow_register_store_feeds($feeds, $uid) {
  foreach($feeds as $feed) {
    $topic = $feed['topic'];
    $keywords = explode(',', $feeds['keywords']);
    //TODO: process feeds
  }
}

/**
 * When user registers, store his last name and affiliation in the db
 */
function _tow_register_save_profile($last_name, $affiliation, $uid) {
  //TODO process it when profile enabled and configured 
  //db_query("INSERT INTO {profile_values} (1, %d, '%s'), (2, %d, '%s')", $uid, $last_name, $uid, $affiliation);
}

/**
 * When "cancel" button is clicked, redirect to front page without error messages.
 * When "add more" button is clicked, just flush error messages
 */
function tow_register_dont_validate_cancel($form, $form_state) {
  if ($form_state['clicked_button']['#value'] == 'Cancel') {
    unset($_SESSION['messages']['error']);
    drupal_goto('<front>');
  }
  if ($form_state['clicked_button']['#value'] == 'Add more') {
    unset($_SESSION['messages']['error']);
  }
}

/**
 * Callback function
 * When user clicks 'add more' return rebuilted form.
 * This is just interface, rebuilding goes in tow_register_form_alter
 */
function tow_register_add_more_js() {
  
  $form_state = array('storage' => NULL, /*'rebuild' => TRUE, */'submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);
  
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  
  drupal_process_form($form_id, $form, $form_state);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
   
  $items = $form['feeds']['items'];
  $output = drupal_render($items);
  
  drupal_json(array('status' => TRUE, 'data' => $output));
}