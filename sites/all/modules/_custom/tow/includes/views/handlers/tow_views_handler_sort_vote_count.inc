<?php

/**
 * @file
 *
 * Implementation of tow_views_handler_sort_vote_count class.
 */

/**
 * Sort handler for vote amount per user.
 */
class tow_views_handler_sort_vote_count extends views_handler_sort {

    /**
     * Called to add the sort to a query.
     */
    function query() {
        $this->ensure_my_table();

        // Define a formula for field.
        $formula = "(SELECT COUNT(*) FROM {votingapi_vote} vv WHERE vv.uid = $this->table_alias.uid)";

        // Add ORDER BY statement.
        $this->query->add_orderby(NULL, $formula, $this->options['order'], $this->real_field);
    }

}
