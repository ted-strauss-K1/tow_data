<?php

/**
 * @file
 * 
 * Implementation of tow_views_handler_filter_users_creation_interval class.
 */

/**
 * Filter handler for creation interval.
 */
class tow_views_handler_filter_users_creation_interval extends views_handler_filter {
  
  /**
   * Define default values for the filter.
   */
  function option_definition() {
    $options = parent::option_definition();

    // Add some extra info.
    $options['operator'] = array('default' => '>=');
    $options['value'] = array('default' => 4);
    
    return $options;
  }

  /**
   * Define the operators the filter would use.
   */
  function operators() {

    // There would be only the one operator.
    $operators = array(
      '>=' => array(
        
        // Define the text to be shown for this operator.
        'title' => t('Created not less than'),
        
        // The 'method' option defines the function to be called for generating the query.
        'method' => 'op_created_after',
        
        // The 'short' is used for representing the operator in summaries etc.
        'short' => t('>='),
        
        // The 'values' defines the number of default values that will be used.
        'values' => 1,
      ),
    );

    return $operators;
  }

  /**
   * Provide a list of all the operators.
   */
  function operator_options($which = 'title') {

    $options = array();

    // Set 'id' => 'title' pairs.
    foreach ($this->operators() as $id => $info) {
      $options[$id] = $info[$which];
    }

    return $options;
  }

  /**
   * Extend the form that represents the filter according to our situation.
   */
  function value_form(&$form, &$form_state) {

    // Operators.
    $operator_options = $this->operator_options();
    $form['operator'] = array(
      '#type' => 'markup',
      '#value' => $operator_options['>='],
    );

    // Values.
    $form['value'] = array(
      '#type' => 'radios',
      '#default_value' => $this->value['value'],
      '#options' => $this->intervals(),
    );
  }
  
  /**
   * Defines the actions to be done to form a view's query.
   */
  function query() {
 
    // Define the field which will be used by the filter for extending the query.
    $this->ensure_my_table();
    $this->operator = (isset($this->operator)) ? $this->operator : '>=';
    $field = "users.created";

    // Call the callbacks of the selected operators.
    $info = $this->operators();
    if (!empty($info[$this->operator]['method'])) {
      $this->{$info[$this->operator]['method']}($field);
    }

  }

  /**
   * Exposed form.
   */
  function exposed_form(&$form, &$form_state) {
    parent::exposed_form($form, $form_state);
    unset($form['operator']);
  }
  
  /**
   * Define a callback function.
   */
  function op_created_after($field) {
    
    // Define the table & the field names.
    $parts = explode('.', $field);
    $table = $parts[0];
    $field = $parts[1];

    $value = $this->value['value'];

    $intervals = $this->intervals();
    if ($value == 4) {
      $value = 0;
    }
    else {
      if ($value == 2) {
        $value = strtotime('-3 months');
      }
      else {
        $value = strtotime('-1 ' . strtolower($intervals[$value]));
      }
    }

    $this->query->add_where($this->options['group'], "$table.$field >= %d", $value);
  }

  /**
   * Admin summary.
   */
  function admin_summary() {
    if (!empty($this->options['exposed'])) {
      return t('exposed');
    }
    
    $this->operator = (isset($this->operator)) ? $this->operator : '>=';
    
    // Define the current operator.
    $options = $this->operator_options('short');
    $intervals = $this->intervals();
    $output = '';
    if ($this->value['value'] == 4) {
      $output = 'all';
    }
    else {

      // Set the output.
      $output = check_plain($options[$this->operator]);
      $output .= ' ' . check_plain(strtolower($intervals[$this->value['value']]));
    }

    return $output;
  }

  /**
   * Intervals.
   */
  function intervals() {
    return array(
      0 => t('Week'), 
      1 => t('Month'), 
      2 => t('Quarter'), 
      3 => t('Year'), 
      4 => t('All'),
    );
  }
}
