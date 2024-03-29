<?php

/**
 * @file
 *   HeartbeatAccess stream object defining activity in ALL groups.
 */


heartbeat_include('HeartbeatAccess');

/**
 * Class GroupsActivity
 *   Concrete class to prepare messages for activity in groups
 *
 */
class GroupsActivity extends HeartbeatAccess {

  /**
   * Skip active user.
   * We never want to skip the active user.
   */
  public function skipActiveUser() {
    return FALSE;
  }

  /**
   * Override of the prepareStream function
   *
   * @param $text
   * @return HeartbeatParser object
   */
  protected function prepareStream() {

    global $user, $language;
    $this->stream->offset_sql = $this->_offset_sql;

    // Prevent already assigned language to be overriden
    if (empty($this->stream->language)) {
      $this->stream->language = $language->language;
    }

    // Sql parts
    $this->stream->sql_start = "SELECT
      hm.hid AS hid, hm.message AS message_orig,
      hm.message_concat AS message_concat_orig,
      hm.attachments AS attachments, hm.concat_args,
      hm.custom AS custom, hm.description as description,
      hm.perms, hm.variables AS variables_orig,
      ua.*,
      1 AS count
    FROM {heartbeat_activity} ua
    LEFT JOIN {heartbeat_messages} hm ON ua.message_id = hm.message_id
    LEFT JOIN {users} ua_users ON ua.uid = ua_users.uid
    LEFT JOIN {users} ua_target_users ON ua.uid_target = ua_target_users.uid
    WHERE
      ua.in_group = 1
      AND ua.language = '%s'
      AND (ua_users.uid = 0 OR ua_users.status = 1)
      AND (ua_target_users.uid = 0 OR ua_target_users.status = 1)
    ";

    // Calls with an offset uaid will fetch newer messages
    if ($this->stream->latest_uaid > 0) {
      $this->stream->sql_start .= "AND (ua.uaid > %d ) ";
    }
    else {
      $this->stream->sql_start .= "AND (ua.timestamp < %d ";

      // Add sql to delete too old messages if set
      if ($this->stream->oldest_date) {
        $this->stream->sql_start .= "AND ua.timestamp > %d ";
      }
      $this->stream->sql_start .= ")";
    }

    $this->checkDeniedMessages();

    $this->stream->sql_end = " ORDER BY timestamp DESC";

    $heartbeat = $this->createHeartbeatParser();

    return $heartbeat;
  }

  public function dressUpMessages(HeartbeatParser $heartbeat) {

    $heartbeat->raw_messages = $this->resultSql();

    return $heartbeat;
  }

  /**
   * Function to add a part of a sql to a query built by views UI
   *
   * @param object $view The view handler object by reference to add our part to the query
   */
  public function addViewQuery(&$view) {

    $view->query->set_where_group('and', 'andgroup');
    $view->query->add_where('andgroup', "$view->table_alias.in_group = %d ", 1);

  }

}
