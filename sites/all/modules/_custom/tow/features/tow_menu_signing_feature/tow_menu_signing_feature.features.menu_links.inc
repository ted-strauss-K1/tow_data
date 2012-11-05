<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function tow_menu_signing_feature_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: menu-signing:<front>
  $menu_links['menu-signing:<front>'] = array(
    'menu_name' => 'menu-signing',
    'link_path' => '<front>',
    'router_path' => '',
    'link_title' => '[realname] [token_custom_points]',
    'options' => array(
      'attributes' => array(),
      'alter' => TRUE,
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '1',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-50',
  );
  // Exported menu link: menu-signing:logout
  $menu_links['menu-signing:logout'] = array(
    'menu_name' => 'menu-signing',
    'link_path' => 'logout',
    'router_path' => 'logout',
    'link_title' => 'sign out',
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
  // Exported menu link: menu-signing:user/login
  $menu_links['menu-signing:user/login'] = array(
    'menu_name' => 'menu-signing',
    'link_path' => 'user/login',
    'router_path' => 'user/login',
    'link_title' => 'login',
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
    'weight' => '-49',
  );
  // Exported menu link: menu-signing:user/register
  $menu_links['menu-signing:user/register'] = array(
    'menu_name' => 'menu-signing',
    'link_path' => 'user/register',
    'router_path' => 'user/register',
    'link_title' => 'sign up',
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
    'weight' => '-48',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('[realname] [token_custom_points]');
  t('login');
  t('sign out');
  t('sign up');


  return $menu_links;
}
