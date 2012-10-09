<?php

/**
 * @file
 * 
 * Implementation of tow_views_handler_field_taxonomy_timestamps class.
 */

/**
 * Field handler for user's top taxonomy.
 */
class tow_views_handler_field_taxonomy_timestamps extends views_handler_field {

  /**
   * Options definition.
   */
  function option_definition() {
    $options = parent::option_definition();
    
    // Date format.
    $options['date_format'] = array(
      'default' => 'Y-m-d H:i:s',
    );
    
    return $options;
  }
  
  /**
   * Extends options form.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Vocabularies array.
    $form['date_format'] = array(
      '#type' => 'textfield',
      '#title' => t('Date format'),
      '#description' => t('Define the output format of the timestamp.'),
      '#value' => $this->options['date_format'],
    );
  }
  
  /**
   * Defines the actions to be done for forming a view's query.
   */
  function query() {
    $this->additional_fields[$this->real_field] = array('table' => $this->table_alias, 'field' => $this->real_field);
    
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  /**
   * Rendering.
   */
  function render($values) {
    return date($this->options['date_format'], $values->{$this->table_alias . '_' . $this->real_field});
  }
}
