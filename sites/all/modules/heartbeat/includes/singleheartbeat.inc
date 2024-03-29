<?php

/**
 * @file
 *   Defines a stream for one activity message.
 */

heartbeat_include('HeartbeatAccess');

/**
 * Concrete class SingleHeartbeat
 *   Defines a stream for one activity message.
 */
class SingleHeartbeat extends heartbeataccess {

  /**
   * Skip active user.
   * We never want to skip the active user. not for
   * logged-in user and not for displayed user (profile).
   * This is ofcourse because private stream is intended to
   * show own activity.
   */
  public function skipActiveUser() {
    return FALSE;
  }

  /**
   * hasAccess
   *
   * @param $text
   * @return boolean to indicate the access to the stream
   */
  protected function hasAccess() {
    return TRUE;
  }

  /**
   * prepareStream
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
    // If users are involved (uid or uid_target) in heartbeat_activity
    // check if they are not blocked (status = 1).
    $this->stream->sql_start = "SELECT
      hm.hid AS hid, hm.message AS message_orig,
      hm.message_concat AS message_concat_orig,
      hm.attachments AS attachments, hm.concat_args,
      hm.custom AS custom, hm.description as description,
      hm.perms, hm.variables AS variables_orig,
      ua.*, ua_users.*,
      1 AS count
    FROM {heartbeat_activity} ua
    LEFT JOIN {heartbeat_messages} hm ON ua.message_id = hm.message_id
    LEFT JOIN {users} ua_users ON ua.uid = ua_users.uid
    LEFT JOIN {users} ua_target_users ON ua.uid_target = ua_target_users.uid
    WHERE
      ua.uaid = %d
      AND (ua_users.uid = 0 OR ua_users.status = 1)
      AND (ua_target_users.uid = 0 OR ua_target_users.status = 1)
    ";

    $this->stream->sql_end = "";

    $heartbeat = $this->createHeartbeatParser();

    return $heartbeat;
  }

  /**
   * resultSql
   *
   * @param $sql String query to extend the current query
   * @param $values Array of values to substitute in the query extension
   * @return array results
   */
  protected function resultSql($sql = '', $values = array()) {

    $result = db_query($this->stream->sql_start, $this->stream->latest_uaid);

    $results = array();
    while ($row = db_fetch_object($result) ) {

      $template = new HeartbeatMessageTemplate($row->hid, $row->message_id, $row->message_orig, $row->message_concat_orig, $row->concat_args);
      $template->perms = $row->perms;
      $template->custom = $row->custom;
      $template->description = $row->description;
      $template->set_variables($row->variables_orig);
      $template->set_attachments($row->attachments);
      $template->set_roles(isset($template->concat_args['roles']) ? $template->concat_args['roles'] : array());

      $template->variables['build_mode'] = 'none';
      $heartbeatactivity = new HeartbeatActivity($row, $template);

      $results[$row->uaid] = $heartbeatactivity;
    }

    $stream = heartbeat_stream_load($this->getAccess());

    $this->stream->filters = array();
    $this->stream->variables['build_mode'] = 'none';

    return $results;
  }

}
