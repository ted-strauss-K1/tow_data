<?php

/**
 * @file
 * Advanced aggregation module; 404 handler.
 *
 */

/**
 * Menu Callback; regenerates a missing css file.
 */
function advagg_missing_css() {
  ignore_user_abort();

  // Try to regenerate missing file
  $msg = advagg_missing_regenerate();

  // If here send out fast 404.
  advagg_missing_fast404($msg);
}

/**
 * Menu Callback; regenerates a missing js file.
 */
function advagg_missing_js() {
  ignore_user_abort();

  // Try to regenerate missing file
  $msg = advagg_missing_regenerate();

  // If here send out fast 404.
  advagg_missing_fast404($msg);
}

/**
 * regenerates a missing css file.
 *
 * @param $filename
 *   filename
 * @param $type
 *   css or js
 * @return
 *   false if bundle couldn't be generated.
 */
function advagg_missing_regenerate() {
  global $base_path, $conf;

  // Get filename from request.
  $arg = arg();
  $filename = array_pop($arg);
  $filename = explode('?', $filename);
  $filename = array_shift($filename);

  $data = advagg_get_bundle_from_filename($filename);
  if (is_array($data)) {
    list($type, $md5, $counter) = $data;
  }
  else {
    return $data;
  }

  $_GET['redirect_counter'] = isset($_GET['redirect_counter']) ? intval($_GET['redirect_counter']) : 0;
  if ($_GET['redirect_counter'] > 5) {
    watchdog('advagg', 'This request could not generate correctly. Loop detected. Request data: %info', array('%info' => $_GET['q']));
    return t('In a Loop.');
  }

  // Counter in database.
  $counter_in_db = db_result(db_query("SELECT counter FROM {advagg_bundles} WHERE bundle_md5 = '%s'", $md5));
  if ($counter_in_db === FALSE) {
    return t('Not a valid bundle.');
  }
  // Cast counter as int
  $counter_in_db = intval($counter_in_db);
  $counter = intval($counter);

  // Set file(s) in cache to FALSE.
  $arg[] = $filename;
  cache_set(implode('/', $arg), FALSE, 'cache_advagg', TRUE);
  advagg_missing_remove_cache($md5);

  // Build filepath.
  list($css_path, $js_path) = advagg_get_root_files_dir();
  if ($type == 'js') {
    $file_type_path = $js_path;
  }
  if ($type == 'css') {
    $file_type_path = $css_path;
  }
  $new_filename = advagg_build_filename($type, $md5, $counter);
  $filepath = $file_type_path . '/' . $new_filename;

  // Only process if we got an older counter.
  // If we have an out of range counter see if a simlar file exists and serve
  // that up.
  if ($counter > $counter_in_db || $counter < 0) {
    $new_filename = advagg_build_filename($type, $md5, $counter_in_db);
    $filepath = $file_type_path . '/' . $new_filename;
    advagg_missing_send_file($filepath, advagg_build_uri($filepath), $type);
    exit;
  }

  // Break connection and do generation in the background.
  if (!empty($_GET['generator'])) {
    advagg_missing_async_opp($md5 . ' ' . $counter);
  }

  // Rebuild file.
  $conf['advagg_async_generation'] = FALSE;
  $good = advagg_rebuild_bundle($md5, $counter, TRUE);
  if (!$good) {
    watchdog('advagg', 'This request could not generate correctly. Aggregate not generated. Request data: %info', array('%info' => $_GET['q']));
    return t('Rebuild Failed.');
  }

  // Serve direct or redirect to file.
  $_GET['redirect_counter']++;
  $uri = $base_path . $_GET['q'] . '?redirect_counter=' . $_GET['redirect_counter'];
  advagg_missing_send_file($filepath, $uri, $type);
  exit;
}

/**
 * Send the file or send a 307 redirect.
 *
 * @param $filepath
 *   filename
 * @param $uri
 *   css or js
 */
function advagg_missing_send_file($filepath, $uri, $type) {
  if (!headers_sent()) {
    // Code from file_download.
    if (file_exists($filepath)) {
      $headers = module_invoke_all('file_download', $filepath, $type);
      if ($key = array_search(-1, $headers)) {
        unset($headers[$key]);
      }

      // Remove all previously set Cache-Control headers, because we're going to
      // override it. Since multiple Cache-Control headers might have been set,
      // simply setting a new, overriding header isn't enough: that would only
      // override the *last* Cache-Control header. Yay for PHP!
      foreach ($headers as $key => $header) {
        if (strpos($header, 'Cache-Control:') === 0) {
          unset($headers[$key]);
        }
        elseif (strpos($header, 'ETag:') === 0) {
          unset($headers[$key]);
        }
      }
      if (function_exists('header_remove')) {
        header_remove('Cache-Control');
        header_remove('ETag');
      }
      else {
        drupal_set_header("Cache-Control:");
        drupal_set_header("Cache-Control:");
        drupal_set_header("ETag:");
        drupal_set_header("ETag:");
      }
      // Set a far future Cache-Control header (480 weeks), which prevents
      // intermediate caches from transforming the data and allows any
      // intermediate cache to cache it, since it's marked as a public resource.
      $headers[] = "Cache-Control: max-age=290304000, no-transform, public";

      if (count($headers)) {
        advagg_missing_file_transfer($filepath, $headers);
      }
    }

    // advagg_missing_file_transfer didn't run/send data, redirect via header.
    header('Location: ' . $uri, TRUE, 307);
    usleep(250000); // Sleep for 250ms
  }
}

