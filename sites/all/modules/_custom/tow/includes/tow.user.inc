<?php

/**
 * @file
 *
 * User API.
 */
/* * **************************************************************************************************
 * Hook handlers
 * ************************************************************************************************** */

/**
 * Part of impelementation of hook_block().
 */
function tow_user_view_block() {

    // User info.
    $account = user_load(arg(1));
    user_build_content($account);
    $block['content'] = theme('user_profile', $account);

    return $block;
}

/**
 * Part of impelementation of hook_menu_alter().
 */
function tow_user_menu_alter(&$items) {

    // User activity.
    $items['user/%user/activity'] = $items['user/%user/heartbeat/privateheartbeat'];
    $items['user/%user/activity']['title'] = 'Activity';
    $items['user/%user/activity']['weight'] = 1;
    unset($items['user/%user/activity']['title callback']);
    unset($items['user/%user/activity']['title arguments']);
    $items['user/%user_uid_optional'] = $items['user/%user/activity'];
    $items['user/%user/activity']['type'] = MENU_DEFAULT_LOCAL_TASK;
    unset($items['user/%user/heartbeat/privateheartbeat']);

    // User votes.
    $items['user/%user/votes'] = $items['user/%user/vote_up_down/votes'];
    $items['user/%user/votes']['weight'] = 5;
    unset($items['user/%user/vote_up_down/votes']);

    // User reputation.
    $items['user/%user/reputation'] = array(
        'title' => 'Reputation',
        'page callback' => 'tow_user_list_userpoints',
        'page arguments' => array(1),
        'access callback' => 'userpoints_access_my_points',
        'access arguments' => array(1),
        'type' => MENU_LOCAL_TASK,
        'weight' => 4,
    );
    unset($items['myuserpoints']);

    // Remove other local tasks.
    $items['user/%user/devel']['type'] = MENU_CALLBACK;
    $items['user/%user/devel/load']['type'] = MENU_CALLBACK;
    $items['user/%user/devel/render']['type'] = MENU_CALLBACK;
    $items['user/%user/devel/token']['type'] = MENU_CALLBACK;
    $items['user/%user/track/navigation']['type'] = MENU_CALLBACK;
    $items['user/%user/ulogin']['type'] = MENU_CALLBACK;
    $items['user/%user/ulogin/delete']['type'] = MENU_CALLBACK;
    $items['user/%user/view']['type'] = MENU_CALLBACK;
    $items['user/%user_category/edit']['weight'] = 6;

    // Login/register.
    $items['user/register']['page arguments'] = array('user_login');
    unset($items['user/register']['file']);
}

/**
 * Part of impelementation of hook_form_alter().
 */
function tow_user_profile_form_alter(&$form, $form_state) {
    $form['#submit'][] = 'tow_user_profile_update_message';
}

function tow_user_profile_update_message(&$form, $form_state) {
    $upd_fields = '';
    foreach ($form['Personal information'] as $item) {
          $is_profile = strpos($item['#name'], 'profile_');
          if(($is_profile === 0) && ($item['#value'] != $item['#default_value'])) {
            $upd_fields .= $item['#title'] . ', ';
          }
    }
    if ($upd_fields != '') {
       $message = substr($upd_fields, 0, -2);
       drupal_set_message('You\'ve just updated ' . $message . ' in your user profile');
    }
}


/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

function tow_user_list_userpoints() {
    $overall_total = 0;
    $unapproved_total = 0;
    $approved_total = 0;

    global $user;

    // User which are displaying points for.
    $uid = (int) arg(1);
    if ($uid) {
        if ($uid != $user->uid && !user_access(USERPOINTS_PERM_ADMIN)) {

            // If not the same as logged in user.
            drupal_access_denied();
            return;
        }
    } else {

        // Default to currently logged in user.
        $uid = $user->uid;
    }
    $point_user = user_load(array('uid' => $uid));

    // Check permissions.
    if (!user_access(USERPOINTS_PERM_VIEW) && !user_access(USERPOINTS_PERM_VIEW_OWN)) {

        // User has no access to view points.
        drupal_access_denied();
        return;
    }

    // Total.
    $args['overall_total'] = userpoints_get_current_points($uid, 'all');

    $header = array(
        array('data' => t('!Points', userpoints_translation()), 'field' => 'points'),
        array('data' => t('Date'), 'field' => 'time_stamp', 'sort' => 'desc'),
        array('data' => t('Description'), 'field' => 'description'),
    );
    $sql = "

    SELECT
      p.points,
      p.time_stamp,
      p.description

      FROM
        {userpoints_txn} p

      WHERE
        p.uid = %d

  ";
    $sql .= tablesort_sql($header);
    $pager_limit = variable_get(USERPOINTS_REPORT_LIMIT, 25);
    $result = pager_query($sql, $pager_limit, 0, NULL, $uid);
    $stati = userpoints_txn_status();
    $num_rows = 0;
    $rows = array();
    while ($row = db_fetch_object($result)) {
        $num_rows++;
        $status = $stati[$row->status];
        if ($row->description) {
            $description = filter_xss($row->description);
        } else {
            $description = t('None');
        }
        $rows[] = array(
            array('data' => $row->points, 'align' => 'center'),
            array('data' => _tow_dataset_ago($row->time_stamp, 'small'), 'align' => 'center'),
            array('data' => $description),
        );
        if ($num_rows <= 0) {
            $rows[] = array(
                array('data' => t('No !Points earned', userpoints_translation()), 'colspan' => 5, 'align' => 'center')
            );
        }
    }
    return theme('tow_user_list_userpoints', $args, $header, $rows);
}