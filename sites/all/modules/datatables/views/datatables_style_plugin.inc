<?php
/**
 * @file
 * Contains the datatables style plugin.
 */

/**
 * Style plugin to render each item as a row in a datatables.
 *
 * @ingroup views_style_plugins
 */
class datatables_style_plugin extends views_plugin_style_table {
  function option_definition() {
    $options = parent::option_definition();
    unset($options['sticky']);
    unset($options['override']);
    
    $options['elements']['search_box'] = array('default' => TRUE);
    $options['elements']['table_info'] = array('default' => TRUE);
    return $options;
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    unset($form['sticky']);
    unset($form['override']);

    $form['description_markup']['#value'] = t('DataTables works best if you disable views pager and configure the page to show all values, since DataTabels contains its own pager implementation.<br/><br/>Place fields into columns; you may combine multiple fields into the same column. If you do, the separator in the column specified will be used to separate the fields. Check the sortable box to make that column click sortable, and check the default sort radio to determine which column will be sorted by default, if any. You may control column order and field labels in the fields section.');


    $form['elements'] = array(
      '#type' => 'fieldset',
      '#title' => t('Widgets & Elements'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,        
    );
    $form['elements']['search_box'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable the search filter box.'),
      '#default_value' => !empty($this->options['elements']['search_box']),
      '#description' => 'The search filter box allows users to dynamically filter the results in the table.  Disabling this option will hide the search filter box from users.',
    );
    $form['elements']['table_info'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable the table information display.'),
      '#default_value' => !empty($this->options['elements']['table_info']),
      '#description' => t('This shows information about the data that is currently visible on the page, including information about filtered data if that action is being performed.'),
    );
    $form['elements']['save_state'] = array(
      '#type' => 'checkbox',
      '#title' => t('Save State'),
      '#default_value' => !empty($this->options['elements']['save_state']),
      '#description' => t("DataTables can use cookies in the end user's web-browser in order to store it's state after each change in drawing. What this means is that if the user were to reload the page, the table should remain exactly as it was (length, filtering, pagination and sorting)"),
    );
    
    $form['layout'] = array(
      '#type' => 'fieldset',
      '#title' => t('Layout and Display'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,      
    );
    
    $form['layout']['autowidth'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable auto-width calculation.'),
      '#default_value' => !empty($this->options['layout']['autowidth']),
      '#description' => t('Enable or disable automatic column width calculation. This can be disabled as an optimisation (it takes some time to calculate the widths).'),
    );
        
    $form['layout']['themeroller'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable jQuery UI ThemeRoller Support'),
      '#default_value' => !empty($this->options['layout']['themeroller']),
      '#description' => t("Create markup and classes to support jQuery UI's ThemeRoller"),
    );
    
    $form['layout']['sdom'] = array(
      '#type' => 'textfield',
      '#title' => t('Set sDOM Initialization Parameter'),
      '#default_value' => $this->options['layout']['sdom'],
      '#description' => t("Use the sDOM parameter to rearrange the interface elements. See the <a href='@sdom_documentation_url'>Datatables sDOM documentation</a> for details on how to use this feature", array('@sdom_documentation_url' => 'http://www.datatables.net/examples/basic_init/dom.html')),
    );
    
    $form['pages'] = array(
      '#type' => 'fieldset',
      '#title' => t('Pagination and Page Length'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['pages']['pagination_style'] = array(
      '#type' => 'select',
      '#title' => t('Pagination Style'),
      '#default_value' => $this->options['pages']['pagination_style'] ? $this->options['pages']['pagination_style'] : 0,
      '#options' => array(
        0 => t('Two-Button (Default)'), 
        'full_numbers' => t('Full Numbers'),
        'no_pagination' => t('No Pagination'),
      ),
      '#description' => t('Selects which pagination style should be used.'),
    );
    
    $form['pages']['length_change'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable Length Change Selection Box'),
      '#default_value' => !empty($this->options['pages']['length_change']),
      '#description' => t('Enable or page length selection menu'),
    );    
    $form['pages']['display_length'] = array(
      '#type' => 'select',
      '#title' => t('Default Page Length'),
      '#default_value' => !empty($this->options['pages']['display_length']),
      '#options' => array(10 => 10, 25 => 25, 50 => 50, 100 => 100),
      '#description' => t('Default number of records to show per page. May be adjusted by users if Length Selection is enabled'),
    );
        
    $columns = $this->display->handler->get_field_labels();
    $form['hidden_columns'] = array(
      '#type' => 'fieldset',
      '#title' => t('Hidden and Expandable Columns'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    
    foreach($columns as $column_name => $column_label) {
      $form['hidden_columns'][$column_name] = array(
        '#type' => 'select',
        '#title' => $column_label,
        '#default_value' => $this->options['hidden_columns'][$column_name] ? $this->options['hidden_columns'][$column_name] : 0,
        '#options' => array(0 => 'Visible', 'hidden' => 'Hidden', 'expandable' => 'Hidden and Expandable'),
      );
    }
  }
}
