<?php

function tow_tests_fk() {
  return drupal_get_form('tow_tests_fk_form');  
}

function tow_tests_fk_form() {
  
  $form['fk'] = array(
    '#description' => 'Use foreign keys',
    '#type' => 'checkbox'
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Ok'
  );
  
  return $form;
}

function tow_tests_fk_form_submit($form, &$form_state) {
  
  $fields = tow_possible_fields();
  foreach ($fields as $field) {
    $tables[] = 'content_field_' . $field;
    $tables[] = 'content_field_title_' . $field;
  }
  
  $query = "ALTER TABLE %s ";
   
  if ($form_state['values']['fk']) {
    $query .= "ADD CONSTRAINT `%s_ibfk_1` FOREIGN KEY (`nid`) REFERENCES {node} (`nid`)";
    foreach ($tables as $table)
      db_query($query, $table, $table);
    db_query("
      ALTER TABLE {content_type_record}
        ADD CONSTRAINT `content_type_record_ibfk_1` FOREIGN KEY (`field_table_nid`) REFERENCES {node} (`nid`)
      ");
    db_query("
      ALTER TABLE {content_type_record}
        ADD CONSTRAINT `content_type_record_ibfk_2` FOREIGN KEY (`nid`) REFERENCES {node} (`nid`)
      ");
    db_query("
      ALTER TABLE {content_type_table}
        ADD CONSTRAINT `content_type_table_ibfk_1` FOREIGN KEY (`field_dataset_nid`) REFERENCES {node} (`nid`)
      ");
    db_query("
      ALTER TABLE {content_type_table}
        ADD CONSTRAINT `content_type_table_ibfk_2` FOREIGN KEY (`nid`) REFERENCES {node} (`nid`)
      ");
    db_query("
      ALTER TABLE {content_type_dataset}
        ADD CONSTRAINT `content_type_dataset_ibfk_1` FOREIGN KEY (`nid`) REFERENCES {node} (`nid`)
      ");
    db_query("
      ALTER TABLE {node}
        ADD CONSTRAINT `node_ibfk_1` FOREIGN KEY (`uid`) REFERENCES {users} (`uid`)
      "); 
  }else{
    $tables[] = 'content_type_record';
    $tables[] = 'content_type_table';
    $tables[] = 'content_type_dataset';
    $query .= "DROP FOREIGN KEY `%s_ibfk_1`";
    foreach ($tables as $table)
      db_query($query, $table, $table);
    db_query("ALTER TABLE {content_type_table}
      DROP FOREIGN KEY `content_type_table_ibfk_2`");
    db_query("ALTER TABLE {content_type_table}
      DROP FOREIGN KEY `content_type_record_ibfk_2`");
    db_query("ALTER TABLE {node}
      DROP FOREIGN KEY `node_ibfk_1`");
  }
}