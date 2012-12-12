<?php

/**
 * @file
 *
 * Primary menu representation.
 */
/* * **************************************************************************************************
 * Hook handlers
 * ************************************************************************************************** */

/**
 * Returns block containing menu links.
 */
function tow_pma_menu_block() {

    global $_tow_dataset;
    global $_tow_table;
    global $_tow_grey_pma_menu;
    global $user;

    // Stop if no dataset is set.
    if (!$_tow_dataset) {
        return;
    }

    // Define browse submenu items.
    // Table nids and titles
    $table_titles_res = db_query("

    SELECT
      t.nid AS nid,
      n.title AS title

      FROM
          {content_type_table} t
        LEFT JOIN
          {node} n
          ON
            t.nid = n.nid

      WHERE
        t.field_dataset_value = %d

  ", $_tow_dataset->nid);

    $subitems = '';
    while ($rs = db_fetch_array($table_titles_res)) {
        $subitems .= '<li>' . l($rs['title'], 'table/' . $rs['nid']) . '</li>';
    }
    
    //Dataset access type
    $access_type = $_tow_dataset->field_access_type[0]['value'];
    //User access to dataset with type: By request
    $result = db_query("

    SELECT
      tar.nid,
      tar.uid,
      tar.status

      FROM
        {tow_access_requests} tar
      
      WHERE
        tar.uid = %d
        
      AND
        tar.nid = %d
    ", $user->uid, $_tow_dataset->nid);

    $dataset_request = db_fetch_array($result);

    // Define menu items.
    $browse_has_submenu = ($subitems) ? 'browse-has-submenu':NULL;
    $items = array();
    $items['browse'] = /*($_tow_grey_pma_menu) ? t('Browse') :  (($_tow_table) ? l('Browse', 'table/' . $_tow_table->nid) : */l('Browse', 'node/' . $_tow_dataset->nid, $options = array('attributes'=>array('class'=>$browse_has_submenu)))/* ) */;
    $items['browse'] .= ($subitems) ? ('<ul class="browse-submenu">' . $subitems . '</ul>'):NULL;
    $items['search'] = /*($_tow_grey_pma_menu) ? t('Search') : */l('Search', 'search_dataset/' . $_tow_dataset->nid);
    $res_doc = db_query("SELECT t.nid FROM {content_type_document} t WHERE t.field_doc_dataset_nid_value = %d", $_tow_dataset->nid);
    $dataset_doc = db_fetch_array($res_doc);
    $items['docs'] = (!$dataset_doc) ? l('Docs', 'node/add/document', array('query' => 'dataset=' . $_tow_dataset->nid)) : l('Docs', 'doc/' . $_tow_dataset->nid);
    $items['new_dataset'] = ($user->uid) ? l('Create new dataset', 'node/add/dataset') : l('Create new dataset', 'user/login', array('query' => 'automodalReload=true', 'attributes' => array('class' => 'automodal')));

    //Set menu items according to user roles
    //Dataset access type: By request
    if ($access_type == 1 && $dataset_request['status'] != 1 && ($_tow_dataset->uid != $user->uid)) {
        $items['browse'] = l('Browse', 'node/' . $_tow_dataset->nid);
        unset($items['search']);
        //Anonymous
        if (!$user->uid) {
            unset($items['docs']);
        }
    }
    
    // Theme the block.
    $block['content'] = theme_item_list($items, NULL, 'ul');

    // Set breadcrumbs.
    tow_pma_menu_breadcrumbs();

    return $block;
}

/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Set breadcrumbs.
 */
function tow_pma_menu_breadcrumbs() {

    global $_tow_dataset;
    global $_tow_table;
    global $_tow_record;

    $args = arg();

    // Load dataset or die.
    if (!$_tow_dataset->nid) {
        $destination = explode('/', $_GET['destination']);
        $destination = $destination[1];
        if (!is_numeric($destination)) {
            die();
        }
        $_tow_dataset = node_load($destination);
    }

    //Set import data page breadcrumbs
    if (arg(0) == 'import') {
        $bc[] = l('Data', '<front>');
        $bc[] .= l($_tow_dataset->title, 'node/' . $_tow_dataset->nid);
        $bc[] .= t('Import data');
        drupal_set_breadcrumb($bc);
        return;
    }
    
    //Set other breadcrumbs
    $bc = array();
    $bc[] = l('Data', '<front>');

    // Select taxonomy term corresponding to dataset category.
    if ($_tow_dataset->taxonomy) {
        foreach ($_tow_dataset->taxonomy as $term_obj) {
            $q = db_result(db_query("SELECT v.vid AS vid FROM {vocabulary} v WHERE v.name = '%s'", 'Dataset categories'));
            if ($term_obj->vid == $q) {
                $bc[] = l($term_obj->name, 'taxonomy/term/' . $term_obj->tid);
                break;
            }
        }
    }

    global $_tow_dataset_search;
    global $_tow_dataset_doc;
    if ($_tow_dataset_search === TRUE) {
        $bc[] = l($_tow_dataset->title, 'node/' . $_tow_dataset->nid);
        $bc[] = 'search';
    } else if ($_tow_table) {
        $bc[] = l($_tow_dataset->title, 'node/' . $_tow_dataset->nid);
        $bc[] = $_tow_table->title;
    } else if ($_tow_dataset_doc === TRUE) {
        $bc[] = l($_tow_dataset->title, 'node/' . $_tow_dataset->nid);
        $bc[] = 'Docs';
    } else {
        $bc[] = $_tow_dataset->title;
    }
    
    // Extend breadcrumbs.
    global $_tow_bc_attachment;
    if ($_tow_bc_attachment) {
        $bc[count($bc) - 1] .= $_tow_bc_attachment;
    }

    drupal_set_breadcrumb($bc);
}
