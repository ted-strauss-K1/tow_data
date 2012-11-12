<?php

/**
 * @file
 *   Contains the heartbeat message RSS row style plugin.
 */

/**
 * Class views_plugin_row_heartbeat_rss
 *   Plugin which performs a node_view on the resulting object
 *   and formats it as an RSS item.
 */
class views_plugin_row_heartbeat_rss extends views_plugin_row {

  function render($row) {

    $item = new stdClass();
    if (isset($row->message)) {
      $item->title = '';
      $item->description = $row->message;
      $item->time = $row->timestamp;
    }

    return theme($this->theme_functions(), $this->view, $this->options, $item);
  }
}