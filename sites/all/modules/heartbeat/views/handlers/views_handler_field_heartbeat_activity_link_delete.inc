<?php

/**
 * @file Views handler field link to delete a heartbeat activity message.
 *
 */

/**
 * Class views_handler_field_heartbeat_activity_link_delete
 *
 */
class views_handler_field_heartbeat_activity_link_delete extends views_handler_field {

  function construct() {
    parent::construct();
    $this->additional_fields['uaid'] = 'uaid';
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['text'] = array('default' => '', 'translatable' => TRUE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    // ensure user has access to edit this node.
    $activity = new stdClass();
    $activity->uaid = $values->{$this->aliases['uaid']};
    if (!_heartbeat_message_delete_access($activity->uaid)) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('delete');
    return l($text, "heartbeat/delete/$activity->uaid", array('query' => drupal_get_destination()));
  }

}