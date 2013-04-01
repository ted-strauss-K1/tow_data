<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function tow_all_roles_permissions_user_default_permissions() {
  $permissions = array();

  // Exported permission: access avatars
  $permissions['access avatars'] = array(
    'name' => 'access avatars',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: access comments
  $permissions['access comments'] = array(
    'name' => 'access comments',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: access content
  $permissions['access content'] = array(
    'name' => 'access content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
      '3' => 'moderator',
      '4' => 'trudat user',
    ),
  );

  // Exported permission: access feedback form
  $permissions['access feedback form'] = array(
    'name' => 'access feedback form',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: access user profiles
  $permissions['access user profiles'] = array(
    'name' => 'access user profiles',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: access vote up/down statistics
  $permissions['access vote up/down statistics'] = array(
    'name' => 'access vote up/down statistics',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: attach creative commons
  $permissions['attach creative commons'] = array(
    'name' => 'attach creative commons',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: change own username
  $permissions['change own username'] = array(
    'name' => 'change own username',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: create dataset
  $permissions['create dataset'] = array(
    'name' => 'create dataset',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: create document
  $permissions['create document'] = array(
    'name' => 'create document',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: create linked_record
  $permissions['create linked_record'] = array(
    'name' => 'create linked_record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: create page content
  $permissions['create page content'] = array(
    'name' => 'create page content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: create record
  $permissions['create record'] = array(
    'name' => 'create record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: create saved_search
  $permissions['create saved_search'] = array(
    'name' => 'create saved_search',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: create story content
  $permissions['create story content'] = array(
    'name' => 'create story content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: create table
  $permissions['create table'] = array(
    'name' => 'create table',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: delete any dataset
  $permissions['delete any dataset'] = array(
    'name' => 'delete any dataset',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete any document
  $permissions['delete any document'] = array(
    'name' => 'delete any document',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete any linked_record
  $permissions['delete any linked_record'] = array(
    'name' => 'delete any linked_record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete any page content
  $permissions['delete any page content'] = array(
    'name' => 'delete any page content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete any record
  $permissions['delete any record'] = array(
    'name' => 'delete any record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete any saved_search
  $permissions['delete any saved_search'] = array(
    'name' => 'delete any saved_search',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete any story content
  $permissions['delete any story content'] = array(
    'name' => 'delete any story content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete any table
  $permissions['delete any table'] = array(
    'name' => 'delete any table',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete heartbeat activity logs
  $permissions['delete heartbeat activity logs'] = array(
    'name' => 'delete heartbeat activity logs',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete own dataset
  $permissions['delete own dataset'] = array(
    'name' => 'delete own dataset',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: delete own document
  $permissions['delete own document'] = array(
    'name' => 'delete own document',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete own heartbeat activity logs
  $permissions['delete own heartbeat activity logs'] = array(
    'name' => 'delete own heartbeat activity logs',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: delete own linked_record
  $permissions['delete own linked_record'] = array(
    'name' => 'delete own linked_record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: delete own page content
  $permissions['delete own page content'] = array(
    'name' => 'delete own page content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: delete own record
  $permissions['delete own record'] = array(
    'name' => 'delete own record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: delete own saved_search
  $permissions['delete own saved_search'] = array(
    'name' => 'delete own saved_search',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: delete own story content
  $permissions['delete own story content'] = array(
    'name' => 'delete own story content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: delete own table
  $permissions['delete own table'] = array(
    'name' => 'delete own table',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: delete revisions
  $permissions['delete revisions'] = array(
    'name' => 'delete revisions',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit any dataset
  $permissions['edit any dataset'] = array(
    'name' => 'edit any dataset',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit any document
  $permissions['edit any document'] = array(
    'name' => 'edit any document',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit any linked_record
  $permissions['edit any linked_record'] = array(
    'name' => 'edit any linked_record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit any page content
  $permissions['edit any page content'] = array(
    'name' => 'edit any page content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit any record
  $permissions['edit any record'] = array(
    'name' => 'edit any record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit any saved_search
  $permissions['edit any saved_search'] = array(
    'name' => 'edit any saved_search',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit any story content
  $permissions['edit any story content'] = array(
    'name' => 'edit any story content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit any table
  $permissions['edit any table'] = array(
    'name' => 'edit any table',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit own dataset
  $permissions['edit own dataset'] = array(
    'name' => 'edit own dataset',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: edit own document
  $permissions['edit own document'] = array(
    'name' => 'edit own document',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: edit own linked_record
  $permissions['edit own linked_record'] = array(
    'name' => 'edit own linked_record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: edit own page content
  $permissions['edit own page content'] = array(
    'name' => 'edit own page content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: edit own record
  $permissions['edit own record'] = array(
    'name' => 'edit own record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: edit own saved_search
  $permissions['edit own saved_search'] = array(
    'name' => 'edit own saved_search',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: edit own story content
  $permissions['edit own story content'] = array(
    'name' => 'edit own story content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: edit own table
  $permissions['edit own table'] = array(
    'name' => 'edit own table',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
      '2' => 'trudat user',
    ),
  );

  // Exported permission: maintain own activity
  $permissions['maintain own activity'] = array(
    'name' => 'maintain own activity',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: post comments
  $permissions['post comments'] = array(
    'name' => 'post comments',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: post comments without approval
  $permissions['post comments without approval'] = array(
    'name' => 'post comments without approval',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: reset vote up/down votes
  $permissions['reset vote up/down votes'] = array(
    'name' => 'reset vote up/down votes',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: search content
  $permissions['search content'] = array(
    'name' => 'search content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
      '3' => 'moderator',
      '4' => 'trudat user',
    ),
  );

  // Exported permission: see vote up/down node stats
  $permissions['see vote up/down node stats'] = array(
    'name' => 'see vote up/down node stats',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: upload avatar in profile
  $permissions['upload avatar in profile'] = array(
    'name' => 'upload avatar in profile',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: use creative commons user defaults
  $permissions['use creative commons user defaults'] = array(
    'name' => 'use creative commons user defaults',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: use realname
  $permissions['use realname'] = array(
    'name' => 'use realname',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: use ulogin
  $permissions['use ulogin'] = array(
    'name' => 'use ulogin',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
      '3' => 'moderator',
      '4' => 'trudat user',
    ),
  );

  // Exported permission: use vote up/down
  $permissions['use vote up/down'] = array(
    'name' => 'use vote up/down',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: use vote up/down on comments
  $permissions['use vote up/down on comments'] = array(
    'name' => 'use vote up/down on comments',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: use vote up/down on nodes
  $permissions['use vote up/down on nodes'] = array(
    'name' => 'use vote up/down on nodes',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: view dataset
  $permissions['view dataset'] = array(
    'name' => 'view dataset',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
      '3' => 'moderator',
      '4' => 'trudat user',
    ),
  );

  // Exported permission: view document
  $permissions['view document'] = array(
    'name' => 'view document',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
      '3' => 'moderator',
      '4' => 'trudat user',
    ),
  );

  // Exported permission: view feedback messages
  $permissions['view feedback messages'] = array(
    'name' => 'view feedback messages',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: view heartbeat messages
  $permissions['view heartbeat messages'] = array(
    'name' => 'view heartbeat messages',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: view linked_record
  $permissions['view linked_record'] = array(
    'name' => 'view linked_record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
      '3' => 'moderator',
      '4' => 'trudat user',
    ),
  );

  // Exported permission: view own userpoints
  $permissions['view own userpoints'] = array(
    'name' => 'view own userpoints',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: view personal heartbeat activity
  $permissions['view personal heartbeat activity'] = array(
    'name' => 'view personal heartbeat activity',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: view record
  $permissions['view record'] = array(
    'name' => 'view record',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
      '3' => 'moderator',
      '4' => 'trudat user',
    ),
  );

  // Exported permission: view saved_search
  $permissions['view saved_search'] = array(
    'name' => 'view saved_search',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
      '3' => 'moderator',
      '4' => 'trudat user',
    ),
  );

  // Exported permission: view table
  $permissions['view table'] = array(
    'name' => 'view table',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
      '3' => 'moderator',
      '4' => 'trudat user',
    ),
  );

  // Exported permission: view userpoints
  $permissions['view userpoints'] = array(
    'name' => 'view userpoints',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  // Exported permission: view vote up/down count on nodes
  $permissions['view vote up/down count on nodes'] = array(
    'name' => 'view vote up/down count on nodes',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
      '2' => 'moderator',
      '3' => 'trudat user',
    ),
  );

  return $permissions;
}
