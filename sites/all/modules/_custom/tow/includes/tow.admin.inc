<?php

/**
 * @file
 *
 * Administration interface.
 */
/* * **************************************************************************************************
 * Forms
 * ************************************************************************************************** */

/**
 * Admin settings forms.
 */
function tow_admin_settings_form() {

    $form = array();

    $form['tow_max_tables_in_dataset'] = array(
        '#type' => 'textfield',
        '#title' => t('Max tables'),
        '#description' => t('Define the maximum amount of tables for each dataset.'),
        '#default_value' => variable_get('tow_max_tables_in_dataset', 12),
    );

    $form['tow_max_records_in_table'] = array(
        '#type' => 'textfield',
        '#title' => t('Max records'),
        '#description' => t('Define the maximum amount of records for each table.'),
        '#default_value' => variable_get('tow_max_records_in_table', 920),
    );

    $form['tow_min_fields_in_table'] = array(
        '#type' => 'textfield',
        '#title' => t('Min fields'),
        '#description' => t('Define the minimum amount of fields for each table.'),
        '#default_value' => variable_get('tow_min_fields_in_table', 1),
    );

    $form['tow_max_fields_in_table'] = array(
        '#type' => 'textfield',
        '#title' => t('Max fields'),
        '#description' => t('Define the maximum amount of fields for each table.'),
        '#default_value' => variable_get('tow_max_fields_in_table', 20),
    );

    $form['tow_cron_hash_sleep'] = array(
        '#type' => 'textfield',
        '#title' => t('Cron hash sleep'),
        '#description' => t('Define the pause length while updating datset hashes during the cron run.'),
        '#default_value' => variable_get('tow_cron_hash_sleep', 0),
    );

    $form['tow_export_file_lifetime'] = array(
        '#type' => 'textfield',
        '#title' => t('Export file lifetime'),
        '#description' => t('Define an existence time of an export file.'),
        '#default_value' => variable_get('tow_export_file_lifetime', 10),
    );

    $form['tow_max_allowed_dataset_keywords'] = array(
        '#type' => 'textfield',
        '#title' => t('Max allowed keywords'),
        '#description' => t('Define the maximum amount of keywords for each dataset.'),
        '#default_value' => variable_get('tow_max_allowed_dataset_keywords', 10),
    );

    $form['tow_admin_cron'] = array(
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#title' => t('Admin cron settings'),
    );

    $form['tow_admin_cron']['tow_admin_cron_period'] = array(
        '#type' => 'textfield',
        '#title' => t('Admin cron period'),
        '#description' => t('Define the period of time for running admin cron tasks (in seconds).'),
        '#default_value' => variable_get('tow_admin_cron_period', 24 * 60 * 60),
    );

    $form['tow_admin_cron']['tow_admin_cron_enabled'] = array(
        '#type' => 'checkbox',
        '#title' => t('Check this if you want to run ToW admin cron tasks.'),
        '#default_value' => variable_get('tow_admin_cron_enabled', 1),
        '#return_value' => 1,
    );

    return system_settings_form($form);
}

/**
 * Admin cron tasks.
 */
function tow_admin_cron() {
    $now = time();
    if (variable_get('tow_admin_cron_enabled', 1) && ($now - variable_get('tow_admin_cron_last', 0) >= variable_get('tow_admin_cron_period', 24 * 60 * 60))) {
        $query = "TRUNCATE TABLE {watchdog} RESTART IDENTITY CASCADE";
        db_query($query);
        variable_set('tow_admin_cron_last', $now);
    }
}