<?php // $Id$

/**
 * @file
 * Functions for integrating the Rules module with Email Change Confirmation.
 */

/**
 * Implementation of hook_rules_event_info().
 * @ingroup rules
 */
function email_confirm_rules_event_info() {
  return array(
    'email_confirm_email_change_request' => array(
      'label' => t('User has requested email change'),
      'arguments' => array(
        'user' => array(
          'type' => 'user',
          'label' => t('User object of the user requesting email change')
        ),
        'old_email' => array(
          'type' => 'string',
          'label' => t('Current email address of the user requesting email change'),
        ),
        'new_email' => array(
          'type' => 'string',
          'label' => t('New email address of the user requesting email change'),
        ),
      ),
      'module' => 'Email Change Confirmation',
    ),
    'email_confirm_email_change_confirmation' => array(
      'label' => t('User has confirmed email change'),
      'arguments' => array(
        'user' => array(
          'type' => 'user',
          'label' => t('User object of the user confirming email change')
        ),
        'old_email' => array(
          'type' => 'string',
          'label' => t('Current email address of the user confirming email change'),
        ),
        'new_email' => array(
          'type' => 'string',
          'label' => t('New email address of the user confirming email change'),
        ),
      ),
      'module' => 'Email Change Confirmation',
    ),
  );
}
