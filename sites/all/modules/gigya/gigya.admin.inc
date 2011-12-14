<?php
// $Id: gigya.admin.inc,v 1.7 2011/01/23 09:31:16 gambaweb Exp $

/**
 * @file
 * Provides the administration settings for the Gigya module.
 */

/**
 * Form builder for the Gigya administration form.
 *
 * @see gigya_admin_settings_form_submit()
 */
function gigya_admin_form() {
  global $user, $base_path;
  include_once 'modules/user/user.pages.inc';
  module_load_include('inc', 'gigya');
  $capabilities = gigya_get_capabilities();
  $form = array();

  gigya_validate();

  // Set up colors. If colorpicker module is enabled, we'll use that instead.
  $color_field_type = (function_exists('colorpicker_2_or_later') ? 'colorpicker_' : '') . 'textfield';
  $form['gigya_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Gigya debuging'),
    '#description' => t('Write errors to watchdog'),
    '#default_value'=> variable_get('gigya_debug', 0),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => -401,
  );

  $form['keys'] = array(
    '#type' => 'fieldset',
    '#title' => t('Gigya Socialize keys'),
    '#description' => t('Keys needed for connecting to the Gigya Socialize service.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => -400,
  );
  $form['keys']['gigya_APIKey'] = array(
    '#type' => 'textfield',
    '#title' => t('Gigya API Key'),
    '#description' => t('Specify the gigya API key for this domain'),
    '#default_value' => variable_get('gigya_APIKey', ''),
    '#required' => TRUE,
    '#weight' => -300,
  );
  $form['keys']['gigya_SecretKey'] = array(
    '#type' => 'textfield',
    '#title' => t('Gigya Secret Key'),
    '#description' => t('Specify the gigya Secret (Base64 encoded) key for this domain'),
    '#default_value' => variable_get('gigya_SecretKey', ''),
    '#required' => TRUE,
    '#weight' => -200,
  );

  $form['gigya_selected_networks'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Networks available to Gigya'),
    '#default_value' => variable_get('gigya_selected_networks', array()),
    '#options' => gigya_get_networks(),
    '#required' => TRUE,
    '#description' => t('Select the networks this site should be allowed to connect to via Gigya'),
    '#weight' => -100,
  );

  $form['gigya_requiredCapabilities'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Required services for Gigya'),
    '#default_value' => variable_get('gigya_requiredCapabilities', array('login' => 'login')),
    '#options' => $capabilities,
    '#description' => t('Choose which capabilities you wish to enable when users connect to social networks.'),
    '#weight' => -8,
  );

  /*
   * LoginUI
   */
  $form['login_uiconfig'] = array(
    '#type' => 'fieldset',
    '#title' => t('Parameters for Login UIConfig'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['login_uiconfig']['gigya_login_surpress_login'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display only Socialize component in the login block (suppress Drupal login)'),
    '#default_value' => variable_get('gigya_login_surpress_login', FALSE),
    '#description' => t('Checking this option will disable normal Drupal login. Admins can login via /user'),
  );

  $form['login_uiconfig']['gigya_login_hide_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide title of Gigya login block'),
    '#default_value' => variable_get('gigya_login_hide_title', FALSE),
  );

  $form['login_uiconfig']['gigya_login_form_position'] = array(
    '#type' => 'radios',
    '#title' => t('Position of Gigya login UI within login form'),
    '#default_value' => variable_get('gigya_login_form_position', 0),
    '#options' => array(t('Top'), t('Bottom')),
  );

  $form['login_uiconfig']['gigya_enable_linking'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable linking of social networks accounts'),
    '#default_value' => variable_get('gigya_enable_linking', TRUE),
    '#description' => t('Checking this option will enable users to login with their Social Network credentials and map those to the same Drupal user.'),
  );

  $form['login_uiconfig']['gigya_login_uiconfig_terms'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show terms link'),
    '#default_value' => variable_get('gigya_login_uiconfig_terms', FALSE),
    '#description' => t('Display the Gigya terms of service link'),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Login UI Title'),
    '#default_value' => variable_get('gigya_login_uiconfig_title', ''),
    '#description' => t('Leave blank for no title. Note: An override for this is also available in the block configuration screen.'),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_width'] = array(
    '#type' => 'textfield',
    '#title' => t('UI Box Width'),
    '#default_value' => variable_get('gigya_login_uiconfig_width', GIGYA_DEFAULT_LOGINUI_WIDTH),
    '#description' => t('Select width for Login Box'),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_height'] = array(
    '#type' => 'textfield',
    '#title' => t('UI Box Height'),
    '#default_value' => variable_get('gigya_login_uiconfig_height', GIGYA_DEFAULT_LOGINUI_HEIGHT),
    '#description' => t('Select height for Login Box'),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_iconsize'] = array(
    '#type' => 'textfield',
    '#title' => t('Icon Size'),
    '#default_value' => variable_get('gigya_login_uiconfig_iconsize', GIGYA_DEFAULT_ICONSIZE),
    '#description' => t('How big should the social network icons be'),
  );

  $form['login_uiconfig']['gigya_login_uiconfig_color_background'] = array(
    '#type' => $color_field_type,
    '#title' => t('Background'),
    '#default_value' => variable_get('gigya_login_uiconfig_color_background', GIGYA_DEFAULT_LOGINUI_COLOR_BACKGROUND),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_color_background_transparent'] = array(
    '#type' => 'checkbox',
    '#title' => t('Transparent Background'),
    '#default_value' => variable_get('gigya_login_uiconfig_color_background_transparent', GIGYA_DEFAULT_LOGINUI_COLOR_BACKGROUND_TRANSPARENT),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_color_frame'] = array(
    '#type' => $color_field_type,
    '#title' => t('Frame'),
    '#default_value' => variable_get('gigya_login_uiconfig_color_frame', GIGYA_DEFAULT_LOGINUI_COLOR_FRAME),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_color_text'] = array(
    '#type' => $color_field_type,
    '#title' => t('Text'),
    '#default_value' => variable_get('gigya_login_uiconfig_color_text', GIGYA_DEFAULT_LOGINUI_COLOR_TEXT),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_color_link'] = array(
    '#type' => $color_field_type,
    '#title' => t('Link'),
    '#default_value' => variable_get('gigya_login_uiconfig_color_link', GIGYA_DEFAULT_LOGINUI_COLOR_LINK),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_color_navigation_arrow'] = array(
    '#type' => $color_field_type,
    '#title' => t('Navigation Arrow'),
    '#default_value' => variable_get('gigya_login_uiconfig_color_navigation_arrow', GIGYA_DEFAULT_LOGINUI_COLOR_NAVIGATION_ARROW),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_color_caption_background'] = array(
    '#type' => $color_field_type,
    '#title' => t('Caption Background'),
    '#default_value' => variable_get('gigya_login_uiconfig_color_caption_background', GIGYA_DEFAULT_LOGINUI_COLOR_CAPTION_BACKGROUND),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_color_caption_text'] = array(
    '#type' => $color_field_type,
    '#title' => t('Caption Text'),
    '#default_value' => variable_get('gigya_login_uiconfig_color_caption_text', GIGYA_DEFAULT_LOGINUI_COLOR_CAPTION_TEXT),
  );
  $form['login_uiconfig']['gigya_login_uiconfig_color_button_text'] = array(
    '#type' => $color_field_type,
    '#title' => t('Button Text'),
    '#default_value' => variable_get('gigya_login_uiconfig_color_button_text', GIGYA_DEFAULT_LOGINUI_COLOR_BUTTON_TEXT),
  );

  $form['login_uiconfig']['login_advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced LoginUI Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 2,
  );
  $form['login_uiconfig']['login_advanced']['gigya_login_uiconfig_urlredirect'] = array(
    '#type' => 'textfield',
    '#title' => t('URL redirect after login'),
    '#default_value' => variable_get('gigya_login_uiconfig_urlredirect', 'user'),
    '#description' => t('Choose an alternate URL to direct users to after login. Defaults to user. Do not use a beginning or ending slash!'),
  );
  $form['login_uiconfig']['login_advanced']['gigya_login_advanced_custombuttons'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Login Icons URL'),
    '#default_value' => variable_get('gigya_login_advanced_custombuttons', ''),
    '#description' => t('Choose an alternate URL to pull in flash button data'),
  );
  $form['login_uiconfig']['login_advanced']['gigya_login_uiconfig_containerID'] = array(
    '#type' => 'textfield',
    '#title' => t('LoginUI container ID'),
    '#default_value' => variable_get('gigya_login_uiconfig_containerID', GIGYA_DEFAULT_LOGINUI_CONTAINERID),
    '#description' => t('div ID for loginUI'),
    '#required' => TRUE,
  );
  $form['login_uiconfig']['login_advanced']['gigya_login_uiconfig_useHTML'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Flash widget instead of HTML widget'),
    '#default_value' => !variable_get('gigya_login_uiconfig_useHTML', GIGYA_DEFAULT_LOGINUI_USEHTML)
  );
  $form['login_uiconfig']['login_advanced']['gigya_login_advanced_raw'] = array(
    '#type' => 'textarea',
    '#title' => t('Login UIConfig Advanced coding (Overwrites above settings)'),
    '#default_value' => variable_get('gigya_login_advanced_raw', ''),
    '#description' => t('Use this textbox to put in your own settings. If you are not using this box, make sure it is empty!'),
  );

  /*
   * ConnectUI
   */
  $form['connect_uiconfig'] = array(
    '#type' => 'fieldset',
    '#title' => t('Parameters for Connect UIConfig'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_captionText'] = array(
    '#type' => 'textfield',
    '#title' => t('ConnectUI Caption'),
    '#default_value' => variable_get('gigya_connect_uiconfig_captionText', GIGYA_DEFAULT_CONNECTUI_CAPTIONTEXT),
    '#description' => t('Leave blank for no title.'),
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_terms'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show terms link'),
    '#default_value' => variable_get('gigya_connect_uiconfig_terms', FALSE),
    '#description' => t('Display the Gigya terms of service link'),
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_width'] = array(
    '#type' => 'textfield',
    '#title' => t('UI Box Width'),
    '#default_value' => variable_get('gigya_connect_uiconfig_width', GIGYA_DEFAULT_CONNECTUI_WIDTH),
    '#description' => t('Select width for Login Box'),
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_height'] = array(
    '#type' => 'textfield',
    '#title' => t('UI Box Height'),
    '#default_value' => variable_get('gigya_connect_uiconfig_height', GIGYA_DEFAULT_CONNECTUI_HEIGHT),
    '#description' => t('Select height for Login Box'),
  );

  $form['connect_uiconfig']['gigya_connect_uiconfig_color_background'] = array(
    '#type' => $color_field_type,
    '#title' => t('Background'),
    '#default_value' => variable_get('gigya_connect_uiconfig_color_background', GIGYA_DEFAULT_CONNECTUI_COLOR_BACKGROUND),
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_color_background_transparent'] = array(
    '#type' => 'checkbox',
    '#title' => t('Transparent Background'),
    '#default_value' => variable_get('gigya_connect_uiconfig_color_background_transparent', GIGYA_DEFAULT_CONNECTUI_COLOR_BACKGROUND_TRANSPARENT),
  );

  if (variable_get('gigya_connect_uiconfig_color_background_transparent', GIGYA_DEFAULT_CONNECTUI_COLOR_BACKGROUND_TRANSPARENT)) {
    variable_set('gigya_connect_uiconfig_color_background', '');
  }

  $form['connect_uiconfig']['gigya_connect_uiconfig_color_frame'] = array(
    '#type' => $color_field_type,
    '#title' => t('Frame'),
    '#default_value' => variable_get('gigya_connect_uiconfig_color_frame', GIGYA_DEFAULT_CONNECTUI_COLOR_FRAME),
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_color_text'] = array(
    '#type' => $color_field_type,
    '#title' => t('Text'),
    '#default_value' => variable_get('gigya_connect_uiconfig_color_text', GIGYA_DEFAULT_CONNECTUI_COLOR_TEXT),
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_color_link'] = array(
    '#type' => $color_field_type,
    '#title' => t('Link'),
    '#default_value' => variable_get('gigya_connect_uiconfig_color_link', GIGYA_DEFAULT_CONNECTUI_COLOR_LINK),
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_color_navigation_arrow'] = array(
    '#type' => $color_field_type,
    '#title' => t('Navigation Arrow'),
    '#default_value' => variable_get('gigya_connect_uiconfig_color_navigation_arrow', GIGYA_DEFAULT_CONNECTUI_COLOR_NAVIGATION_ARROW),
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_color_caption_background'] = array(
    '#type' => $color_field_type,
    '#title' => t('Caption Background'),
    '#default_value' => variable_get('gigya_connect_uiconfig_color_caption_background', GIGYA_DEFAULT_CONNECTUI_COLOR_CAPTION_BACKGROUND),
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_color_caption_text'] = array(
    '#type' => $color_field_type,
    '#title' => t('Caption Text'),
    '#default_value' => variable_get('gigya_connect_uiconfig_color_caption_text', GIGYA_DEFAULT_CONNECTUI_COLOR_CAPTION_TEXT),
  );
  $form['connect_uiconfig']['gigya_connect_uiconfig_color_button_text'] = array(
    '#type' => $color_field_type,
    '#title' => t('Button Text'),
    '#default_value' => variable_get('gigya_connect_uiconfig_color_button_text', GIGYA_DEFAULT_CONNECTUI_COLOR_BUTTON_TEXT),
  );
  $form['connect_uiconfig']['connect_advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced ConnectUI Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 2,
  );
  $form['connect_uiconfig']['connect_advanced']['gigya_connect_uiconfig_containerID'] = array(
    '#type' => 'textfield',
    '#title' => t('connectUI container ID:'),
    '#default_value' => variable_get('gigya_connect_uiconfig_containerID', GIGYA_DEFAULT_CONNECTUI_CONTAINERID),
    '#description' => t('div ID for connectUI'),
  );
  $form['connect_uiconfig']['connect_advanced']['gigya_connect_uiconfig_useHTML'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Flash widget instead of HTML widget'),
    '#default_value' => !variable_get('gigya_connect_uiconfig_useHTML', GIGYA_DEFAULT_CONNECTUI_USEHTML)
  );
  $form['connect_uiconfig']['connect_advanced']['gigya_connect_uiconfig_raw'] = array(
    '#type' => 'textarea',
    '#title' => t('ConnectUI Advanced coding (Overwrites above settings)'),
    '#default_value' => variable_get('gigya_connect_uiconfig_raw', ''),
    '#description' => t('Use this textbox to put in your own settings. If you are not using this box, make sure it is empty!'),
  );

  /*
   * Profile and Content Profile field mapping.
   */
  $form['bio_mapping'] = array(
    '#type' => 'fieldset',
    '#description' => t('You may use replacement patterns (tokens) to map data retrieved from Social Networks into your Drupal profile fields.'),
    '#title' => t('Profile Mappings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  if (!module_exists('token')) {
    $form['bio_mapping']['token_required'] = array(
      '#type' => 'item',
      '#value' => t('Please download and install the Token module to enable this functionality.')
    );
  }
  else{
    $form['bio_mapping']['token_help'] = array(
      '#title' => t('Replacement patterns'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['bio_mapping']['token_help']['help'] = array(
      '#value' => theme('token_help', array('gigya')),
    );

    // Base Drupal fields.
    $form['bio_mapping']['gigya_map_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Drupal username'),
      '#default_value' => variable_get('gigya_map_name', '[gigya-nickname]')
    );
    $form['bio_mapping']['gigya_map_mail'] = array(
      '#type' => 'textfield',
      '#title' => t('Email address'),
      '#default_value' => variable_get('gigya_map_mail', '[gigya-email]')
    );


    // Build a list of core Profile registration form fields.
    if (module_exists('profile')) {
      $profile_registration_fields = _profile_get_fields(NULL, TRUE);
      while($profile_registration_field = db_fetch_object($profile_registration_fields)) {
        $form['bio_mapping']['gigya_map_' . $profile_registration_field->name] = array(
          '#type' => 'textfield',
          '#title' => $profile_registration_field->title,
          '#default_value' => variable_get('gigya_map_'. $profile_registration_field->name, ''),
        );
      }
    }

    if (module_exists('content_profile')) {
      // Build a list of Content Profile registration form fields.
      if ($cp_registration_types = content_profile_get_types('names','registration_use', 1)) {
        foreach ($cp_registration_types as $cp_type_key => $cp_type_value) {
          if ($cp_type = content_types($cp_type_key)) {
            $cp_type_form_fields = array();
            foreach ($cp_type['fields'] as $cp_field) {
              $cp_type_form_fields['gigya_map_' . $cp_field['field_name']] = array(
                '#type' => 'textfield',
                '#title' => $cp_field['widget']['label'],
                '#default_value' => variable_get('gigya_map_' . $cp_field['field_name'], ''),
              );
            }
            if (count($cp_type_form_fields) > 0) {
              $form['bio_mapping']['content_profile_registration_' . $cp_type_key] = array(
                '#title' => t('Content Profile registration fields for type: @content_type', array('@content_type' => $cp_type_value)),
                '#type' => 'fieldset',
                '#collapsible' => TRUE,
                '#collapsed' => TRUE,
              );
              $form['bio_mapping']['content_profile_registration_' . $cp_type_key] += $cp_type_form_fields;
            }
          }
        }
      }
    }
  }

  /* Not ready for prime time.
  $non_reg_fields = array();

  // Build a list of core Profile non-registration fields.
  foreach (profile_categories() as $profile_category) {
    $profile_fields = _profile_get_fields($profile_category['name'], FALSE);

    while ($profile_field = db_fetch_object($profile_fields)) {
      $non_reg_fields['gigya_map_' . $profile_field->name] = array(
        '#type' => 'textfield',
        '#title' => $profile_field->title,
        '#default_value' => variable_get('gigya_map_' . $profile_field->name, ''),
      );
    }
  }

  // Build a list of Content Profile non-registration fields.
  if ($cp_types = content_profile_get_types('names', 'registration_use', 0)) {
    foreach ($cp_types as $cp_type_key => $cp_type_value) {
      if ($cp_type = content_types($cp_type_key)) {
        $cp_type_form_fields = array();
        foreach ($cp_type['fields'] as $cp_field) {
          $cp_type_form_fields['gigya_map_' . $cp_field['field_name']] = array(
            '#type' => 'textfield',
            '#title' => $cp_field['widget']['label'],
            '#default_value' => variable_get('gigya_map_' . $cp_field['field_name'], ''),
          );
        }
        if (count($cp_type_form_fields) > 0) {
          $non_reg_fields['content_profile_' . $cp_type_key] = array(
            '#title' => t('Content Profile fields for type: @content_type', array('@content_type' => $cp_type_value)),
            '#type' => 'fieldset',
            '#collapsible' => TRUE,
            '#collapsed' => TRUE,
          );
          $non_reg_fields['content_profile_'. $cp_type_key] += $cp_type_form_fields;
        }
      }
    }
  }

  // Only display the non-registration fieldgroup if there are
  // non-registration fields in existence.
  if (count($non_reg_fields)) {
    $form['bio_mapping']['non_reg_fields'] = array(
      '#title' => t('Profile fields not displayed on the registration form'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['bio_mapping']['non_reg_fields'] += $non_reg_fields;
  }
  */

  $form['#submit'][0] = 'gigya_admin_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Form submission handler for gigya_admin_settings_form().
 *
 * @see gigya_admin_settings_form()
 */
function gigya_admin_settings_form_submit($form, &$form_state) {
  $form_state['values']['gigya_login_uiconfig_useHTML'] = !$form_state['values']['gigya_login_uiconfig_useHTML'];
  $form_state['values']['gigya_connect_uiconfig_useHTML'] = !$form_state['values']['gigya_connect_uiconfig_useHTML'];
  variable_set(GIGYA_ENABLE_FACEBOOK_POPULATION, $form_state['values'][GIGYA_ENABLE_FACEBOOK_POPULATION]);
  if (!empty($form_state['values'][GIGYA_POPULATE_NON_REG_PROFILE_FIELDS])) {
    variable_set(GIGYA_POPULATE_NON_REG_PROFILE_FIELDS, $form_state['values'][GIGYA_POPULATE_NON_REG_PROFILE_FIELDS]);
  }
}
