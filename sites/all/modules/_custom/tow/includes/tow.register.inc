<?php

/**
 * @file
 * 
 * Functions for registration form.
 */



/****************************************************************************************************
 * Constants
 ****************************************************************************************************/

define('TOW_MAX_ALLOWED_DATASET_KEYWORDS', 10);



/****************************************************************************************************
 * Hook handlers
 ****************************************************************************************************/

/**
 * Alter user registration form.
 */
function tow_register_form_alter(&$form, &$form_state) {
  
  // Get help text.
  $help = _tow_register_help();
  
  // Replace tokens with necessary data.
  $help = str_replace(
    array(
      '!datalink',
      '!feedlink',
      '!semantic_web_link',
      '!newslink',
      '!new_ideas_link',
      '!hear_from_you_link'
    ),
    array(
      l('Data', 'data'),
      l('Feed', 'feed'),
      l('Semantic web technology', 'http://en.wikipedia.org/wiki/Semantic_Web'),
      l('New', 'news'),
      l('working on new ideas', 'new_ideas'),
      l('hear from you', 'hear_from_you')
    ),
    $help
  );
  
  // Set help.
  $form['user_registration_help'][0]['#value'] = $help;
  
  // Account settings.
  $form['account']['email_verify'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail verify'),
    '#required' => TRUE,
    '#weight' => 2
  );
  $form['account']['pass']['#weight'] = 3;
  
  // Personal information.
  $form['Personal information']['#weight'] = -9;
  
  // Elements for dataset creation.
  $form['dataset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Data (you can update this field later)'),
    '#collapsible' => FALSE,
    '#tree' => TRUE
  );
  
  // Dataset.
  $form['dataset']['name_guide'] = array(
    '#type' => 'item',
    '#value' => t("Name your data. Give a short title that describes your data, lab, or project. This title should give users a basic idea of what they'll find in your data set. You can come back and modify this title later.")
  );  
  $form['dataset']['name'] = array(
    '#type' => 'textfield'
  );
  $form['dataset']['description_guide'] = array(
    '#type' => 'item',
    '#value' => t("Describe your data. In 250 words or less, describe what's in your dataset, how it was collected and used.")
  );
  $form['dataset']['description'] = array(
    '#type' => 'textarea'
  );
  $form['dataset']['category_guide'] = array(
    '#type' => 'item',
    '#value' => t('Please choose one subject area and enter keywords (separated by commas) associated with your data set.')
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
    '#value' => t('Share data with other users? Users will be able to access and search your data.')
  );
  $form['dataset']['access'] = array(
    '#type' => 'radios',
    '#options' => array(t('Open'), t('By request'), t('Private')),
    '#default_value' => 0
  );
  
  // Elements for feeds.
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
    '#value' => t("Feeds are setup by giving a topic, keywords and key-phrases associated with the topic. Enter up to 10 topics below, providing keywords and key-phrases for each one. Proper names are fine. For example, if you study depression in minority groups, your topic might be 'depression in ethnic minority populations', with keywords 'racial minority, ethnic minority, depression, mental health, health disparity'.")
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
  
  // Get feeds parameters from form state and save to form.
  if (is_array($form_state['values']['feeds']['items'])) {
    
    foreach ($form_state['values']['feeds']['items'] as $index => $item) {
      $form['feeds']['items'][$index] = _tow_register_feed_element($item);
    }
    if (count($form_state['values']['feeds']['items']) < 10) {
      $form['feeds']['items'][] = _tow_register_feed_element(array());
    }
    else {
      unset($form['feeds']['add_more']);
    }
  }
  else {
    for ($i = 0; $i < 3; $i++) {
      $form['feeds']['items'][] = _tow_register_feed_element(array());
    }
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

  // Terms of service.
  $form['terms_of_service'] = array(
    '#type' => 'checkbox',
    '#title' => t("Check here to agree on Theory on Wheels' " . l('terms of service', 'terms-of-service') . ' and ' . l('privacy statement', 'privacy')),
  );
  
  // Newsletter.
  $form['newsletter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Check here if you would like to receive our newsletter, sent out once a month.'),
  );
  
  // Validation callbacks.
  $form['#validate'][] = 'tow_register_dont_validate_cancel';
  $form['#validate'][] = 'tow_register_user_validate';

}



/****************************************************************************************************
 * Callback functions
 ****************************************************************************************************/

/**
 * When "cancel" button is clicked, redirect to front page without error messages.
 * When "add more" button is clicked, just flush error messages.
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
 * Validate user registration form.
 */
