<?php
/**
 * Redirects to the page with results. Final URL contains search string and 
 * additional filter: search in records only
 */
function tow_search_within_dataset_submit_simple($form, &$form_state) {
  //x('submit simple search called');
 
  $args = implode($_GET);
  
  // Submit may be called from two types of URL: 
  // search/apachesolr_search/test?filters=type:record,sis_cck_field_dataset_nid:[dataset-nid] and
  // search_dataset/[dataset-nid]
  // The following lines extract dataset-nid from both URLs
  
  $matches_qty = preg_match('/(?<=dataset_nid:|search_dataset\/)\d{1,}$/', $args, $dataset_nids);
	//x($dataset_nids);
	if ($matches_qty) {
		$dataset_nid = $dataset_nids[0];
	}
	else {
	// TODO: GOTO 404 page 
	}
  
  // tell apachesolr to perform search
  $path = 'search/apachesolr_search/' . $form_state['clicked_button']['#post']['simple_search']['textfield'];
    
  // search in records only
  $query = 'filters=type:record';
  
  // limit search to the specified dataset only, separate search option by space
  $query .= ' ' . 'sis_tow_dataset_nid:' . $dataset_nid;   
  drupal_goto($path, $query);

}

function tow_search_within_dataset_submit($form, &$form_state) {
 x('success 2');
}