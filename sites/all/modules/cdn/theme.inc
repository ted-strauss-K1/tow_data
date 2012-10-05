<?php

/**
 * @file
 * Theme functions.
 */

/**
 * Render the CDN integration page statistics.
 *
 * @param $file_count
 *   The number of files detected on this page.
 * @param $cdn_file_count
 *   The number of files on this page that are served from a CDN.
 * @param $synced_files_per_server_count
 *   The number of files synced to each server.
 * @param $total_time
 *   The total time it took to get all current CDN URLs.
 * @param $synced_files
 *   Array of synchronized files.
 * @param $unsynced_files
 *   Array of unsynchronized files.
 * @return
 *   The rendered HTML.
 */
function theme_cdn_page_stats($file_count, $cdn_file_count, $synced_files_per_server_count, $total_time, $synced_files, $unsynced_files) {
  $output = '';
  $items = array();
  $mode = variable_get(CDN_MODE_VARIABLE, CDN_MODE_BASIC);

  $percentage = 100;
  if ($file_count > 0) {
    $percentage = number_format($cdn_file_count / $file_count * 100);
  }
  $avg_time = 0;
  if ($file_count > 0) {
    $avg_time = round($total_time * 1000 / $file_count, 3);
  }

  $output .= '<div id="cdn-integration-page-stats">';
  $items[] = t(
    'Total number of files on this page: <strong>!file-count</strong>.',
    array('!file-count' => $file_count)
  );
  $items[] = t(
    'Number of files served from CDNs: <strong>!cdn-file-count</strong>
    (!pct% coverage).',
    array('!cdn-file-count' => $cdn_file_count, '!pct' => $percentage)
  );
  $items[] = t(
    'Total time altering file URLs: !total ms, or !avg ms per file.',
    array(
      '!total' => round($total_time * 1000, 3),
      '!avg'   => $avg_time,
    )
  );

  // Nested list of unsynced files.
  if (count($unsynced_files)) {
    $unsynced_items = array();
    foreach ($unsynced_files as $file) {
      $unsynced_items[] = theme(
        'cdn_page_stats_file_link',
        $file, file_create_url($file), FALSE, $file, NULL
      );
    }
    $item = t('Files not served from CDNs:');
    $item .= theme('item_list', $unsynced_items, NULL, 'ol');
    $items[] = $item;
  }

  // Nested list of synced files.
  if (count($synced_files)) {
    $synced_items = array();
    foreach ($synced_files as $synced_file) {
      $file    = $synced_file['file'];
      $cdn_url = $synced_file['cdn_url'];
      $server  = $synced_file['server'];

      if (!isset($synced_items[$server])) {
        $synced_items[$server] = array();
      }
      $synced_items[$server][] = theme(
        'cdn_page_stats_file_link',
        $file,
        $synced_file['absolute path'],
        TRUE,
        $cdn_url,
        $server
      );
    }
    $synced_by_server = array();
    foreach (array_keys($synced_items) as $server) {
      $c = $synced_files_per_server_count[$server];
      $item = t(
        '<strong>!pct%</strong> of files served from %server (!count files):',
        array(
          '%server' => $server,
          '!count'  => $c,
          '!pct'    => number_format($c / $cdn_file_count * 100),
        )
      );
      $item .= theme(
        'item_list',
        $synced_items[$server],
        NULL,
        'ol'
      );
      $synced_by_server[] = $item;
    }
    $item = t('Files served from CDNs:');
    $item .= theme('item_list', $synced_by_server);
    $items[] = $item;
  }

  $title = t('CDN integration statistics for %drupal_path', array(
    '%drupal_path' => $_GET['q']
  ));
  $output .= theme(
    'item_list',
    $items,
    '<strong>'. $title .'</strong>'
  );
  $output .= '</div>';
  return $output;
}

/**
 * Render a file link in the CDN integration page statistics.
 *
 * @param $file
 *   A string containing the Drupal path (i.e. path relative to the Drupal
 *   root directory) of the file to generate the URL for.
 * @param $absolute_path
 *   The absolute path (on the filesystem) to the file.
 * @param $synced
 *   Whether this file has been synced to the CDN or not.
 * @param $cdn_url
 *   The CDN URL of the file, or the normal URL when the file is not on a CDN.
 * @param $server
 *   The server on which the file resides.
 * @return
 *   The rendered HTML.
 */
function theme_cdn_page_stats_file_link($file, $absolute_path, $synced, $cdn_url, $server) {
  $file_link = l(
    t('!file', array('!file' => $file)),
    $cdn_url,
    array(
      'external'   => TRUE,
      'attributes' => array('title' => $absolute_path),
    )
  );
  $touch_link = l(
    t('touch'),
    'admin/cdn/touch/' . $file,
    array('query' => drupal_get_destination())
  );

  $output = '';
  $output .= '<span class="file-link">' . $file_link;
  if ($synced) {
    $output .= '<span class="touch-link">';
    $output .=   '<span class="arrow">' . t('→') . '</span>';
    $output .=   $touch_link;
    $output .=   '<span class="touch-help">';
    $output .=   t('Touching this file will trigger a resync to the CDN.');
    $output .=   '</span>';
    $output .= '</span>';
  }
  $output .= '</span>';
  return $output;
}
