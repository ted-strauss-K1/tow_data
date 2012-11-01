<?php

/**
 * @file
 * Administrative page callbacks for the httprl module.
 */

/**
 * Form definition; general settings.
 */
function httprl_admin_settings_form() {
  $form['httprl_server_addr'] = array(
    '#type'           => 'textfield',
    '#title'          => t('IP Address to send all self server requests to'),
    '#default_value'  => variable_get('httprl_server_addr', FALSE),
    '#description'    => t('If left blank it will use the same server as the request. If set to -1 it will use the host name instead of an IP address. This controls the output of httprl_build_url_self()'),
  );

  return system_settings_form($form);
}

/**
 * Validate form values.
 */
function httprl_admin_settings_form_validate($form, &$form_state) {
  $advagg_server_addr = $form_state['values']['httprl_server_addr'];

  // If the IP field is not blank, check that its a valid address.
  if (   !empty($advagg_server_addr)
      && $advagg_server_addr != -1
      && ip2long($advagg_server_addr) === FALSE
        ) {
    form_set_error('httprl_server_addr', t('Must be a valid IP address.'));
  }
}
