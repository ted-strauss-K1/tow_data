<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function tow_menu_users_feature_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: menu-users:node/add/dataset
  $menu_links['menu-users:node/add/dataset'] = array(
    'menu_name' => 'menu-users',
    'link_path' => 'node/add/dataset',
    'router_path' => 'node/add/dataset',
    'link_title' => 'New dataset',
    'options' => array(
      'attributes' => array(),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-45',
  );
  // Exported menu link: menu-users:user/login
  $menu_links['menu-users:user/login'] = array(
    'menu_name' => 'menu-users',
    'link_path' => 'user/login',
    'router_path' => 'user/login',
    'link_title' => 'New dataset',
    'options' => array(
      'attributes' => array(
        'class' => 'automodal',
      ),
      'query' => 'automodalReload=true',
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-44',
  );
  // Exported menu link: menu-users:users/moderators
  $menu_links['menu-users:users/moderators'] = array(
    'menu_name' => 'menu-users',
    'link_path' => 'users/moderators',
    'router_path' => 'users/moderators',
    'link_title' => 'Moderators',
    'options' => array(
      'attributes' => array(),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-47',
  );
  // Exported menu link: menu-users:users/new_users
  $menu_links['menu-users:users/new_users'] = array(
    'menu_name' => 'menu-users',
    'link_path' => 'users/new_users',
    'router_path' => 'users/new_users',
    'link_title' => 'New Users',
    'options' => array(
      'attributes' => array(),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-50',
  );
  // Exported menu link: menu-users:users/reputation
  $menu_links['menu-users:users/reputation'] = array(
    'menu_name' => 'menu-users',
    'link_path' => 'users/reputation',
    'router_path' => 'users/reputation',
    'link_title' => 'Reputation',
    'options' => array(
      'attributes' => array(),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-46',
  );
  // Exported menu link: menu-users:users/searchers
  $menu_links['menu-users:users/searchers'] = array(
    'menu_name' => 'menu-users',
    'link_path' => 'users/searchers',
    'router_path' => 'users/searchers',
    'link_title' => 'Searchers',
    'options' => array(
      'attributes' => array(),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-49',
  );
  // Exported menu link: menu-users:users/voters
  $menu_links['menu-users:users/voters'] = array(
    'menu_name' => 'menu-users',
    'link_path' => 'users/voters',
    'router_path' => 'users/voters',
    'link_title' => 'Voters',
    'options' => array(
      'attributes' => array(),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-48',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Moderators');
  t('New Users');
  t('New dataset');
  t('Reputation');
  t('Searchers');
  t('Voters');


  return $menu_links;
}
