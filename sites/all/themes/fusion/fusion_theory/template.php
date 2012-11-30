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


?>
