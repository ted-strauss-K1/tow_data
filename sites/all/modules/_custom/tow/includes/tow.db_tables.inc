<?php

/**
 * @file
 *
 * Functions returning arrays with db tables descriptions for Theory on Wheels.
 */
/* * **************************************************************************************************
 * Tables
 * ************************************************************************************************** */

/**
 * 'ToW Access Requests' table.
 */
function tow_db_tables_tow_access_requests() {
    return array(
        'fields' => array(
            'nid' => array(
                'type' => 'int',
                'size' => 'normal',
                'not null' => TRUE,
            ),
            'uid' => array(
                'type' => 'int',
                'size' => 'normal',
                'not null' => TRUE,
            ),
            'status' => array(
                'type' => 'int',
                'size' => 'normal',
                'not null' => TRUE,
            ),
            'request_timestamp' => array(
                'type' => 'int',
                'size' => 'big',
                'not null' => TRUE,
            ),
        ),
        'primary key' => array(
            'nid',
            'uid',
        ),
    );
}

/**
 * 'ToW ApacheSolr Requests' table.
 */
function tow_db_tables_tow_apachesolr_requests() {
    return array(
        'fields' => array(
            'rid' => array(
                'type' => 'serial',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'query' => array(
                'type' => 'text',
                'size' => 'big',
                'not null' => TRUE,
            ),
            'stats' => array(
                'type' => 'text',
                'size' => 'big',
                'not null' => TRUE,
            ),
            'hash' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'tnid' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'timestamp' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
        ),
        'primary key' => array(
            'rid',
        ),
    );
}

/**
 * 'ToW Dataset Counter' table.
 */
function tow_db_tables_tow_dataset_counter() {
    return array(
        'fields' => array(
            'nid' => array(
                'type' => 'int',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'count' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'timestamp' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
        ),
        'primary key' => array(
            'nid',
        ),
    );
}

/**
 * 'ToW Field Info' table.
 */
function tow_db_tables_tow_field_info() {
    return array(
        'fields' => array(
            'nid' => array(
                'type' => 'int',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'ind' => array(
                'type' => 'int',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'description' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
            ),
            'statistics' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
            ),
        ),
        'primary key' => array(
            'nid',
            'ind',
        ),
    );
}

/**
 * 'ToW Import Progress' table.
 */
function tow_db_tables_tow_import_progress() {
    return array(
        'fields' => array(
            'sid' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'default' => '',
            ),
            'hash' => array(
                'type' => 'int',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'stage1' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'default' => '',
            ),
            'stage2' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'default' => '',
            ),
            'stage3' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'default' => '',
            ),
            'stage4' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'default' => '',
            ),
            'stage5' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'default' => '',
            ),
            'stage6' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'default' => '',
            ),
            'timestamp' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
        ),
        'indexes' => array(
            'hash' => array(
                'hash',
            ),
            'sid' => array(
                'sid',
            ),
        ),
    );
}

/**
 * 'ToW Invites' table.
 */
function tow_db_tables_tow_invites() {
    return array(
        'fields' => array(
            'nid' => array(
                'type' => 'int',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'token' => array(
                'type' => 'varchar',
                'length' => 32,
                'not null' => TRUE,
            ),
            'email' => array(
                'type' => 'varchar',
                'length' => 64,
                'not null' => TRUE,
            ),
        ),
        'indexes' => array(
            'nid' => array(
                'nid',
                'token',
            ),
            'email' => array(
                'email',
            ),
        ),
    );
}

/**
 * 'ToW Link Tables' table.
 */
function tow_db_tables_tow_link_tables() {
    return array(
        'fields' => array(
            'record_nid' => array(
                'type' => 'int',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'table_nid' => array(
                'type' => 'int',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'dataset_nid' => array(
                'type' => 'int',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'op' => array(
                'type' => 'varchar',
                'length' => 6,
                'not null' => TRUE,
            ),
            'timestamp' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
        ),
    );
}

/**
 * 'ToW Table Counter' table.
 */
function tow_db_tables_tow_table_counter() {
    return array(
        'fields' => array(
            'nid' => array(
                'type' => 'int',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'count' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
        ),
        'primary key' => array(
            'nid',
        ),
    );
}

/**
 * 'ToW Term Timestamps' table.
 */
function tow_db_tables_tow_term_timestamps() {
    return array(
        'fields' => array(
            'tid' => array(
                'type' => 'int',
                'size' => 'normal',
                'unsigned' => TRUE,
                'not null' => TRUE,
            ),
            'created' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'default' => 0,
            ),
            'updated' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'default' => 0,
            ),
            'first_used' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'default' => 0,
            ),
            'last_used' => array(
                'type' => 'int',
                'size' => 'big',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'default' => 0,
            ),
        ),
        'primary key' => array(
            'tid',
        ),
    );
}