/**
 * Set cache value to FALSE.
 *
 * @param $bundle_md5
 *   Bundle's machine name.
 */
function advagg_missing_remove_cache($bundle_md5) {
  $files = array();
  $results = db_query("SELECT filename, filetype FROM {advagg_files} AS af INNER JOIN {advagg_bundles} AS ab USING ( filename_md5 ) WHERE bundle_md5 = '%s' ORDER BY porder ASC", $bundle_md5);
  while ($row = db_fetch_array($results)) {
    $files[] = $row['filename'];
    $type = $row['filetype'];
  }

  list($css_path, $js_path) = advagg_get_root_files_dir();
  if ($type == 'js') {
    $file_type_path = $js_path;
  }
  if ($type == 'css') {
    $file_type_path = $css_path;
  }

  $filenames = advagg_get_filename($files, $type, '', $bundle_md5);
  if (!empty($filenames)) {
    foreach ($filenames as $key => $info) {
      $filename = $info['filename'];
      $filepath = $file_type_path . '/' . $filename;

      cache_set($filepath, FALSE, 'cache_advagg', TRUE);
    }
  }
}

/**
 * Output text & set php in async mode.
 *
 * @param $output
 *  string - Text to output to open connection.
 * @param $wait
 *  bool - Wait 1 second?
 * @param $content_type
 *  string - Content type header.
 * @param $length
 *  int - Content length.
 */
function advagg_missing_async_opp($output, $wait = TRUE, $content_type = "text/html; charset=utf-8", $length = 0) {
  if (headers_sent()) {
    return FALSE;
  }

  // Calculate Content Length
  if ($length == 0) {
    $output .= "\n";
    $length = (advagg_missing_strlen($output) -1);
  }
  // Prime php for background operations
  $loop = 0;
  while (ob_get_level() && $loop < 25) {
    ob_end_clean();
    $loop++;
  }
  header("Connection: close");
  ignore_user_abort();

  // Output headers & data
  ob_start();
  header("Content-type: " . $content_type);
  header("Expires: Sun, 19 Nov 1978 05:00:00 GMT");
  header("Cache-Control: no-cache");
  header("Cache-Control: must-revalidate");
  header("Content-Length: " . $length);
  header("Connection: close");
  print($output);
  ob_end_flush();
  flush();
  // wait for 1 second
  if ($wait) {
    sleep(1);
  }

  // text returned and connection closed.
  // Do background processing. Time taken after should not effect page load times.
  return TRUE;
}

/**
 * Get the length of a string in bytes
 *
 * @param $string
 *   get string length
 */
function advagg_missing_strlen($string) {
  if (function_exists('mb_strlen')) {
    return mb_strlen($string, '8bit');
  }
  else {
    return strlen($string);
  }
}

/**
 * Transfer file using http to client. Pipes a file through Drupal to the
 * client.
 *
 * @param $source File to transfer.
 * @param $headers An array of http headers to send along with file.
 */
function advagg_missing_file_transfer($source, $headers) {
  $source = advagg_missing_file_create_path($source);
  $fd = fopen($source, 'rb');
  // Return if we can't open the file. Will try a 307 in browser to send file.
  if (!$fd) {
    return;
  }

  // Clear the buffer.
  if (ob_get_level()) {
    ob_end_clean();
  }

  // Add in headers.
  foreach ($headers as $header) {
    // To prevent HTTP header injection, we delete new lines that are
    // not followed by a space or a tab.
    // See http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html#sec4.2
    $header = preg_replace('/\r?\n(?!\t| )/', '', $header);
    drupal_set_header($header);
  }

  // Transfer file in 8096 byte chunks.
  while (!feof($fd)) {
    print fread($fd, 8096);
  }
  fclose($fd);
  exit();
}

/**
 * Make sure the destination is a complete path and resides in the file system
 * directory, if it is not prepend the file system directory.
 *
 * @param $dest A string containing the path to verify. If this value is
 *   omitted, Drupal's 'files' directory will be used.
 * @return A string containing the path to file, with file system directory
 *   appended if necessary, or FALSE if the path is invalid (i.e. outside the
 *   configured 'files' or temp directories).
 */
function advagg_missing_file_create_path($dest = 0) {
  list($css_path, $js_path) = advagg_get_root_files_dir();

  $valid_paths = array();
  $valid_paths[] = file_directory_path();
  $valid_paths[] = $css_path;
  $valid_paths[] = $js_path;
  foreach ($valid_paths as $file_path) {
    if (!$dest) {
      return $file_path;
    }
    // file_check_location() checks whether the destination is inside the Drupal files directory.
    if (file_check_location($dest, $file_path)) {
      return $dest;
    }
    // check if the destination is instead inside the Drupal temporary files directory.
    elseif (file_check_location($dest, file_directory_temp())) {
      return $dest;
    }
    // Not found, try again with prefixed directory path.
    elseif (file_check_location($file_path . '/' . $dest, $file_path)) {
      return $file_path . '/' . $dest;
    }
  }
  // File not found.
  return FALSE;
}
