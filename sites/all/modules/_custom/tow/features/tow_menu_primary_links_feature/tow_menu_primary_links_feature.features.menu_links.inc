<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function tow_menu_primary_links_feature_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: primary-links:<front>
  $menu_links['primary-links:<front>'] = array(
    'menu_name' => 'primary-links',
    'link_path' => '<front>',
    'router_path' => '',
    'link_title' => 'Data',
    'options' => array(
      'attributes' => array(),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '1',
    'has_children' => '0',
    'expanded' => '1',
    'weight' => '0',
  );
  // Exported menu link: primary-links:tags
  $menu_links['primary-links:tags'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'tags',
    'router_path' => 'tags',
    'link_title' => 'Tags',
    'options' => array(
      'attributes' => array(
        'title' => 'All tags sorted in an alphabetical order.',
      ),
    ),
    'module' => 'system',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '0',
  );
  // Exported menu link: primary-links:users
  $menu_links['primary-links:users'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'users',
    'router_path' => 'users',
    'link_title' => 'Users',
    'options' => array(
      'attributes' => array(
        'title' => 'Users',
      ),
    ),
    'module' => 'system',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '0',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Data');
  t('Tags');
  t('Users');


  return $menu_links;
}
