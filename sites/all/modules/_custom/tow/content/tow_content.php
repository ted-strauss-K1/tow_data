<?php

/**
 * This function is for auto insertion of topics
 * Run it one time after term_data term_hierarchy tables are truncated
 */
function tow_content_insert_topics() {
  
  $file = file_get_contents(drupal_get_path('module', 'tow') . '/content/taxonomy_of_topics.txt');
  $terms = explode("\r", $file);
  
  foreach($terms as $term) {
  	$tid++;
  	db_query("INSERT INTO {term_data} VALUES (NULL, 2, '%s', NULL, 0)", $term);
  	db_query("INSERT INTO {term_hierarchy} (tid, parent) VALUES (%d, 0)", $tid);
  }
}