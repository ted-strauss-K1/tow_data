<?php

/**
 * Implementation of hook_rules_defaults().
 */
function tow_rules_heartbeat_feature_rules_defaults() {
  return array(
    'rules' => array(
      'rules_tow_heartbeat_delete_content' => array(
        '#type' => 'rule',
        '#set' => 'event_node_delete',
        '#label' => 'ToW Heartbeat: Delete Content',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Heartbeat',
          'tow_rules_heartbeat_feature' => 'tow_rules_heartbeat_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'type' => array(
                'dataset' => 'dataset',
                'saved_search' => 'saved_search',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#name' => 'rules_condition_content_is_type',
            '#info' => array(
              'label' => 'Deleted content is Dataset or Saved Search',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'module' => 'Node',
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Logs default activity',
              'eval input' => array(
                '0' => 'uid_param',
                '1' => 'uid_target_param',
                '2' => 'nid_param',
                '3' => 'nid_target_param',
                '4' => 'message_id_param',
                '5' => 'variables_param',
              ),
              'module' => 'heartbeat',
            ),
            '#name' => 'heartbeat_rules_default_action',
            '#settings' => array(
              'uid_param' => '[user:uid]',
              'uid_target_param' => '[node:author-uid]',
              'nid_param' => '',
              'nid_target_param' => '',
              'message_id_param' => 'heartbeat_delete_content',
              'variables_param' => '@username=|=[node:author-name-url]-|-@node_type=|=[node:type-name]-|-@node_title=|=[node:title-link]-|-',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'variables_param' => array(
                    '0' => 'node',
                  ),
                  'uid_param' => array(
                    '0' => 'user',
                  ),
                  'uid_target_param' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_heartbeat_edit_content' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'ToW Heartbeat: Edit Content',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Heartbeat',
          'tow_rules_heartbeat_feature' => 'tow_rules_heartbeat_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Updated content is Dataset or Document or Saved Search',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'module' => 'Node',
            ),
            '#name' => 'rules_condition_content_is_type',
            '#settings' => array(
              'type' => array(
                'dataset' => 'dataset',
                'document' => 'document',
                'saved_search' => 'saved_search',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#type' => 'condition',
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'uid_param' => '[user:uid]',
              'uid_target_param' => '[author:uid]',
              'nid_param' => '',
              'nid_target_param' => '',
              'message_id_param' => 'heartbeat_edit_node',
              'variables_param' => '@username=|=[author:user-name-url]-|-@node_type=|=[node:type-name]-|-@node_title=|=[node:title-link]-|-',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'variables_param' => array(
                    '0' => 'node',
                    '1' => 'author',
                  ),
                  'uid_param' => array(
                    '0' => 'user',
                  ),
                  'uid_target_param' => array(
                    '0' => 'author',
                  ),
                ),
              ),
            ),
            '#name' => 'heartbeat_rules_default_action',
            '#info' => array(
              'label' => 'Logs default activity',
              'eval input' => array(
                '0' => 'uid_param',
                '1' => 'uid_target_param',
                '2' => 'nid_param',
                '3' => 'nid_target_param',
                '4' => 'message_id_param',
                '5' => 'variables_param',
              ),
              'module' => 'heartbeat',
            ),
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_heartbeat_post_coment' => array(
        '#type' => 'rule',
        '#set' => 'event_comment_insert',
        '#label' => 'ToW Heartbeat: Post Coment',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Heartbeat',
          'tow_rules_heartbeat_feature' => 'tow_rules_heartbeat_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'type' => array(
                'dataset' => 'dataset',
                'saved_search' => 'saved_search',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#name' => 'rules_condition_content_is_type',
            '#info' => array(
              'label' => 'Commented content is Dataset or Saved Search',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'module' => 'Node',
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Logs default activity',
              'eval input' => array(
                '0' => 'uid_param',
                '1' => 'uid_target_param',
                '2' => 'nid_param',
                '3' => 'nid_target_param',
                '4' => 'message_id_param',
                '5' => 'variables_param',
              ),
              'module' => 'heartbeat',
            ),
            '#name' => 'heartbeat_rules_default_action',
            '#settings' => array(
              'uid_param' => '[user:uid]',
              'uid_target_param' => '[node_author:uid]',
              'nid_param' => '',
              'nid_target_param' => '',
              'message_id_param' => 'heartbeat_add_comment',
              'variables_param' => '@username=|=[user:user-name-url]  -|-@owner=|=[node_author:user-name-url]-|-@type=|=[node:title-link]-|-@title=|=[node:title-link]-|-@comment=|=[comment:comment-body-raw]-|-',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'variables_param' => array(
                    '0' => 'comment',
                    '1' => 'node',
                    '2' => 'node_author',
                    '3' => 'user',
                  ),
                  'uid_param' => array(
                    '0' => 'user',
                  ),
                  'uid_target_param' => array(
                    '0' => 'node_author',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_heartbeat_post_content' => array(
        '#type' => 'rule',
        '#set' => 'event_node_insert',
        '#label' => 'ToW Heartbeat: Post Content',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Heartbeat',
          'tow_rules_heartbeat_feature' => 'tow_rules_heartbeat_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Created content is Dataset or Saved Search',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'module' => 'Node',
            ),
            '#name' => 'rules_condition_content_is_type',
            '#settings' => array(
              'type' => array(
                'dataset' => 'dataset',
                'saved_search' => 'saved_search',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#type' => 'condition',
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'action',
            '#settings' => array(
              'uid_param' => '[author:uid]',
              'uid_target_param' => '[author:uid]',
              'nid_param' => '',
              'nid_target_param' => '',
              'message_id_param' => 'heartbeat_add_node',
              'variables_param' => '@username=|=[author:user-name-url]-|-@node_type=|=[node:type-name]-|-@node_title=|=[node:title-link]-|-',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'uid_param' => array(
                    '0' => 'author',
                  ),
                  'uid_target_param' => array(
                    '0' => 'author',
                  ),
                  'variables_param' => array(
                    '0' => 'node',
                    '1' => 'author',
                  ),
                ),
              ),
            ),
            '#name' => 'heartbeat_rules_default_action',
            '#info' => array(
              'label' => 'Logs default activity',
              'eval input' => array(
                '0' => 'uid_param',
                '1' => 'uid_target_param',
                '2' => 'nid_param',
                '3' => 'nid_target_param',
                '4' => 'message_id_param',
                '5' => 'variables_param',
              ),
              'module' => 'heartbeat',
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_heartbeat_tag_content' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'ToW Heartbeat: Tag Content',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Heartbeat',
          'tow_rules_heartbeat_feature' => 'tow_rules_heartbeat_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Updated content is Dataset',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'module' => 'Node',
            ),
            '#name' => 'rules_condition_content_is_type',
            '#settings' => array(
              'type' => array(
                'dataset' => 'dataset',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#type' => 'condition',
          ),
          '1' => array(
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => '$old_terms = $node_unchanged->taxonomy;
$terms = $node->taxonomy;
$old_typed_terms = array();
$typed_terms = array();
foreach ($old_terms as $key) {
  $old_typed_terms[] = $key->name;
}
foreach ($terms as $key) {
  $typed_terms[] = $key->name;
}

if (array_diff($old_typed_terms, $typed_terms) === array_diff($typed_terms, $old_typed_terms)) {
  return FALSE;
}

$added_terms = array_diff($typed_terms, $old_typed_terms);
return (empty($added_terms)) ? FALSE : TRUE;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'condition',
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'action',
            '#settings' => array(
              'code' => '$old_terms = $node_unchanged->taxonomy;
$terms = $node->taxonomy;
$old_typed_terms = array();
$typed_terms = array();
foreach ($old_terms as $key) {
  $old_typed_terms[] = $key->name;
}
foreach ($terms as $key) {
  $typed_terms[] = $key->name;
}

if (array_diff($old_typed_terms, $typed_terms) === array_diff($typed_terms, $old_typed_terms)) {
  return FALSE;
}

$added_terms = array_diff($typed_terms, $old_typed_terms);
$new_tags = \'\';
foreach ($added_terms as $tag) {
  $term_array = taxonomy_get_term_by_name($tag);
  $tid = $term_array[0]->tid;
  $new_tags .= l($tag, \'taxonomy/term/\' . $tid) . \' \';
}
$node->new_tags = $new_tags;
return array(\'node\' => $node);',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'rules_action_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
          ),
          '1' => array(
            '#type' => 'action',
            '#settings' => array(
              'uid_param' => '[user:uid]',
              'uid_target_param' => '[author:uid]',
              'nid_param' => '',
              'nid_target_param' => '',
              'message_id_param' => 'heartbeat_tag_content',
              'variables_param' => '@username=|=[author:user-name-url]-|-@node_type=|=[node:type-name]-|-@node_title=|=[node:title-link]-|-@tags=|=<?php echo $node->new_tags; ?>-|-',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'uid_param' => array(
                    '0' => 'user',
                  ),
                  'uid_target_param' => array(
                    '0' => 'author',
                  ),
                  'variables_param' => array(
                    '0' => 'node',
                    '1' => 'author',
                  ),
                ),
                'rules_input_evaluator_php' => array(
                  'variables_param' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#name' => 'heartbeat_rules_default_action',
            '#info' => array(
              'label' => 'Logs default activity',
              'eval input' => array(
                '0' => 'uid_param',
                '1' => 'uid_target_param',
                '2' => 'nid_param',
                '3' => 'nid_target_param',
                '4' => 'message_id_param',
                '5' => 'variables_param',
              ),
              'module' => 'heartbeat',
            ),
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_heartbeat_update_profile' => array(
        '#type' => 'rule',
        '#set' => 'event_user_update',
        '#label' => 'ToW Heartbeat: Update Profile',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Heartbeat',
          'tow_rules_heartbeat_feature' => 'tow_rules_heartbeat_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'action',
            '#settings' => array(
              'uid_param' => '[user:uid]',
              'uid_target_param' => '[account:uid]',
              'nid_param' => '',
              'nid_target_param' => '',
              'message_id_param' => 'heartbeat_edit_account',
              'variables_param' => '@username=|=[account:user-name-url]-|-',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'variables_param' => array(
                    '0' => 'account',
                  ),
                  'uid_param' => array(
                    '0' => 'user',
                  ),
                  'uid_target_param' => array(
                    '0' => 'account',
                  ),
                ),
              ),
            ),
            '#name' => 'heartbeat_rules_default_action',
            '#info' => array(
              'label' => 'Logs default activity',
              'eval input' => array(
                '0' => 'uid_param',
                '1' => 'uid_target_param',
                '2' => 'nid_param',
                '3' => 'nid_target_param',
                '4' => 'message_id_param',
                '5' => 'variables_param',
              ),
              'module' => 'heartbeat',
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_heartbeat_userpoints' => array(
        '#type' => 'rule',
        '#set' => 'event_userpoints_event_points_awarded',
        '#label' => 'ToW Heartbeat: Userpoints',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Heartbeat',
          'tow_rules_heartbeat_feature' => 'tow_rules_heartbeat_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Logs default activity',
              'eval input' => array(
                '0' => 'uid_param',
                '1' => 'uid_target_param',
                '2' => 'nid_param',
                '3' => 'nid_target_param',
                '4' => 'message_id_param',
                '5' => 'variables_param',
              ),
              'module' => 'heartbeat',
            ),
            '#name' => 'heartbeat_rules_default_action',
            '#settings' => array(
              'uid_param' => '[user:uid]',
              'uid_target_param' => '[user:uid]',
              'nid_param' => '',
              'nid_target_param' => '',
              'message_id_param' => 'heartbeat_userpoints',
              'variables_param' => '@username=|=[user:user-name-url]-|-@userpoints=|=[user:userpoints]-|-',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'variables_param' => array(
                    '0' => 'user',
                  ),
                  'uid_param' => array(
                    '0' => 'user',
                  ),
                  'uid_target_param' => array(
                    '0' => 'user',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_heartbeat_vote_on_comment' => array(
        '#type' => 'rule',
        '#set' => 'event_voting_rules_insert_comment',
        '#label' => 'ToW Heartbeat: Vote on Comment',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Heartbeat',
          'tow_rules_heartbeat_feature' => 'tow_rules_heartbeat_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#info' => array(
              'label' => 'Load content by id',
              'arguments' => array(
                'nid' => array(
                  'type' => 'number',
                  'label' => 'Content ID',
                ),
                'vid' => array(
                  'type' => 'number',
                  'label' => 'Content Revision ID',
                  'description' => 'If you want to load a specific revision, specify it\'s revision id. Else leave it empty to load the current revision.',
                  'required' => FALSE,
                ),
              ),
              'new variables' => array(
                'node_loaded' => array(
                  'label' => 'content with id [comment:comment-nid]',
                  'type' => 'node',
                  'label callback' => 'rules_action_load_node_variable_label',
                ),
              ),
              'module' => 'Node',
            ),
            '#name' => 'rules_action_load_node',
            '#settings' => array(
              'nid' => '[comment:comment-nid]',
              'vid' => '',
              '#argument map' => array(
                'node_loaded' => 'node_loaded',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'nid' => array(
                    '0' => 'comment',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
          '1' => array(
            '#weight' => 0,
            '#type' => 'action',
            '#settings' => array(
              'uid_param' => '[user:uid]',
              'uid_target_param' => '[node_loaded:author-uid]',
              'nid_param' => '',
              'nid_target_param' => '',
              'message_id_param' => 'heartbeat_vote_comment',
              'variables_param' => '@username=|=[user:user]-|-@owner=|=[node_loaded:author-name-url]-|-@comment=|=[comment:comment-title]-|-@type=|=[node_loaded:type-name]-|-@content=|=[node_loaded:title-link]-|-',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'variables_param' => array(
                    '0' => 'node_loaded',
                    '1' => 'comment',
                    '2' => 'user',
                  ),
                  'uid_param' => array(
                    '0' => 'user',
                  ),
                  'uid_target_param' => array(
                    '0' => 'node_loaded',
                  ),
                ),
              ),
            ),
            '#name' => 'heartbeat_rules_default_action',
            '#info' => array(
              'label' => 'Logs default activity',
              'eval input' => array(
                '0' => 'uid_param',
                '1' => 'uid_target_param',
                '2' => 'nid_param',
                '3' => 'nid_target_param',
                '4' => 'message_id_param',
                '5' => 'variables_param',
              ),
              'module' => 'heartbeat',
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_heartbeat_vote_on_content' => array(
        '#type' => 'rule',
        '#set' => 'event_voting_rules_insert_node',
        '#label' => 'ToW Heartbeat: Vote on Content',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Heartbeat',
          'tow_rules_heartbeat_feature' => 'tow_rules_heartbeat_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'type' => array(
                'dataset' => 'dataset',
                'saved_search' => 'saved_search',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#name' => 'rules_condition_content_is_type',
            '#info' => array(
              'label' => 'The node being voted on is Dataset or Saved Search',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'module' => 'Node',
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Logs default activity',
              'eval input' => array(
                '0' => 'uid_param',
                '1' => 'uid_target_param',
                '2' => 'nid_param',
                '3' => 'nid_target_param',
                '4' => 'message_id_param',
                '5' => 'variables_param',
              ),
              'module' => 'heartbeat',
            ),
            '#name' => 'heartbeat_rules_default_action',
            '#settings' => array(
              'uid_param' => '[user:uid]',
              'uid_target_param' => '[node:author-uid]',
              'nid_param' => '',
              'nid_target_param' => '',
              'message_id_param' => 'heartbeat_vote_content',
              'variables_param' => '@username=|=[user:user-name-url]-|-@owner=|=[node:author-name-url]-|-@type=|=[node:type-name]-|-@content=|=[node:title-link]-|-',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'variables_param' => array(
                    '0' => 'node',
                    '1' => 'user',
                  ),
                  'uid_param' => array(
                    '0' => 'user',
                  ),
                  'uid_target_param' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
    ),
  );
}
