<?php

/**
 * @file
 *
 * Functions for fetching and updating dataset and table statistics.
 */
/* * **************************************************************************************************
 * Functions
 * ************************************************************************************************** */

/**
 * Prepare global variable with dataset statistics.
 */
function tow_statistics_dataset_extract($nid) {
    global $_tow_dataset_statistics;
    $_tow_dataset_statistics = db_fetch_array(db_query("

    SELECT
      t.totalcount AS count,
      t.timestamp AS timestamp

      FROM
        {node_counter} t

      WHERE
        t.nid = %d

      LIMIT 1
  ", $nid));
}

/**
 * If current user is not an owner of dataset node, store data about page view in tow_node_views_counter table.
 */
function tow_statistics_node_views_update($node) {

    // Dataset owner should not affect statistics of his datasets.
    global $user;
    if ($user->uid == $node->uid) {
        return;
    }

    $time = time();

    $count = db_result(db_query("

    SELECT
      MAX(t.count) AS count

      FROM
        {tow_node_views_counter} t

      WHERE
        t.nid = %d

  ", $node->nid));

    if (!$count) {
        $count++;
        db_query("

      INSERT
        INTO
          {tow_node_views_counter}
            (nid, count, timestamp)

        VALUES
          (%d, %d, %d)

    ", $node->nid, $count, $time);
    } else {
        $count++;
        db_query("

      UPDATE
        {tow_node_views_counter}

      SET
        count = %d,
        timestamp = %d

      WHERE
        nid = %d

    ", $count, $time, $node->nid);
    }
}

/**
 * Prepare global variable with table statistics.
 */
function tow_statistics_table_extract($nid) {
    global $_tow_table_statistics;
    $_tow_table_statistics = db_result(db_query("

    SELECT
      t.count AS count

      FROM
        {tow_table_counter} t

      WHERE nid = %d

      LIMIT 1

  ", $nid));
}

/**
 * If current user is not an owner of table node, store data about page view in tow_table_counter table.
 */
function tow_statistics_table_update($node) {

    // Table owner should not affect statistics of his datasets.
    global $user;
    if ($user->uid == $node->uid) {
        return;
    }

    $count = db_result(db_query("

    SELECT
      MAX(t.count) AS count

      FROM
        {tow_table_counter} t

      WHERE
        t.nid = %d

  ", $node->nid));
    $count++;

    if (!$count) {
        db_query("

      INSERT
        INTO
          {tow_table_counter}
            (nid, count)

        VALUES
          (%d, %d)

    ", $node->nid, $count);
    } else {
        db_query("

      UPDATE
        {tow_table_counter}

      SET
        count = %d

      WHERE
        nid = %d

    ", $count, $node->nid);
    }
}