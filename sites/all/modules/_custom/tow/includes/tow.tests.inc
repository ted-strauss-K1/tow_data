<?php

/**
 * @file
 *
 * Testing interface.
 */
/* * **************************************************************************************************
 * Hook handlers
 * ************************************************************************************************** */

/**
 * Returns an array of menu items.
 * Each item for single test.
 * Items will be used inside hook_menu implementation.
 */
function tow_tests_menu() {
    $items = array();

    // Truncate CCK tables.
    $items['tests/tow_tests_truncate_cck'] = array(
        'title' => 'Truncate cck tables',
        'access arguments' => array('administer site configuration'),
        'page callback' => 'tow_tests_truncate_cck',
        'menu_name' => 'menu-tests',
        'file' => "includes/tests/tow.tests.truncate_cck.inc",
    );

    // Truncate linked records.
    $items['tests/tow_tests_truncate_linked_records'] = array(
        'title' => 'Truncate linked records',
        'access arguments' => array('administer site configuration'),
        'page callback' => 'tow_tests_truncate_linked_records',
        'menu_name' => 'menu-tests',
        'file' => "includes/tests/tow.tests.truncate_linked_records.inc",
    );

    // Truncate nodes.
    $items['tests/tow_tests_truncate_nodes'] = array(
        'title' => 'Truncate nodes',
        'access arguments' => array('administer site configuration'),
        'page callback' => 'tow_tests_truncate_nodes',
        'menu_name' => 'menu-tests',
        'file' => "includes/tests/tow.tests.truncate_nodes.inc",
    );

    // Generate Data.
    $items['tests/tow_tests_generate_data'] = array(
        'title' => 'Generate Data',
        'access arguments' => array('administer site configuration'),
        'page callback' => 'tow_tests_generate_data',
        'menu_name' => 'menu-tests',
        'file' => "includes/tests/tow.tests.generate_data.inc",
    );

    // Switch table engine.
    $items['tests/switch_table_engine'] = array(
        'title' => 'Switch table engine',
        'access arguments' => array('administer site configuration'),
        'page callback' => 'tow_tests_switch_table_engine',
        'menu_name' => 'menu-tests',
        'file' => "includes/tests/tow.tests.switch_table_engine.inc",
    );

    // Foreign keys.
    $items['tests/fk'] = array(
        'title' => 'Foreign keys',
        'access arguments' => array('administer site configuration'),
        'page callback' => 'tow_tests_fk',
        'menu_name' => 'menu-tests',
        'file' => "includes/tests/tow.tests.fk.inc",
    );

    return $items;
}

/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Initializes timer for performance analysis.
 */
function tow_timer_init($timer_name, $action) {
    global $_tow_timer;
    global $_tow_timer_index;
    global $_tow_print_timings;

    if (!$_tow_timer_index) {
        $_tow_print_timings[$timer_name] = 1;
        $_tow_timer = array();
        $_tow_timer_index = 0;
        $_tow_timer[$_tow_timer_index]['action'] = $action;
        $_tow_timer[$_tow_timer_index]['time'] = microtime(true);
    } else {

        // When timer is already initialized.
        tow_timer_tick($action);
    }
}

/**
 * Shows timings of the registered actions.
 */
function tow_timer_show_results($timer_name) {
    global $_tow_timer;
    global $_tow_timer_index;
    global $_tow_print_timings;

    // Print timings only if $timer_name corresponds to the initial name.
    if ($_tow_print_timings[$timer_name]) {

        $timings = array();
        foreach ($_tow_timer as $value) {
            $timings[$value['action']] = $value['elapsed'];
        }
        drupal_set_message(filter_xss("<pre>" . print_r($timings, TRUE) . "</pre>"));

        unset($timings);
        unset($_tow_timer);
        unset($_tow_timer_index);
        unset($_tow_print_timings);
    }
}

/**
 * Timer tick.
 */
function tow_timer_tick($action_finished) {
    global $_tow_timer;
    global $_tow_timer_index;

    if ($_tow_timer_index || $_tow_timer_index === 0) {
        $_tow_timer[++$_tow_timer_index]['action'] = $action_finished;
        $_tow_timer[$_tow_timer_index]['time'] = microtime(true);
        $_tow_timer[$_tow_timer_index]['elapsed'] = $_tow_timer[$_tow_timer_index]['time'] - $_tow_timer[$_tow_timer_index - 1]['time'];
    }
}

/* * **************************************************************************************************
 * Helper functions
 * ************************************************************************************************** */

/**
 * Returns a random string.
 */
function _tow_tests_get_random_string($str_len) {
    global $_tow_data;
    global $_tow_spaces;
    global $_tow_spaces_count;

    // Generation.
    $len = mt_rand(1, $str_len);
    $data_index = mt_rand(0, $_tow_spaces_count - 1 - $len);
    $res = substr($_tow_data, $_tow_spaces[$data_index] + 1, $_tow_spaces[$data_index + $len] - $_tow_spaces[$data_index] - 1);

    return $res;
}
