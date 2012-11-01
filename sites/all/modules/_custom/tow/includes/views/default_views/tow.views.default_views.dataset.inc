<?php

/**
 * @file
 *
 * Default Views: Dataset.
 */
/* * **************************************************************************************************
 * Views
 * ************************************************************************************************** */

/**
 * Dataset.
 */
function tow_views_default_views_dataset() {
    $view = new view;
    $view->name = 'dataset';
    $view->description = 'View emulationg "browse database"';
    $view->tag = 'tow';
    $view->base_table = 'node';
    $view->core = 0;
    $view->api_version = '2';
    $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
    $handler = $view->new_display('default', 'Defaults', 'default');
    $handler->override_option('fields', array(
        'title' => array(
            'label' => 'Table',
            'alter' => array(
                'alter_text' => 0,
                'text' => '',
                'make_link' => 0,
                'path' => '',
                'link_class' => '',
                'alt' => '',
                'prefix' => '',
                'suffix' => '',
                'target' => '',
                'help' => '',
                'trim' => 0,
                'max_length' => '',
                'word_boundary' => 1,
                'ellipsis' => 1,
                'html' => 0,
                'strip_tags' => 0,
            ),
            'empty' => '',
            'hide_empty' => 0,
            'empty_zero' => 0,
            'link_to_node' => 0,
            'exclude' => 0,
            'id' => 'title',
            'table' => 'node',
            'field' => 'title',
            'relationship' => 'none',
            'override' => array(
                'button' => 'Override',
            ),
        ),
        'phpcode_2' => array(
            'label' => 'Action',
            'alter' => array(
                'alter_text' => 0,
                'text' => '',
                'make_link' => 0,
                'path' => '',
                'absolute' => 0,
                'link_class' => '',
                'alt' => '',
                'rel' => '',
                'prefix' => '',
                'suffix' => '',
                'target' => '',
                'help' => '',
                'trim' => 0,
                'max_length' => '',
                'word_boundary' => 1,
                'ellipsis' => 1,
                'html' => 0,
                'strip_tags' => 0,
            ),
            'empty' => '',
            'hide_empty' => 0,
            'empty_zero' => 0,
            'hide_alter_empty' => 1,
            'value' => '
        <?php
          global $_tow_dataset;
          global $user;
          $arg = arg(1);
          if ($_tow_dataset->uid == $user->uid) {
            $output = l(\'edit\', \'node/\' . $data->nid . \'/edit\', array(\'query\' => \'destination=dataset/\' . $arg)) . \'   \';
            $output .= l(\'delete\', \'node/\' . $data->nid . \'/delete\', array(\'query\' => \'destination=dataset/\' . $arg)) . \'   \';
          }
          else {
            $output = l(\'structure\', \'node/\' . $data->nid) . \'   \';
          }
          $output .= l(\'browse\', \'table/\' . $data->nid);
          print $output;
        ?>
      ',
            'exclude' => 1,
            'id' => 'phpcode_2',
            'table' => 'customfield',
            'field' => 'phpcode',
            'override' => array(
                'button' => 'Override',
            ),
            'relationship' => 'none',
        ),
        'nid' => array(
            'label' => 'Action',
            'alter' => array(
                'alter_text' => 1,
                'text' => '[phpcode_2]',
                'make_link' => 0,
                'path' => '',
                'absolute' => 0,
                'link_class' => '',
                'alt' => '',
                'rel' => '',
                'prefix' => '',
                'suffix' => '',
                'target' => '',
                'help' => '',
                'trim' => 0,
                'max_length' => '',
                'word_boundary' => 1,
                'ellipsis' => 1,
                'html' => 0,
                'strip_tags' => 0,
            ),
            'empty' => '',
            'hide_empty' => 0,
            'empty_zero' => 0,
            'hide_alter_empty' => 1,
            'link_to_node' => 0,
            'exclude' => 0,
            'id' => 'nid',
            'table' => 'node',
            'field' => 'nid',
            'relationship' => 'none',
        ),
        'phpcode' => array(
            'label' => 'Records',
            'alter' => array(
                'alter_text' => 0,
                'text' => '',
                'make_link' => 0,
                'path' => '',
                'link_class' => '',
                'alt' => '',
                'prefix' => '',
                'suffix' => '',
                'target' => '',
                'help' => '',
                'trim' => 0,
                'max_length' => '',
                'word_boundary' => 1,
                'ellipsis' => 1,
                'html' => 0,
                'strip_tags' => 0,
            ),
            'empty' => '',
            'hide_empty' => 0,
            'empty_zero' => 0,
            'value' => '
        <?php
          print db_result(db_query(\'SELECT COUNT(*) FROM {content_type_record} WHERE field_table_value = %d\', $data->nid));
        ?>
      ',
            'exclude' => 0,
            'id' => 'phpcode',
            'table' => 'customfield',
            'field' => 'phpcode',
            'override' => array(
                'button' => 'Override',
            ),
            'relationship' => 'none',
        ),
        'body' => array(
            'label' => 'Description',
            'alter' => array(
                'alter_text' => 0,
                'text' => '',
                'make_link' => 0,
                'path' => '',
                'absolute' => 0,
                'link_class' => '',
                'alt' => '',
                'rel' => '',
                'prefix' => '',
                'suffix' => '',
                'target' => '',
                'help' => '',
                'trim' => 0,
                'max_length' => '',
                'word_boundary' => 1,
                'ellipsis' => 1,
                'html' => 0,
                'strip_tags' => 0,
            ),
            'empty' => '',
            'hide_empty' => 0,
            'empty_zero' => 0,
            'hide_alter_empty' => 1,
            'exclude' => 0,
            'id' => 'body',
            'table' => 'node_revisions',
            'field' => 'body',
            'override' => array(
                'button' => 'Override',
            ),
            'relationship' => 'none',
        ),
    ));
    $handler->override_option('sorts', array(
        'title' => array(
            'order' => 'ASC',
            'id' => 'title',
            'table' => 'node',
            'field' => 'title',
            'relationship' => 'none',
        ),
    ));
    $handler->override_option('arguments', array(
        'nid' => array(
            'default_action' => 'not found',
            'style_plugin' => 'default_summary',
            'style_options' => array(),
            'wildcard' => 'all',
            'wildcard_substitution' => 'All',
            'title' => '',
            'breadcrumb' => '',
            'default_argument_type' => 'fixed',
            'default_argument' => '',
            'validate_type' => 'none',
            'validate_fail' => 'not found',
            'break_phrase' => 0,
            'not' => 0,
            'id' => 'nid',
            'table' => 'node',
            'field' => 'nid',
            'validate_user_argument_type' => 'uid',
            'validate_user_roles' => array(
                2 => 0,
            ),
            'override' => array(
                'button' => 'Override',
            ),
            'relationship' => 'field_dataset_nid',
            'default_options_div_prefix' => '',
            'default_argument_fixed' => '',
            'default_argument_user' => 0,
            'default_argument_php' => '',
            'validate_argument_node_type' => array(
                'dataset' => 0,
                'page' => 0,
                'record' => 0,
                'story' => 0,
                'table' => 0,
            ),
            'validate_argument_node_access' => 0,
            'validate_argument_nid_type' => 'nid',
            'validate_argument_vocabulary' => array(),
            'validate_argument_type' => 'tid',
            'validate_argument_transform' => 0,
            'validate_user_restrict_roles' => 0,
            'validate_argument_php' => '',
        ),
    ));
    $handler->override_option('filters', array(
        'type' => array(
            'operator' => 'in',
            'value' => array(
                'table' => 'table',
            ),
            'group' => '0',
            'exposed' => FALSE,
            'expose' => array(
                'operator' => FALSE,
                'label' => '',
            ),
            'id' => 'type',
            'table' => 'node',
            'field' => 'type',
            'relationship' => 'none',
        ),
        'status' => array(
            'operator' => '=',
            'value' => '1',
            'group' => '0',
            'exposed' => FALSE,
            'expose' => array(
                'operator' => FALSE,
                'label' => '',
            ),
            'id' => 'status',
            'table' => 'node',
            'field' => 'status',
            'relationship' => 'none',
        ),
    ));
    $handler->override_option('access', array(
        'type' => 'none',
    ));
    $handler->override_option('cache', array(
        'type' => 'none',
    ));
    $handler->override_option('items_per_page', 0);
    $handler->override_option('style_plugin', 'datatables');
    $handler->override_option('style_options', array(
        'grouping' => '',
        'order' => 'asc',
        'summary' => '',
        'columns' => array(
            'title' => 'title',
            'phpcode_2' => 'phpcode_2',
            'nid' => 'nid',
            'phpcode' => 'phpcode',
            'phpcode_1' => 'phpcode_1',
            'field_units_value' => 'field_units_value',
        ),
        'info' => array(
            'title' => array(
                'sortable' => 1,
                'separator' => '',
            ),
            'phpcode_2' => array(
                'separator' => '',
            ),
            'nid' => array(
                'sortable' => 0,
                'separator' => '',
            ),
            'phpcode' => array(
                'separator' => '',
            ),
            'phpcode_1' => array(
                'separator' => '',
            ),
            'field_units_value' => array(
                'sortable' => 1,
                'separator' => '',
            ),
        ),
        'default' => 'title',
        'search_box' => 1,
        'table_info' => 1,
    ));
    $handler = $view->new_display('page', 'Page', 'page_1');
    $handler->override_option('style_plugin', 'table');
    $handler->override_option('style_options', array(
        'grouping' => '',
        'override' => 1,
        'sticky' => 0,
        'order' => 'asc',
        'summary' => '',
        'columns' => array(
            'title' => 'title',
            'phpcode_2' => 'phpcode_2',
            'phpcode' => 'phpcode',
            'phpcode_1' => 'phpcode_1',
            'body' => 'body',
        ),
        'info' => array(
            'title' => array(
                'sortable' => 0,
                'separator' => '',
            ),
            'phpcode_2' => array(
                'separator' => '',
            ),
            'phpcode' => array(
                'separator' => '',
            ),
            'phpcode_1' => array(
                'separator' => '',
            ),
            'body' => array(
                'separator' => '',
            ),
        ),
        'default' => '-1',
    ));
    $handler->override_option('path', 'dataset');
    $handler->override_option('menu', array(
        'type' => 'none',
        'title' => 'dataset',
        'description' => '',
        'weight' => '-10',
        'name' => 'navigation',
    ));
    $handler->override_option('tab_options', array(
        'type' => 'none',
        'title' => '',
        'description' => '',
        'weight' => 0,
        'name' => 'navigation',
    ));

    return $view;
}
