<?php

/**
 * Manual-pushing callback.
 */
function s3autopush_sync_callback($dir = '') {
  return drupal_get_form('s3autopush_sync_modules_batch_form');
}

function s3autopush_sync_modules_batch($packages = array()) {
  if ($packages['misc']) {
    $dirs[] = 'misc';
  }

  if (!empty($packages['s3autopush-custom'])) {
    $name   = $packages['s3autopush-custom'];
    $dirs[] = drupal_get_path('module', $name);
  }

  if ($packages['enabled-themes']) {
    foreach (_system_theme_data() as $name => $info) {
      $dirs[] = drupal_get_path('theme', $name);
    }
  }

  if ($packages['files']) {
    $dirs[] = file_directory_path();
  }

  if ($packages['libraries']) {
    $dirs[] = 'sites/all/libraries';
  }

  if ($packages['enabled-modules']) {
    foreach (module_list() as $name) {
      $dirs[] = drupal_get_path('module', $name);
    }
  }
  
  if (!empty($dirs)) {
    foreach ($dirs as $dir) {
      $files = file_scan_directory($dir, variable_get('s3autopush_mask', S3AUTOPUSH_MASK_DEFAULT));
      foreach (array_keys($files) as $path) {
        $operations[] = array(
          '__s3autopush_batch_process', array($path)
        );
      }
    }
  }

  $batch = array(
    'operations' => $operations, 
    'finished'   => '__s3autopush_batch_finished',
  );

  return $batch;
}

function s3autopush_sync_modules_batch_form() {
  $form['packages'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Select packages to push their static files to S3',
    '#options' => array(
      'enabled-modules' => 'Enabled modules',
      'enabled-themes' => 'Enabled themes',
      'misc' => 'Drupal misc',
      'files' => 'Site uploaded-files',
      'libraries' => 'Site libraries',
    ),
  );

  $form['custom'] = array(
    '#type' => 'textfield',
    '#title' => 'Module',
    '#description' => 'Enter custom module name you would like push its static files to S3.',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Go', 
  );

  return $form;
}

function s3autopush_sync_modules_batch_form_submit(&$form, &$form_state) {
  if (!empty($form_state['values']['custom'])) {
    $form_state['values']['packages']['s3autopush-custom'] = $form_state['values']['custom'];
  }
  $batch = s3autopush_sync_modules_batch($form_state['values']['packages']);
  batch_set($batch);
  $form_state['redirect'] = 's3autopush-sync';
}
