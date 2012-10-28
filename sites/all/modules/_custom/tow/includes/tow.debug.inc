<?php

/**
 * @file
 *
 * Debug functions.
 */
/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Prints the info on the callers stack.
 */
function tow_debug_backtrace_dpm() {

    // Get the chain of callers.
    $debug = debug_backtrace();

    // Message.
    dpm($debug);
}

/**
 * Adds a log entry to the site's journal with the info on the callers stack.
 */
function tow_debug_backtrace_watchdog() {

    // Get the chain of callers.
    $debug = debug_backtrace();

    // Define the message to be shown.
    # $message .= dvr(array($debug), TRUE);
    $message .= kprint_r(array($debug), TRUE);

    // Log entry.
    watchdog('ToW', $message, array(), WATCHDOG_DEBUG);
}

/**
 * prints the info on the researched variable.
 */
function tow_debug_dpm($var = '') {

    // Get the chain of callers.
    $debug = debug_backtrace();

    // Define the message to be shown.
    $message = '<p>';
    $message .= 'Called from <strong>';
    $message .= $debug[1]['function'];
    $message .= '()</strong> located on <strong>line ';
    $message .= $debug[0]['line'];
    $message .= '</strong> in <strong>file ';
    $message .= $debug[0]['file'];
    $message .= '</strong>.';
    $message .= '</p>';
    # $message .= ($var == '') ? '' : dvr(array($var), TRUE);
    $message .= ($var == '') ? '' : kprint_r(array($var), TRUE);

    // Messaging.
    dpm($var, $message);
}

/**
 * Adds a log entry to the site's journal with the info on the researched variable.
 */
function tow_debug_watchdog($var = '') {

    // Get the chain of callers.
    $debug = debug_backtrace();

    // Define the message to be shown.
    $message = '<p>';
    $message .= 'Called from <strong>';
    $message .= $debug[1]['function'];
    $message .= '()</strong> located on <strong>line ';
    $message .= $debug[0]['line'];
    $message .= '</strong> in <strong>file ';
    $message .= $debug[0]['file'];
    $message .= '</strong>.';
    $message .= '</p>';
    # $message .= ($var == '') ? '' : dvr(array($var), TRUE);
    $message .= ($var == '') ? '' : kprint_r(array($var), TRUE);

    // Log entry.
    watchdog('ToW', $message, array(), WATCHDOG_DEBUG);
}
