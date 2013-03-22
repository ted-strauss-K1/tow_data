<?php
/*
* Drupal Status Messages
*/
function fusion_theory_status_messages($display = NULL) {
  $noty = array();
  $noty_pattern = "$.hrd.noty({'text':'%s','type':'%s'});";
  foreach (drupal_get_messages($display) as $type => $messages) {
    $noty_type = false;
    switch ($type) {
      case 'status' :
        $noty_type = 'success';
      break;
      case 'warning' :
        $noty_type = 'warning';
      break;
      case 'error' :
        $noty_type = 'error';
      break;
    }
    if ($noty_type !== false) {
      foreach ($messages as $message) {
        $noty[] = sprintf($noty_pattern, addslashes($message), $noty_type);
      }
    }
  }
  drupal_add_js('$(document).ready(function(){'.implode('',$noty).'});', 'inline');
  return false;
}

function fusion_theory_preprocess_page(&$vars) { 
  // Add per content type pages
  if (isset($vars['node'])) {
    // Add template naming suggestion. It should alway use hyphens.
    // If node type is "custom_news", it will pickup "page-custom-news.tpl.php".
    // Use page-contenttype.tpl.php for edit form.
    // Use page-node-add-contenttype.tpl.php for add-a-new-node form.
    $vars['template_files'][] = 'page-'. str_replace('_', '-', $vars['node']->type);
  }
}

function fusion_theory_links($links, $attributes = array('class' => '')) {
  global $language;
  $output = '';
  
  if (count($links) > 0) {
  	dpm($links);
	  
    $output = '<div'. drupal_attributes($attributes) .'>';

    $num_links = count($links);
    $i = 1;

    foreach ($links as $key => $link) {
      $class = $key;

      // Add first, last and active classes to the list of links to help out themers.
      if ($i == 1) {
        $class .= ' first';
      }
      if ($i == $num_links) {
        $class .= ' last';
      }
      if (isset($link['href']) && ($link['href'] == $_GET['q'] || ($link['href'] == '<front>' && drupal_is_front_page()))
          && (empty($link['language']) || $link['language']->language == $language->language)) {
        $class .= ' active';
      }
      $output .= '<div class="btn btn-small">'; //. drupal_attributes(array('class' => $class)) .'>';

      if (isset($link['href'])) {
        // Pass in $link as $options, they share the same keys.
        $output .= l($link['title'], $link['href'], $link);
      }
      else if (!empty($link['title'])) {
        // Some links are actually not links, but we wrap these in <span> for adding title and class attributes
        if (empty($link['html'])) {
          $link['title'] = check_plain($link['title']);
        }
        $span_attributes = '';
        if (isset($link['attributes'])) {
          $span_attributes = drupal_attributes($link['attributes']);
        }
        $output .= '<span'. $span_attributes .'>'. $link['title'] .'</span>';
      }

      $i++;
      $output .= "</div>\n";
    }

    $output .= '</div>';
  }

  return $output;
}

function fusion_theory_preprocess_node(&$variables) {
  $node = $variables['node'];
  $variables['terms']     = theme('links', $variables['taxonomy'], array('class' => 'tags_btns'));
}

/**
 * Format the 'submitted' output for comments
 */
function fusion_theory_comment_submitted($comment) {
    
    return t('— !username !datetime', 
    array(
    '!username' => theme('username', $comment), 
    '!datetime' => format_interval(time() - $comment->timestamp, 1) . ' ' . t('ago'),
  ));

}

function fusion_theory_menu_item_link($link) {
  // Local tasks for view and edit nodes shouldn't be displayed.
  if ($link['type'] & MENU_LOCAL_TASK && 
  	  ($link['path'] === 'node/%/edit' || 
  	   $link['path'] === 'node/%/view' || 
  	   $link['path'] === 'node/%/vud-votes' || 
  	   $link['path'] === 'node/%/track' || 
  	   $link['path'] === 'node/%/devel')) {
    return '';
  }
  else {
    if (empty($link['localized_options'])) {
      $link['localized_options'] = array();
    }

    return l($link['title'], $link['href'], $link['localized_options']);
  }
}

function fusion_theory_menu_local_task($link, $active = FALSE) {
  // Don't return a <li> element if $link is empty
  if ($link === '') {
    return '';
  }
  else {
    return '<li '. ($active ? 'class="active" ' : '') .'>'. $link ."</li>\n";  
  }
}

?>
