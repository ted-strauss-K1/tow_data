<?php

/**
 * @file
 *
 * Theming functions for Theory on Wheels.
 */
/* * **************************************************************************************************
 * Handlers
 * ************************************************************************************************** */

/**
 * Themes facet links.
 */
function theme_tow_search_inner_facet_link($facet_text, $path, $options = array(), $count, $active = FALSE, $num_found = NULL) {
    $options['attributes']['class'][] = 'apachesolr-facet';

    if ($active) {
        $options['attributes']['class'][] = 'active';
    }

    $options['attributes']['class'] = implode(' ', $options['attributes']['class']);
    $link = _tow_theme_search_inner_l($facet_text, $path, $options) . " ($count)";

    return $link;
}

/**
 * Themes unclicked links.
 */
function theme_tow_search_inner_unclick_link($facet_text, $path, $options = array(), $count = 0, $active = FALSE, $num_found = NULL) {

    if (empty($options['html'])) {
        $facet_text = check_plain(html_entity_decode($facet_text));
    }

    $options['attributes']['class'] = 'tow-inner-search-selected';
    $link = _tow_theme_search_inner_l($facet_text . " ($count)", $path, $options);

    return $link;
}

/**
 * Themes a sortable table.
 */
function theme_tow_table_sortable_table($header, $rows, $nid, $uid = NULL, $attributes = array(), $caption = NULL) {
    global $user;

    $active = _tow_theme_table_headers($header, $nid);

    // Hide the first column for all users except a table owner.
    if ($user->uid == $uid || user_access('edit any table')) {
        $header[0]['data'] = 'Action';

        foreach ($rows as $key => $value) {
            $record_nid = $rows[$key]['data'][0]['data'];
            $rows[$key]['data'][0] = l('edit', 'node/' . $record_nid . '/edit', array('query' => 'destination=table/' . $nid));
            $rows[$key]['data'][0] .= ' ' . l('delete', 'node/' . $record_nid . '/delete', array('query' => 'destination=table/' . $nid));
        }
    } else {
        unset($header[0]);
        foreach ($rows as $key => $value) {
            unset($rows[$key]['data'][0]);
        }
    }

    // Add sticky headers, if applicable.
    if (count($header)) {
        drupal_add_js('misc/tableheader.js');

        // Add 'sticky-enabled' class to the table to identify it for JS.
        // This is needed to target tables constructed by this function.
        $attributes['class'] = empty($attributes['class']) ? 'sticky-enabled' : ($attributes['class'] . ' sticky-enabled');
    }

    $output = "<table" . drupal_attributes($attributes) . ">\n";

    if (isset($caption)) {
        $output .= "<caption>" . $caption . "</caption>\n";
    }

    // Format the table header:
    if (count($header)) {

        $ts = tablesort_init($header);

        // HTML requires that the thead tag has tr tags in it followed by tbody
        // tags. Using ternary operator to check and see if we have any rows.
        $output .= (count($rows) ? ' <thead><tr>' : ' <tr>');

        foreach ($header as $cell) {
            $output .= _theme_table_cell($cell, TRUE);
        }
        // Using ternary operator to close the tags based on whether or not there are rows.
        $output .= (count($rows) ? " </tr></thead>\n" : "</tr>\n");
    } else {
        $ts = array();
    }

    // Format the table rows:
    if (count($rows)) {
        $output .= "<tbody>\n";
        $flip = array('even' => 'odd', 'odd' => 'even');
        $class = 'even';
        foreach ($rows as $number => $row) {
            $attributes = array();

            // Check if we're dealing with a simple or complex row.
            if (isset($row['data'])) {
                foreach ($row as $key => $value) {
                    if ($key == 'data') {
                        $cells = $value;
                    } else {
                        $attributes[$key] = $value;
                    }
                }
            } else {
                $cells = $row;
            }
            if (count($cells)) {

                // Add odd/even class.
                $class = $flip[$class];
                if (isset($attributes['class'])) {
                    $attributes['class'] .= ' ' . $class;
                } else {
                    $attributes['class'] = $class;
                }

                // Build row.
                $output .= ' <tr' . drupal_attributes($attributes) . '>';
                $i = 0;
                if (isset($active)) {
                    $cells[$active]['class'] = 'active';
                }
                foreach ($cells as $cell) {
                    $cell = tablesort_cell($cell, $header, $ts, $i++);
                    $output .= _theme_table_cell($cell);
                }
                $output .= " </tr>\n";
            }
        }
        $output .= "</tbody>\n";
    }

    $output .= "</table>\n";

    return $output;
}

