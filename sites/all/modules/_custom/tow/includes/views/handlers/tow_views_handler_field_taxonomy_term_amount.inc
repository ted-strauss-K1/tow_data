<?php

/**
 * @file
 * 
 * Implementation of tow_views_handler_field_taxonomy_term_amount class.
 */

/**
 * Field handler for user's top taxonomy.
 */
class tow_views_handler_field_taxonomy_term_amount extends views_handler_field {

  /**
   * Defines the actions to be done for forming a view's query.
   */
  function query() {
    $this->ensure_my_table();
    $formula = "(SELECT COUNT(*) FROM {term_node} tn WHERE tn.tid = $this->table_alias.tid)";
    $this->query->add_field(NULL, $formula, $this->table_alias . '_' . $this->real_field);
  }

  /**
   * Rendering.
   */
  function render($values) {
    return $values->{$this->table_alias . '_' . $this->real_field};
  }
}
