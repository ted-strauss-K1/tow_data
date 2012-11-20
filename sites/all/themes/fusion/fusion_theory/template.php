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

?>
