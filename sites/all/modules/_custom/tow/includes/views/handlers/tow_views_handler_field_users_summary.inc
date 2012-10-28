<?php

/**
 * @file
 *
 * Implementation of tow_views_handler_field_users_summary class.
 */

/**
 * Field handler for user's top taxonomy.
 */
class tow_views_handler_field_users_summary extends views_handler_field {

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

    /**
     * Defines the actions to be done for forming a view's query.
     */
    function query() {
        $this->ensure_my_table();

        $formula = $this->formula();
        $this->query->add_field(NULL, $formula, $this->table_alias . '_' . $this->real_field);
    }

    /**
     * Rendering.
     */
    function render($values) {
        return $values->{$this->table_alias . '_' . $this->real_field};
    }

}