function tow_register_user_validate($form, &$form_state) {

  if ($form_state['clicked_button']['#value'] != t('Create new account')) {
    return;
  }
  
  if ($form_state['values']['email_verify'] != $form_state['values']['mail']) {
    $error = TRUE;
    $err_field_names[] = 'email_verify';
    $err_msgs[] = t('E-mail address and E-mail verify should be same');
  }
  
  if (!$form_state['values']['terms_of_service']) {
    $error = TRUE;
    $err_field_names[] = 'terms_of_service';
    $err_msgs[] = t('You should agree on terms of service and privacy terms');
  }
  
  _tow_show_validation_error($error, $err_field_names, $err_msgs);
}



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Process user registration submission.
 */
function tow_register(&$edit, $account) {

  if ($_POST['op'] != 'Create new account') {
    return;
  }
  
  // Create dataset when user is registered.
  if ($edit['dataset']['name']) {
    _tow_register_create_dataset($edit['dataset'], $account->uid);
  }
  
  // Store feeds in the db when user is registered.
  _tow_register_store_feeds($edit['feeds']['items'], $account->uid);
  
  foreach (array('email_verify', 'dataset', 'feeds', 'terms_of_service', 'newsletter', 'cancel') as $item_name) {
    unset($edit[$item_name]);
  }
}

/**
 * Returns array of taxonomy terms.
 * Keys are term IDs, values are term names, array is sorted alphabetically.
 */
function tow_register_taxonomy() {
  $res = db_query("SELECT tid, name FROM {term_data} WHERE vid = 2 ORDER BY name ASC");
  while ($term = db_fetch_array($res)) {
    $terms[$term['tid']] = $term['name'];
  }
  return $terms;
}



/****************************************************************************************************
 * Helper functions
 ****************************************************************************************************/

/**
 * Create dataset when user is registered.
 */
function _tow_register_create_dataset($data, $uid) {
  
  // Prepare node object.
  $dataset = new stdClass;
  $dataset->type = 'dataset';
  $dataset->uid = $uid;
  $dataset->title = $data['name'];
  $dataset->status = 1;
  $dataset->body = $data['description'];
  $dataset->field_access_type[0]['value'] = $data['access'];
  
  // Set taxonomy.
  if ($data['category']) {
    $dataset->taxonomy[0] = taxonomy_get_term($data['category']);
  }
  
  // Get terms.
  if ($data['keywords']) {
    $terms = explode(',', $data['keywords']);
    
    // Prevent mass terming.
    if (count($terms) > TOW_MAX_ALLOWED_DATASET_KEYWORDS) {
      array_slice($terms, 0, TOW_MAX_ALLOWED_DATASET_KEYWORDS);
    }
    
    // Save terms.
    foreach ($terms as $term_name) {
      $term = array(
        'vid' => 1,
        'name' => trim($term_name)
      );
      taxonomy_save_term($term);
    }
    
  }
  
  // Save dataset.
  node_save($dataset);  
}

/**
 * Returns form element with textfields for feed topic and keywords.
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
  
  if ($item['topic']) {
    $element['topic']['#default_value'] = $item['topic'];
  }
  
  if ($item['keywords']) {
    $element['keywords']['#default_value'] = $item['keywords'];
  }
  
  return $element;
}

/**
 * Returns help text.
 */
function _tow_register_help() {
  
  $help = '
    <h6>
      After you fill this short registration form, you\'ll get free access to two services.
    </h6>
    <ul class="services-help-information">
      <li class="data">
        !datalink
        <p>
          The Data service lets you upload your research data, search it, manage it, and share it with others. You get 10 tables free, with a maximum of 10,000 rows each. The tables can be linked together through indexes, making for more powerful searching. The search form adjusts itself based on the content of your data, making this an ideal tool for sharing your data with colleagues.
        </p>
      </li>
      <li>
        !feedlink
        <p>
          The Feed service scours the internet looking for news, blogging, and other content related to your research interests. You provide a few keywords related to your discipline, and Feed uses !semantic_web_link to collect new web content related to your interests. This steady stream of \'mentions\' helps keep track of how your discipline is relevant around the world, and outside of esoteric research journals.
        </p>
      </li>
      <li>
        !newslink
        <p>
          Theory on Wheels is always looking for new ways to make researchers lives easier, more fun, and more interactive. We\'re are !new_ideas_link and would like to !hear_from_you_link.
        </p>
      </li>
    </ul>
  ';
  
  return $help;
}

/**
 * Store feeds in the db when user is registered.
 */
function _tow_register_store_feeds($feeds, $uid) {
  foreach ($feeds as $feed) {
    $topic = $feed['topic'];
    $keywords = explode(',', $feeds['keywords']);
  }
}
