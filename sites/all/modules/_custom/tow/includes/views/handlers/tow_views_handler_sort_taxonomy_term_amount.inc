<?php

/**
 * @file
 * 
 * Implementation of tow_views_handler_sort_taxonomy_term_amount class.
 */



/**
 * Sort handler for term amount.
 */
class tow_views_handler_sort_taxonomy_term_amount extends views_handler_sort {
  
  /**
   * Called to add the sort to a query.
   */
  function query() {
    $this->query->add_orderby($this->table, $this->real_field, $this->options['order']);
  }
}
