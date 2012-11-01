<?php

/**
 * @file
 *
 * Implementation of tow_views_handler_filter_users_summary class.
 */

/**
 * Filter handler for creation interval.
 */
class tow_views_handler_filter_users_summary extends views_handler_filter_string {

    // Exposed filter options.
    var $no_single = TRUE;
    var $no_optional = TRUE;

    /**
     * Add this filter to the query.
     */
    function query() {
        $this->ensure_my_table();
        $field = $this->formula();
        $upper = $this->case_transform();

        $info = $this->operators();
        if (!empty($info[$this->operator]['method'])) {
            $this->{$info[$this->operator]['method']}($field, $upper);
        }
    }

    /**
     * Formula.
     */
    function formula() {

        $included_fields = array();
        $formula = "(CONCAT(";

        $view = $this->view;
        foreach ($view->display as $display) {
            foreach ($display->display_options['fields'] as $id => $field) {
                if (!$field['exclude']) {
                    $included_fields[$id] = $field;
                }
            }
        }
        unset($included_fields[$this->real_field]);
        unset($included_fields['top_taxonomy']);

        foreach ($included_fields as $field) {
            $field = (object) $field;
            $formula .= "$field->table.$field->field, ' ', ";
        }
        $formula = trim($formula, ", ' ', ");
        $formula .= "))";

        return $formula;
    }

}
