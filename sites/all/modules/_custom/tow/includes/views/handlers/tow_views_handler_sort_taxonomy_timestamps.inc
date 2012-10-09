<?php

/**
 * @file
 * 
 * Implementation of tow_views_handler_sort_taxonomy_timestamps class.
 */

/**
 * Sort handler for vote amount per user.
 */
class tow_views_handler_sort_taxonomy_timestamps extends views_handler_sort {
  
  /**
   * Called to add the sort to a query.
   */
  function query() {
    $this->ensure_my_table();
    
    // Add ORDER BY statement.
    $this->query->add_orderby($this->table_alias, $this->real_field, $this->options['order']);
  }
}
