<?php

/**
 * Implementation of hook_heartbeat_message_info().
 */
function tow_heartbeat_messages_feature_heartbeat_message_info() {
  $messages = array();

  // Exported heartbeat message: heartbeat_delete_content
  $messages['heartbeat_delete_content'] = array (
    'message' => '!username\'s !node_type "!node_title" has been deleted.',
    'message_concat' => '!username has updated %node_title%',
    'message_id' => 'heartbeat_delete_content',
    'concat_args' => 
    array (
      'type' => 'single',
      'group_by' => 'user',
      'group_target' => 'node_title',
      'group_by_target' => '',
      'group_num_max' => '',
      'merge_separator' => ',',
      'merge_end_separator' => ' and',
      'roles' => 
      array (
        0 => '0',
        1 => '0',
        2 => '0',
        3 => '0',
      ),
      'merge_separator_t' => '0',
      'merge_end_separator_t' => '1',
    ),
    'description' => '',
    'perms' => '1',
    'custom' => 1,
    'variables' => 
    array (
    ),
  );

  // Exported heartbeat message: heartbeat_tag_content
  $messages['heartbeat_tag_content'] = array (
    'message' => '!username\'s !node_type "!node_title" has been tagged as !tags.',
    'message_concat' => '!username has updated %node_title%',
    'message_id' => 'heartbeat_tag_content',
    'concat_args' => 
    array (
      'type' => 'single',
      'group_by' => 'user',
      'group_target' => 'node_title',
      'group_by_target' => '',
      'group_num_max' => '',
      'merge_separator' => ',',
      'merge_end_separator' => 'and',
      'roles' => 
      array (
        0 => '0',
        1 => '0',
        2 => '0',
        3 => '0',
      ),
      'merge_separator_t' => '0',
      'merge_end_separator_t' => '1',
    ),
    'description' => '',
    'perms' => '1',
    'custom' => 1,
    'variables' => 
    array (
    ),
  );

  // Exported heartbeat message: heartbeat_userpoints
  $messages['heartbeat_userpoints'] = array (
    'message' => '!username\'s reputation was updated to !userpoints points.',
    'message_concat' => '',
    'message_id' => 'heartbeat_userpoints',
    'concat_args' => 
    array (
      'type' => 'single',
      'group_by' => 'none',
      'group_target' => '',
      'group_by_target' => '',
      'group_num_max' => '',
      'merge_separator' => '',
      'merge_end_separator' => '',
      'roles' => 
      array (
        0 => '0',
        1 => '0',
        2 => '0',
        3 => '0',
      ),
      'merge_separator_t' => '0',
      'merge_end_separator_t' => '0',
    ),
    'description' => '',
    'perms' => '1',
    'custom' => 1,
    'variables' => 
    array (
    ),
  );

  // Exported heartbeat message: heartbeat_vote_content
  $messages['heartbeat_vote_content'] = array (
    'message' => '!username has voted on !owner\'s !type !content.',
    'message_concat' => '',
    'message_id' => 'heartbeat_vote_content',
    'concat_args' => 
    array (
      'type' => 'single',
      'group_by' => 'none',
      'group_target' => '',
      'group_by_target' => '',
      'group_num_max' => '',
      'merge_separator' => '',
      'merge_end_separator' => '',
      'roles' => 
      array (
        0 => '0',
        1 => '0',
        2 => '0',
        3 => '0',
      ),
      'merge_separator_t' => '0',
      'merge_end_separator_t' => '0',
    ),
    'description' => '',
    'perms' => '1',
    'custom' => 1,
    'variables' => 
    array (
    ),
  );

  // Exported heartbeat message: heartbeat_vote_comment
  $messages['heartbeat_vote_comment'] = array (
    'message' => '!username has voted on !owner\'s !comment to !type !content.',
    'message_concat' => '',
    'message_id' => 'heartbeat_vote_comment',
    'concat_args' => 
    array (
      'type' => 'single',
      'group_by' => 'none',
      'group_target' => '',
      'group_by_target' => '',
      'group_num_max' => '',
      'merge_separator' => '',
      'merge_end_separator' => '',
      'roles' => 
      array (
        0 => '0',
        1 => '0',
        2 => '0',
        3 => '0',
      ),
      'merge_separator_t' => '0',
      'merge_end_separator_t' => '0',
    ),
    'description' => '',
    'perms' => '1',
    'custom' => 1,
    'variables' => 
    array (
    ),
  );


  return $messages;
}
