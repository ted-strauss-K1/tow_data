<?php
/**
 * Construct a query for record data extraction from the database.
 * @param $node node with data
 * $display_options ('sort' => ,
                     'filter' => ,
                     'limits' => );                     
 *
 * @Return array(
 *		'query' => "query string, contains replacement operators,
 *      'arguments' => "array of arguments for db_query",
		'titles' => "array of column titles");
 *  
 * An example of query string may be found below.
 */

//function tow_construct_query($node_id, $sort_index = 'nid', $sort_order = "ASC", $first_elem_no = 0, $total_elements = 30) {
function tow_construct_table_query($node, $display_options = '') {

// 1.0 Read Data from node nid (node may be also passed as input variable).
//	$current_table = node_load($node_id, NULL, FALSE);
	$current_table = $node;
	$argmts[] = $current_table->nid;
// 1.1 weights responsible for the column order
	$weights_array = unserialize($current_table->field_weights[0]['value']);

// 1.2 Preparing auxiliary array with all data for query construction.
	$index = 0;
//	$aux_array = array();
	foreach (tow_possible_fields() as $field_type) {
		$delta = 0;
		if (!is_array($current_table->{'field_title_' . $field_type}))
      continue;
    foreach($current_table->{'field_title_' . $field_type} as $title_value) {
      if ($title_value['value']) {
        $aux_array[$weights_array[$index]]['title'] = $title_value['value'];
				$aux_array[$weights_array[$index]]['delta'] = $delta;
				$aux_array[$weights_array[$index]]['type'] = $field_type;
				$aux_array[$weights_array[$index]]['alias'] = "`$field_type" . "$delta`";
				
				$index++;
				$delta++;
			}
		}
	}
	if ($aux_array)
		ksort($aux_array);
	
// 2. Query construction 
// 	Query is constructed of two parts. Both parts are recorded in one foreach cycle, and then concatenated.

// 2.1 first part of query (starting from SELECT)
// "SELECT `rec`.nid AS `nid`,"
  if ($aux_array) {
		$query .= "SELECT " . "`rec`" . ".nid AS `nid`,\n";
	
 
// 2.2 second part of query (starting from FROM)
	$query_part2 = "FROM {content_type_record} `rec`\n";

// 3. Main cycle for data selection

/* 	$column_index is responsible for the actual column numeration, from 0 to maximum column number. 
	We use it because $index in $aux_array may contain any values depending on  a) filter settings, 
	b) values returned by tow_possible_fields(), c) etc.
	
	In action 5.2 $column_index will be used to check the validity of $sort_index, where $sort_index is
	the index of the column, which values will be used for sorting. $sort_index shall be less or equal 
	to the number of columns, counted from zero.
*/	
	$column_index = 0;

	foreach ($aux_array as $index => $column_info) {
	
	// first part of the query:
	//	"`int0`.field_int_value AS `0`, `char0`.field_char_value AS `1`, `char1`.field_char_value AS `2`, `int1`.field_int_value AS `3`, `char2`.field_char_value AS `4`, `text0`.field_text_value AS `5`"
		$query = $query . $column_info['alias'] . ".field_" . $column_info['type'] . "_value AS `" . $column_index . "`, ";
	
	// second part:
	// starting from FROM, includes all LEFT JOIN
	//	if ($column_index) {
		
			$query_part2 = $query_part2 . "LEFT JOIN {content_field_" . 
				$column_info['type'] . "} " . $column_info['alias'] . "\n";
			
      $query_part2 = $query_part2 . "\tON " . $column_info['alias'] .
				".nid = " . '`rec`' . ".nid\n";
			
      $query_part2 = $query_part2 . "\tAND " . $column_info['alias'] .
				".delta = " . $column_info['delta'] . "\n";
		
/*			if ($column_index == 1) {
				$query_part2 = $query_part2 . "\tAND " . $aux_array[0]['alias'] .
					".delta = " .  $aux_array[0]['delta'] . "\n";
			}
		} else {
			$query_part2 = $query_part2 . "{content_field_" . 
				$column_info['type'] . "} " . $column_info['alias'] . "\n";*/
//	  }
		
	// part of query, responsible for filtering
		if (($display_options['filter']) && ($display_options['filter']['filter_column_index'] == $column_index)) {
			$filter_query_part = "AND " . $column_info['alias'] . ".field_" . $column_info['type']. "_value ";    
      
      $filter_query_part = $filter_query_part . "%s" . " ";
      $argmts[] = tow_table_filter_translate_operator($display_options['filter']['filter_operator_key']);
      
      if (is_string($display_options['filter']['filter_value'])) {
        $filter_query_part = $filter_query_part . "'%s' \n"; 
      } else {
        $filter_query_part = $filter_query_part . "%d \n"; 
      }
      $argmts[] = $display_options['filter']['filter_value'];
    }
		
		// recording titles into $result to be returned from the function
		$result['titles']['values'][$column_index] = $column_info['title'];
		$result['titles']['types'][$column_index] = $column_info['type'];
    
		$column_index++;
	
	
	}
// Correction total number of columns 
	$column_index--;
// Correction of last symbols in the first part of the query
	$query = rtrim($query, ", ");	
	$query .= "\n";
	
// 4. Concatenation fo two query parts (SELECT and FROM parts)
	$query = $query . $query_part2;

// 5. Add WHERE part 
/* 	WHERE `rec`.nid
	IN (
		SELECT nid
		FROM {content_type_record}
		WHERE field_table_nid =  %d
	)
*/
	//	5.2 Choose only records included in this table ( field_table_nid == table_nid )
	$query = $query . "WHERE " . "`rec`" . ".nid\n IN (\n ";
  }
	$query .= "SELECT nid\n FROM {content_type_record}\n WHERE field_table_nid =  %d";
  if ($aux_array) {	
	$query .="\n)\n";

//	5.2 Filter results
	if ($display_options['filter']) {
		$query .= $filter_query_part;
	}
  }
// 6. Add ORDER  part
// updated: ORDER BY `%s` %s
// old variant: ORDER BY `nid` DESC
  
// 6.1
	 $query = $query . "ORDER BY `";
// 6.2
// 6.2.0 
   $query = $query . "%s";
  
// 6.2.1 check if sort index is correct (not larger than the number of columns)
   if (!($display_options['sort']) || ($display_options['sort']['sort_index'] > $column_index)) { 
     $argmts[] = 'nid';
   } else {
     $argmts[] = (string)$display_options['sort']['sort_index'];
   }
  
// 6.3
    $query = $query . "` " . "%s" . "\n";

    if (!isset($display_options['sort']['sort_order'])) {  
      $display_options['sort']['sort_order'] = "DESC";
    }
    
    $argmts[] = $display_options['sort']['sort_order'];
    
// 7. Add LIMIT part 
// updated: LIMIT %n , %n
// old variant: LIMIT 0 , 100
	
  if (!($display_options['paged_view']) && ($display_options['limits'])) {
    $query = $query . "LIMIT " . "%d" 			. " , " . "%d";
//old:	$query = $query . "LIMIT " . $first_elem_no . " , " . $total_elements;
    $argmts[] = $display_options['limits']['first_elem_no'];
    $argmts[] = $display_options['limits']['total_elements'];
  }
  
// 8. Output
	if(!$result['titles']['values'])
		$result['titles']['values']=array();
	$result['query'] = $query;
	$result['arguments'] = $argmts;
  return $result;

/* The whole query (example):

SELECT `rec`.nid AS `nid`,
`int0`.field_int_value AS `0`, `char0`.field_char_value AS `1`, `char1`.field_char_value AS `2`, `int1`.field_int_value AS `3`, `char2`.field_char_value AS `4`, `text0`.field_text_value AS `5`
FROM content_field_int `int0`
LEFT JOIN {content_field_char} `char0`
	ON `char0`.nid = `int0`.nid
	AND `char0`.delta = 0
	AND `int0`.delta = 0
LEFT JOIN {content_field_char} `char1`
	ON `char1`.nid = `int0`.nid
	AND `char1`.delta = 1
LEFT JOIN {content_field_int} `int1`
	ON `int1`.nid = `int0`.nid
	AND `int1`.delta = 1
LEFT JOIN {content_field_char} `char2`
	ON `char2`.nid = `int0`.nid
	AND `char2`.delta = 2
LEFT JOIN {content_field_text} `text0`
	ON `text0`.nid = `int0`.nid
	AND `text0`.delta = 0
WHERE `rec`.nid
IN (
 SELECT nid
 FROM {content_type_record}
 WHERE field_table_nid = %d
) 

AND `int0`.field_int_value > 5

ORDER BY `%s` %s
LIMIT %d , %d
*/
}