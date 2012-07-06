<?php

/**
 * Calculate total size of user's records. (BIT_LENGTH of field_char_value, field_int_value etc.)
 * The query is:

SELECT SUM(BIT_LENGTH(field_int_value)) + SUM(BIT_LENGTH(field_char_value)) + ... + SUM(BIT_LENGTH(field_code_value)) AS sum
FROM node n INNER JOIN content_type_record r
  ON n.nid = r.nid
INNER JOIN content_field_int f_int     ON r.nid = f_int.nid
INNER JOIN content_field_char f_char   ON r.nid = f_char.nid
........................................................
INNER JOIN content_field_code f_code   ON r.nid = f_code.nid
WHERE n.uid = 1
 
 */

function tow_size_all_records($uid) {

  $select = "SELECT";
  $from = "FROM node n INNER JOIN content_type_record r ON n.nid = r.nid\n";
  
  foreach (tow_possible_fields() as $field) {
    if ($field == 'datetime' || $field == 'timestamp')
      continue;
    $select .= " SUM(BIT_LENGTH(field_" . $field . "_value)) +";
    $from .= "INNER JOIN content_field_" . $field . " f_" . $field .  " ON r.nid = f_" . $field . ".nid\n"; 
  }

  $query = rtrim($select, '+') . "AS sum ". $from . "WHERE n.uid = %d";

  return db_result(db_query($query, $uid))/8;
}