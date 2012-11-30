<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function tow_menu_tags_feature_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: menu-tags:tags/name
  $menu_links['menu-tags:tags/name'] = array(
    'menu_name' => 'menu-tags',
    'link_path' => 'tags/name',
    'router_path' => 'tags/name',
    'link_title' => 'Name',
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
    'weight' => '-49',
  );
  // Exported menu link: menu-tags:tags/new
  $menu_links['menu-tags:tags/new'] = array(
    'menu_name' => 'menu-tags',
    'link_path' => 'tags/new',
    'router_path' => 'tags/new',
    'link_title' => 'New',
    'options' => array(
      'attributes' => array(
        'title' => 'All tags sorted by date that tag was first added to a dataset.',
      ),
    ),
    'module' => 'system',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-48',
  );
  // Exported menu link: menu-tags:tags/popular
  $menu_links['menu-tags:tags/popular'] = array(
    'menu_name' => 'menu-tags',
    'link_path' => 'tags/popular',
    'router_path' => 'tags/popular',
    'link_title' => 'Popular',
    'options' => array(
      'attributes' => array(
        'title' => 'All tags sorted by usage frequency descending.',
      ),
    ),
    'module' => 'system',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-50',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Name');
  t('New');
  t('Popular');


  return $menu_links;
}