/**
 * Themes table's form.
 */
function theme_tow_table_tabular_form($form) {

    $rows = array();
    foreach (element_children($form['data']) as $key) {
        foreach (element_children($form['data'][$key]) as $name) {
            $rows[$key][] = drupal_render($form['data'][$key][$name]);
        }
    }

    return theme('table', $form['header']['#value'], $rows);
}

/**
 * Purpose: Allow my_userpoints to be themeable.
 */
function theme_tow_user_list_userpoints($args, $header, $rows) {
    $pager_limit = variable_get(USERPOINTS_REPORT_LIMIT, 10);
    $output = theme('table', $header, $rows);
    $output .= "<p>";
    if (isset($args) && isset($args['subtotals'])) {
        foreach ($args['subtotals'] as $tid => $data) {
            $output .= '<strong>' . $data['name'] . ' ' . t('!points Balance', userpoints_translation()) . ':</strong> ' . $data['total'] . '<br />';
        }
    }
    $output .= "</p>";
    $output .= "<p>";
    $output .= '<strong>' . t('Total !points Balance', userpoints_translation()) . ':</strong> ' . $args['overall_total'] . '<br />';
    $output .= "</p>";
    $output .= theme('pager', NULL, $pager_limit, 0);

    return $output;
}

/* * **************************************************************************************************
 * Helper functions
 * ************************************************************************************************** */

/**
 * A replacement for l():
 *  - doesn't add the 'active' class;
 *  - retains all $_GET parameters that ApacheSolr may not be aware of;
 *  - if set, $options['query'] MUST be an array.
 *
 * @return
 *   an HTML string containing a link to the given path.
 */
function _tow_theme_search_inner_l($text, $path, $options = array()) {

    // Merge in defaults.
    $options += array(
        'attributes' => array(),
        'html' => FALSE,
        'query' => array(),
    );

    // Don't need this, and just to be safe.
    unset($options['attributes']['title']);

    // Double encode + characters for clean URL Apache quirks.
    if (variable_get('clean_url', '0')) {
        $path = str_replace('+', '%2B', $path);
    }

    // Retain GET parameters that ApacheSolr knows nothing about.
    $query = apachesolr_current_query(NULL, 'tow_inner_search');
    $get = array_diff_key($_GET, array('q' => 1, 'page' => 1), $options['query'], $query->get_url_queryvalues());
    $options['query'] += $get;
    unset($options['query']['zoom']);

    return '<a href="' . check_url(url($path, $options)) . '"' . drupal_attributes($options['attributes']) . '>' . ($options['html'] ? $text : check_plain(html_entity_decode($text))) . '</a>';
}

/**
 * Generates the output of table headers.
 */
function _tow_theme_table_headers(&$headers, $nid) {
    $count = 0;
    $titles = $headers;

    // Generate links.
    foreach ($headers as $field => $header) {
        if (!$header) {
            $offset++;
            continue;
        }
        $headers[$field]['data'] = l($header['data'], 'table/' . $nid, array(
            'query' => 'order=' . $count . '&sort=asc'
                ));
        $count++;
    }

    // Sorting.
    if (is_numeric($_GET['order']) && (int) $_GET['order'] < $count && (int) $_GET['order'] >= 0) {
        $order = $_GET['order'];
        $active = $order + $offset;
        if ($_GET['sort'] == 'asc') {
            $headers[$active]['data'] = l($titles[$active]['data'], 'table/' . $nid, array('query' => 'order=' . $order . '&sort=desc')) . '<img width="13" height="13" title="sort descending" alt="sort icon" src="/misc/arrow-desc.png">';
        } elseif ($_GET['sort'] == 'desc') {
            $headers[$active]['data'] = l($titles[$active]['data'], 'table/' . $nid, array('query' => 'order=' . $order . '&sort=asc')) . '<img width="13" height="13" title="sort descending" alt="sort icon" src="/misc/arrow-asc.png">';
        }
        return $active;
    }
}
