<?php

/**
 * Implementation of hook_rules_defaults().
 */
function tow_rules_userpoints_feature_rules_defaults() {
  return array(
    'rules' => array(
      'rules_tow_userpoints_create_document_250_words' => array(
        '#type' => 'rule',
        '#set' => 'event_node_insert',
        '#label' => 'ToW Userpoints: Create document 250 words',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'type' => array(
                'document' => 'document',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#name' => 'rules_condition_content_is_type',
            '#info' => array(
              'label' => 'Created content is Document',
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
              'code' => '$body = $node->body;
$words_count = str_word_count($body);
if ($words_count > 250) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'node',
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
              'points' => '1',
              'description' => '',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'user',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_delete_vote_on_comment' => array(
        '#type' => 'rule',
        '#set' => 'event_voting_rules_delete_comment',
        '#label' => 'ToW Userpoints: Delete Vote on Comment',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
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
                'node' => 'node_loaded',
              ),
            ),
            '#name' => 'rules_condition_content_is_type',
            '#info' => array(
              'label' => 'Content with id [comment:comment-nid] is Dataset or Saved Search',
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
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Deleting vote on comment to [node_loaded:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'user',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'description' => array(
                    '0' => 'comment',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
          ),
          '1' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Load a user account',
              'new variables' => array(
                'comment_author' => array(
                  'label' => 'comment author',
                  'label callback' => FALSE,
                  'type' => 'user',
                ),
              ),
              'module' => 'User',
              'eval input' => array(
                '0' => 'userid',
                '1' => 'username',
              ),
            ),
            '#name' => 'rules_action_load_user',
            '#settings' => array(
              'username' => '',
              'userid' => '[comment:comment-author-uid]',
              '#argument map' => array(
                'user_loaded' => 'comment_author',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'userid' => array(
                    '0' => 'comment',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
          ),
          '2' => array(
            '#type' => 'action',
            '#settings' => array(
              'points' => '-[vote:value]',
              'description' => 'Being deleted a vote on comment to [node_loaded:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'comment_author',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'points' => array(
                    '0' => 'vote',
                  ),
                ),
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_delete_vote_on_content' => array(
        '#type' => 'rule',
        '#set' => 'event_voting_rules_delete_node',
        '#label' => 'ToW Userpoints: Delete Vote on Content',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
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
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Deleting vote on [node:type-name] [node:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'user',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'description' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
          '1' => array(
            '#type' => 'action',
            '#settings' => array(
              '#argument map' => array(
                'node' => 'node',
                'author' => 'author',
              ),
            ),
            '#name' => 'rules_action_node_load_author',
            '#info' => array(
              'label' => 'Load the The node being voted on author',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'new variables' => array(
                'author' => array(
                  'label' => 'the node being voted on author',
                  'type' => 'user',
                  'label callback' => 'rules_action_node_load_author_variable_label',
                ),
              ),
              'module' => 'Node',
            ),
            '#weight' => 0,
          ),
          '2' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-[vote:value]',
              'description' => 'Being deleted a vote on [node:type-name] [node:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'author',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'points' => array(
                    '0' => 'vote',
                  ),
                  'description' => array(
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
      'rules_tow_userpoints_minus_affiliation' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus affiliation',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_affiliation != NULL && $account->profile_affiliation == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_age' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus age',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_age != NULL && $account->profile_age == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_bio' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus bio',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_bio != NULL && $account->profile_bio == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_city' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus city',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_city != NULL && $account->profile_city == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_country' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus country',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_country != NULL && $account->profile_country == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
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
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_firstname' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus firstName',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_firstName != NULL && $account->profile_firstName == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_gender' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus gender',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_gender != NULL && $account->profile_gender == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_lastname' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus lastName',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_lastName != NULL && $account->profile_lastName == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_photourl' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus photoURL',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_photoURL != NULL && $account->profile_photoURL == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_state' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus state',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_state != NULL && $account->profile_state == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_thumbnailurl' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus thumbnailURL',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_thumbnailURL != NULL && $account->profile_thumbnailURL == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_minus_zip' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: minus zip',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_bio != NULL && $account->profile_bio == NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '-1',
              'description' => 'Removing an update to user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_affiliation' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus affiliation',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_affiliation == NULL && $account->profile_affiliation != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_age' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus age',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_age == NULL && $account->profile_age != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_bio' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus bio',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_bio == NULL && $account->profile_bio != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_city' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus city',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_city == NULL && $account->profile_city != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_country' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus country',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_country == NULL && $account->profile_country != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_firstname' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus firstName',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_firstName == NULL && $account->profile_firstName != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_gender' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus gender',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_gender == NULL && $account->profile_gender != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_lastname' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus lastName',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_lastName == NULL && $account->profile_lastName != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_photourl' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus photoURL',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_photoURL == NULL && $account->profile_photoURL != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_state' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus state',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_state == NULL && $account->profile_state != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_thumbnailurl' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus thumbnailURL',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_thumbnailURL == NULL && $account->profile_thumbnailURL != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_plus_zip' => array(
        '#type' => 'rule',
        '#set' => 'event_profile_update',
        '#label' => 'ToW Userpoints: plus zip',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'if($account_unchanged->profile_zip == NULL && $account->profile_zip != NULL) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'account',
                '1' => 'account_unchanged',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Updating user profile.',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'account',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_post_comment_on_saved_search' => array(
        '#type' => 'rule',
        '#set' => 'event_comment_insert',
        '#label' => 'ToW Userpoints: Post Comment on saved search',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'type' => array(
                'saved_search' => 'saved_search',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#name' => 'rules_condition_content_is_type',
            '#info' => array(
              'label' => 'Commented content is Saved Search',
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
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Commenting [node:type-name] [node:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'user',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'description' => array(
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
      'rules_tow_userpoints_post_content' => array(
        '#type' => 'rule',
        '#set' => 'event_node_insert',
        '#label' => 'ToW Userpoints: Post Content',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
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
              'label' => 'Created content is Dataset or Saved Search',
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
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Posting new [node:type-name] [node:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'user',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'description' => array(
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
      'rules_tow_userpoints_update_document_250_words' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'ToW Userpoints: update document 250 words',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'type' => array(
                'document' => 'document',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#name' => 'rules_condition_content_is_type',
            '#info' => array(
              'label' => 'Updated content is Document',
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
          '1' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => '$body_before = $node_unchanged->body;
$body_after = $node->body;
$words_count_before = str_word_count($body_before);
$words_count_after = str_word_count($body_after);
if ($words_count_after > 250 && $words_count_before <= 250) {
  return true;
}
else {
  return false;
}',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
            ),
            '#type' => 'condition',
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'points' => '1',
              'description' => '',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'author_unchanged',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_view_content' => array(
        '#type' => 'rule',
        '#set' => 'event_node_view',
        '#label' => 'ToW Userpoints: View Content',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => '$views = db_result(db_query("SELECT t.count AS count FROM {tow_node_views_counter} t WHERE t.nid = %d LIMIT 1", $node->nid));
if (!$views) {
  return FALSE;
}
$views %= 10;

return ($views === 0) ? TRUE : FALSE;',
              'vars' => array(
                '0' => 'node',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'Execute custom PHP code',
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
          '1' => array(
            '#type' => 'condition',
            '#settings' => array(
              'type' => array(
                'saved_search' => 'saved_search',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#name' => 'rules_condition_content_is_type',
            '#info' => array(
              'label' => 'Viewed content is Saved Search',
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
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Being viewed with [node:type-name] [node:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'author',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'description' => array(
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
      'rules_tow_userpoints_vote_on_comment' => array(
        '#type' => 'rule',
        '#set' => 'event_voting_rules_insert_comment',
        '#label' => 'ToW Userpoints: Vote on Comment',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Content with id [comment:comment-nid] is Dataset or Saved Search',
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
                'node' => 'node_loaded',
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
              'points' => '1',
              'description' => 'Voting on comment to [node_loaded:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'user',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'description' => array(
                    '0' => 'node_loaded',
                  ),
                ),
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
          ),
          '1' => array(
            '#info' => array(
              'label' => 'Load a user account',
              'new variables' => array(
                'comment_author' => array(
                  'label' => 'comment author',
                  'label callback' => FALSE,
                  'type' => 'user',
                ),
              ),
              'module' => 'User',
              'eval input' => array(
                '0' => 'userid',
                '1' => 'username',
              ),
            ),
            '#name' => 'rules_action_load_user',
            '#settings' => array(
              'username' => '',
              'userid' => '[comment:comment-author-uid]',
              '#argument map' => array(
                'user_loaded' => 'comment_author',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'userid' => array(
                    '0' => 'comment',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
          '2' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '[vote:value]',
              'description' => 'Being voted on comment to [node_loaded:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'comment_author',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'points' => array(
                    '0' => 'vote',
                  ),
                  'description' => array(
                    '0' => 'node_loaded',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_tow_userpoints_vote_on_content' => array(
        '#type' => 'rule',
        '#set' => 'event_voting_rules_insert_node',
        '#label' => 'ToW Userpoints: Vote on Content',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'Theory on Wheels',
          '1' => 'Userpoints',
          'tow_rules_userpoints_feature' => 'tow_rules_userpoints_feature',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
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
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '1',
              'description' => 'Vote on [node:type-name] [node:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'user',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'description' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
          '1' => array(
            '#type' => 'action',
            '#settings' => array(
              '#argument map' => array(
                'node' => 'node',
                'author' => 'author',
              ),
            ),
            '#name' => 'rules_action_node_load_author',
            '#info' => array(
              'label' => 'Load the The node being voted on author',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'new variables' => array(
                'author' => array(
                  'label' => 'the node being voted on author',
                  'type' => 'user',
                  'label callback' => 'rules_action_node_load_author_variable_label',
                ),
              ),
              'module' => 'Node',
            ),
            '#weight' => 0,
          ),
          '2' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Grant points to a user',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'Userpoints',
              'eval input' => array(
                '0' => 'points',
                '1' => 'entity_id',
                '2' => 'description',
                '3' => 'operation',
              ),
            ),
            '#name' => 'userpoints_action_grant_points',
            '#settings' => array(
              'points' => '[vote:value]',
              'description' => 'Being voted on [node:type-name] [node:title-link].',
              'operation' => '',
              'entity_type' => '',
              'entity_id' => '',
              'tid' => '',
              '#argument map' => array(
                'user' => 'author',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'points' => array(
                    '0' => 'vote',
                  ),
                  'description' => array(
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
