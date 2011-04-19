<?php
// $Id: tow_statistics.inc,v 1.0.0.0 2011/02/28 23:42:15 tow Exp $
/**
 * @file
 * Functions for fetching and updating dataset and table statistics. 
 */

/**
 * Prepare global variable with dataset statistics
 */
function tow_statistics_dataset_extract($nid) {  
  global $dataset_statistics;
  $dataset_statistics = db_fetch_array(db_query("SELECT count, timestamp FROM {tow_dataset_counter} WHERE nid = %d LIMIT 1", $nid));
}

/**
 * If current user is not an owner of dataset node, store data about page view in tow_dataset_counter table
 */
function tow_statistics_dataset_update($node) {
  // dataset owner should not affect statistics of his fatasets
  global $user;
  if ($user->uid == $node->uid) {
    return;
  }
    
  $time = time();
  db_query("
    INSERT INTO {tow_dataset_counter} (nid, count, timestamp)
    VALUES (%d, 1, %d)
    ON DUPLICATE KEY UPDATE count = count+1, timestamp = %d
    ", $node->nid, $time, $time);
}

/**
 * Prepare global variable with table statistics
 */
function tow_statistics_table_extract($nid) {  
  global $table_statistics;
  $table_statistics = db_result(db_query("SELECT count FROM {tow_table_counter} WHERE nid = %d LIMIT 1", $nid));
}

/**
 * If current user is not an owner of table node, store data about page view in tow_table_counter table
 */
function tow_statistics_table_update($node) {
  // table owner should not affect statistics of his fatasets
  global $user;
  if ($user->uid == $node->uid) {
    return;
  }

  db_query("
    INSERT INTO {tow_table_counter} (nid, count)
    VALUES (%d, 1)
    ON DUPLICATE KEY UPDATE count = count+1
    ", $node->nid);
}