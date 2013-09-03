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
function tow_mongodb_admin_settings_form() {

    $form = array();

    $form['tow_mongodb_connection_url'] = array(
        '#type' => 'textfield',
        '#title' => t('ToW MongoDB connection URL'),
        '#size' => 256,
        '#description' => t('Connection settings to the MongoDB instance.'),
        '#default_value' => variable_get('tow_mongodb_connection_url', 'mongodb://localhost:27017'),
    );

    $form['tow_mongodb_user'] = array(
        '#type' => 'textfield',
        '#title' => t('ToW MongoDB user'),
        '#size' => 256,
        '#description' => t('ToW MongoDB user. Leave empty for no user.'),
        '#default_value' => variable_get('tow_mongodb_user', ''),
    );

    $form['tow_mongodb_password'] = array(
        '#type' => 'password',
        '#title' => t('ToW MongoDB password'),
        '#size' => 256,
        '#description' => t('ToW MongoDB user password. Leave empty for no password.'),
        '#default_value' => variable_get('tow_mongodb_password', ''),
    );

    $form['tow_mongodb_db'] = array(
        '#type' => 'textfield',
        '#title' => t('ToW MongoDB database'),
        '#size' => 256,
        '#description' => t('ToW MongoDB database for storing data.'),
        '#default_value' => variable_get('tow_mongodb_db', 'admin'),
    );

    $form['tow_mongodb_cron_limit'] = array(
        '#type' => 'textfield',
        '#title' => t('ToW MongoDB cron limit'),
        '#size' => 256,
        '#description' => t('Max number of nodes being indexed per cron run.'),
        '#default_value' => variable_get('tow_mongodb_cron_limit', 100),
    );

    return system_settings_form($form);
}

/**
 * Gets information about the fields already in solr index.
 */
function tow_mongodb_admin_index_page() {

    $status = tow_mongodb_index_status();
    $output = "<p>" . t('Number of documents in index: @num', array('@num' => $status['indexed'])) . "</p>\n";

    // Display the Delete Index form.
    $output .= drupal_get_form('tow_mongodb_admin_delete_index_form');

    return $output;
}

/**
 * Create a form for deleting the contents of the Solr index.
 */
function tow_mongodb_admin_delete_index_form() {
    $form = array();
    $form['markup'] = array(
        '#prefix' => '<h3>',
        '#value' => t('Index controls'),
        '#suffix' => '</h3>',
    );
    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Delete the index'),
        '#submit' => array('tow_mongodb_admin_delete_index_submit'),
    );
    $form['delete-desc'] = array(
        '#type' => 'item',
        '#description' => t('Deletes all of the documents in the MongoDB index.'),
    );

    return $form;
}

/**
 * Submit function for the "Delete the index" button.
 */
function tow_mongodb_admin_delete_index_submit($form, &$form_state) {
    $form_state['redirect'] = 'admin/settings/tow/mongodb/index/delete/confirm';
}

/**
 * Confirmation form for "Delete the index" button.
 */
function tow_mongodb_admin_delete_index_confirm() {
    $form = array();
    return confirm_form($form, t('Are you sure you want to delete your search index and start re-indexing?'), 'admin/settings/tow/mongodb/index', NULL, t('Delete'), t('Cancel'));
}

/**
 * Submit function for the "Delete the index" confirmation form.
 */
function tow_mongodb_admin_delete_index_confirm_submit($form, &$form_state) {
    $form_state['redirect'] = 'admin/settings/tow/mongodb/index';
    try {
        tow_mongodb_index_delete_index();
    } catch (Exception $e) {
        watchdog('ToW MongoDB', nl2br(check_plain($e->getMessage())), NULL, WATCHDOG_ERROR);
    }
}