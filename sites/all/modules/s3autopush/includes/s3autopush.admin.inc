<?php

/**
 * Settings form for S3AutoPush module.
 */
function s3autopush_settings_form() {
  $form['s3autopush_bucket_url'] = array(
   '#type' => 'textfield',
   '#title' => t('Bucket URL'),
   '#required' => TRUE,
   '#description' => 'Format: http://{BUCKETNAME}.s3.amazonaws.com',
   '#default_value' => variable_get('s3autopush_bucket_url', ''),
  );

  $form['s3autopush_bucket'] = array(
   '#type' => 'textfield',
   '#title' => t('Bucket name'),
   '#required' => TRUE,
   '#default_value' => variable_get('s3autopush_bucket', ''),
  );
  
  $form['s3autopush_access_key'] = array(
   '#type' => 'textfield',
   '#title' => t('Access key'),
   '#required' => TRUE,
   '#default_value' => variable_get('s3autopush_access_key', ''),
  );
  
  $form['s3autopush_secret_key'] = array(
   '#type' => 'textfield',
   '#title' => t('Secret key'),
   '#required' => TRUE,
   '#default_value' => variable_get('s3autopush_secret_key', ''),
  );
  
  $form['s3autopush_mask'] = array(
    '#type' => 'textfield',
    '#title' => 'Files filter mask',
    '#required' => TRUE,
    '#default_value' => variable_get('s3autopush_mask', S3AUTOPUSH_MASK_DEFAULT),
  );

  return system_settings_form($form);
}

function s3autopush_settings_form_validate(&$form, &$form_state) {
  $v = &$form_state['values'];
}
