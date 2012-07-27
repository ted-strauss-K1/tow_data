<?php

/**
 * @file
 * 
 * Debug functions.
 */



/****************************************************************************************************
 * Functions
 ****************************************************************************************************/

/**
 * Adds a log entry to the site's journal.
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
  $message .= ($var != '') ? kprint_r($var, TRUE, 'The researched variable is ') : '';

  // Log entry.
  watchdog('ToW', $message, array(), WATCHDOG_DEBUG);
}
