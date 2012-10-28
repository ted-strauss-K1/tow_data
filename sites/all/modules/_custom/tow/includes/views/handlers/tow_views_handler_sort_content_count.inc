<?php

/**
 * @file
 *
 * Implementation of tow_views_handler_sort_content_count class.
 */

/**
 * Sort handler for content amount per user.
 *
 * This handler enables content type selection.
 */
class tow_views_handler_sort_content_count extends views_handler_sort {

    /**
     * Extends basic options defenition with a set of content types for selection.
     */
    function option_definition() {
        $options = parent::option_definition();

        $options['content_types'] = array(
            'contains' => array(),
        );

        return $options;
    }

    /**
     * Extends options form.
     */
    function options_form(&$form, &$form_state) {
        parent::options_form($form, $form_state);

        $content_types = node_get_types('names');
        $form['content_types'] = array(
            '#type' => 'checkboxes',
            '#title' => t('Content Types'),
            '#options' => $content_types,
            '#description' => t('Define content types to include while calculating content amount.'),
            '#default_value' => $this->options['content_types'],
        );
    }

    /**
     * Called to add the sort to a query.
     */
    function query() {
        $this->ensure_my_table();

        // Get selected content types.
        $content_types = $this->options['content_types'];
        foreach ($content_types as $k => $v) {
            if (!$v) {
                unset($content_types[$k]);
            }
        }

        // Define a formula for field.
        $formula = "(SELECT COUNT(*) FROM {node} n WHERE";
        foreach ($content_types as $ct) {
            $formula .= " n.type = '$ct' OR";
        }
        $formula = trim($formula, ' OR');
        $formula .= (empty($content_types)) ? "" : " AND";
        $formula .= " n.uid = $this->table_alias.uid)";

        // Add ORDER BY statement.
        $this->query->add_orderby(NULL, $formula, $this->options['order'], $this->real_field);
    }

}
