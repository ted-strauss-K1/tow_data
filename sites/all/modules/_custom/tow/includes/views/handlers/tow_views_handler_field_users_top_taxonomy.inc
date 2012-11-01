<?php

/**
 * @file
 *
 * Implementation of tow_views_handler_field_users_top_taxonomy class.
 */

/**
 * Field handler for user's top taxonomy.
 */
class tow_views_handler_field_users_top_taxonomy extends views_handler_field {

    /**
     * Extends basic options defenition.
     */
    function option_definition() {
        $options = parent::option_definition();

        // Vocabularies array.
        $options['vocabularies'] = array(
            'contains' => array(),
        );

        // Number of terms to output.
        $options['terms_amount'] = array(
            'default' => 3,
        );

        // Term separation method.
        $options['separator'] = array(
            'default' => 0,
        );

        // Case tags has to be linked to term pages.
        $options['is_linked'] = array(
            'default' => 1,
        );

        return $options;
    }

    /**
     * Extends options form.
     */
    function options_form(&$form, &$form_state) {
        parent::options_form($form, $form_state);

        // Vocabularies array.
        $vocabularies = taxonomy_get_vocabularies();
        foreach ($vocabularies as $vid => $vocabulary) {
            $vocabularies[$vid] = $vocabulary->name;
        }
        $form['vocabularies'] = array(
            '#type' => 'checkboxes',
            '#title' => t('Vocabularies'),
            '#options' => $vocabularies,
            '#description' => t('Define the vocabularies to get information from.'),
            '#default_value' => $this->options['vocabularies'],
        );

        // Number of terms to output.
        $form['terms_amount'] = array(
            '#type' => 'textfield',
            '#title' => t('Terms Amount'),
            '#description' => t('Define the amount of terms to get from the vocabs.'),
            '#default_value' => $this->options['terms_amount'],
        );

        // Term separation method.
        $form['separator'] = array(
            '#type' => 'radios',
            '#title' => t('Separation Method'),
            '#options' => array(
                0 => 'Inline',
                1 => 'Ordered list',
                2 => 'Unordered list',
            ),
            '#description' => t('Define the type of term separation.'),
            '#default_value' => $this->options['separator'],
        );

        // Case tags has to be linked to term pages.
        $form['is_linked'] = array(
            '#type' => 'checkbox',
            '#title' => t('Link tags to their taxonomy terms'),
            '#default_value' => $this->options['is_linked'],
        );
    }

    /**
     * Defines the actions to be done for forming a view's query.
     */
    function query() {
        $this->ensure_my_table();

        // Options.
        $vocabularies = $this->options['vocabularies'];
        $limit = $this->options['terms_amount'];
        $is_linked = $this->options['is_linked'];
        $method = $this->options['separator'];

        // Vocabularies' and user's portions of SQL request.
        foreach ($vocabularies as $k => $vid) {
            if ($vid == 0) {
                unset($vocabularies[$k]);
            }
        }
        $voc_sql = empty($vocabularies) ? "" : " WHERE ";
        $user_sql = empty($vocabularies) ? " WHERE " : " AND ";
        foreach ($vocabularies as $vid) {
            $voc_sql .= " td.vid = " . $vid . " OR";
        }
        $voc_sql = trim($voc_sql, " OR");
        $user_sql .= "n.uid = $this->table_alias.uid";

        // Generate query.
        $formula = "(SELECT array_to_string(array(SELECT (t.termid || ':' || t.qty || ':' || t.name) FROM (SELECT td.name AS name, td.tid AS termid, COUNT(*) AS qty FROM {term_data} td INNER JOIN {term_node} tn ON td.tid = tn.tid INNER JOIN {node} n ON tn.nid = n.nid $voc_sql $user_sql GROUP BY termid ORDER BY qty DESC LIMIT $limit) t), '|'))";
        $this->query->add_field(NULL, $formula, $this->table_alias . '_' . $this->real_field);
    }

    /**
     * Rendering.
     */
    function render($values) {

        // Options.
        $vocabularies = $values->options['vocabularies'];
        $limit = $this->options['terms_amount'];
        $is_linked = $this->options['is_linked'];
        $method = $this->options['separator'];
        $f_alias = $this->table_alias . '_' . $this->real_field;

        // Get terms.
        $terms = array();
        $aux = explode('|', $values->{$f_alias});
        foreach ($aux as $term_info) {
            $aux_aux = explode(':', $term_info);
            $term->tid = $aux_aux[0];
            $term->qty = $aux_aux[1];
            $term->name = $aux_aux[2];
            $terms[] = ($is_linked) ? l($term->name, "taxonomy/term/$term->tid", array('amount' => $term->qty)) : $term->name;
        }

        // Generate the output.
        switch ($method) {

            // Inline.
            case 0:
                $output = '<div class="top-taxonomy">';
                foreach ($terms as $term) {
                    $output .= $term . ', ';
                }
                $output = trim($output, ', ');
                $output .= '</div>';
                break;

            // Ordered list.
            case 1:
                $output = '<ol class="top-taxonomy">';
                foreach ($terms as $term) {
                    $output .= '<li>' . $term . '</li>';
                }
                $output .= '</ol>';
                break;

            // Unordered list.
            case 2:
                $output = '<ul class="top-taxonomy">';
                foreach ($terms as $term) {
                    $output .= '<li>' . $term . '</li>';
                }
                $output .= '</ul>';
                break;
        }

        return $output;
    }

}
