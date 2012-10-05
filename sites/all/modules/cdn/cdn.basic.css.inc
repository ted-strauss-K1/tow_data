<?php

/**
 * @file
 * Overrides of Drupal's CSS aggregation system. Ensures that files referenced
 * by CSS files are also served from the CDN, according to the CDN module's
 * CSS aggregation rules.
 */


/**
 * Mostly based on drupal_get_css().
 */
function _cdn_css_aggregate(&$vars) {
  // Don't override Drupal core's aggregation if this page is not going to use
  // a CDN anyway.
  if (!cdn_check_protocol() && !cdn_check_drupal_path($_GET['q'])) {
    return;
  }

  $output = '';
  $prefix = $suffix = '<!-- CSS aggregated by CDN module. -->' . "\n";

  $css = $vars['css'];

  $no_module_preprocess = '';
  $no_theme_preprocess = '';

  // Create different CSS aggregation files for HTTP and HTTPS.
  $schema = 'http';
  $https_mapping = variable_get(CDN_BASIC_MAPPING_HTTPS_VARIABLE, '');
  if (!empty($https_mapping) && cdn_serve_from_https()) {
    $schema = 'https';
  }

  $query_string = variable_get('css_js_query_string', '0');
  $preprocess_css = (variable_get('preprocess_css', FALSE) && (!defined('MAINTENANCE_MODE') || MAINTENANCE_MODE != 'update'));
  $directory = file_directory_path();
  $is_writable = is_dir($directory) && is_writable($directory) && (variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PUBLIC);

  foreach ($css as $media => $types) {
    // If CSS preprocessing is off, we still need to output the styles.
    // Additionally, go through any remaining styles if CSS preprocessing is on and output the non-cached ones.
    foreach ($types as $type => $files) {
      if ($type == 'module') {
        // Setup theme overrides for module styles.
        $theme_styles = array();
        foreach (array_keys($css[$media]['theme']) as $theme_style) {
          $theme_styles[] = basename($theme_style);
        }
      }
      foreach ($types[$type] as $file => $preprocess) {
        // If the theme supplies its own style using the name of the module style, skip its inclusion.
        // This includes any RTL styles associated with its main LTR counterpart.
        if ($type == 'module' && in_array(str_replace('-rtl.css', '.css', basename($file)), $theme_styles)) {
          // Unset the file to prevent its inclusion when CSS aggregation is enabled.
          unset($types[$type][$file]);
          continue;
        }
        // Only include the stylesheet if it exists.
        if (file_exists($file)) {
          if (!$preprocess || !($is_writable && $preprocess_css)) {
            // Also build CSS cache files for other files; so that the file
            // URLs in them are altered to point to the CDN.
            $fake_types = array($type => array($file => TRUE));
            $filename = 'cdn_css_' . $schema . '_' . md5(serialize($types) . $query_string . 'cdn') .'_'. basename($file);
            $file = _cdn_build_css_cache($fake_types, $filename);

            // If a CSS file is not to be preprocessed and it's a module CSS file, it needs to *always* appear at the *top*,
            // regardless of whether preprocessing is on or off.
            if (!$preprocess && $type == 'module') {
              $no_module_preprocess .= '<link type="text/css" rel="stylesheet" media="'. $media .'" href="'. _cdn_css_file_create_url($file) .'" />'."\n";
            }
            // If a CSS file is not to be preprocessed and it's a theme CSS file, it needs to *always* appear at the *bottom*,
            // regardless of whether preprocessing is on or off.
            elseif (!$preprocess && $type == 'theme') {
              $no_theme_preprocess .= '<link type="text/css" rel="stylesheet" media="'. $media .'" href="'. _cdn_css_file_create_url($file) .'" />'."\n";
            }
            else {
              $output .= '<link type="text/css" rel="stylesheet" media="'. $media .'" href="'. _cdn_css_file_create_url($file) .'" />'."\n";
            }
          }
        }
      }
    }

    if ($is_writable && $preprocess_css) {
      // Prefix filename to prevent blocking by firewalls which reject files
      // starting with "ad*".
      $filename = 'cdn_css_'. md5(serialize($types) . $query_string . 'cdn') .'.css';
      $preprocess_file = _cdn_build_css_cache($types, $filename);
      $output .= '<link type="text/css" rel="stylesheet" media="'. $media .'" href="'. _cdn_css_file_create_url($preprocess_file) .'" />'."\n";
    }
  }

  $vars['styles'] = $prefix . $no_module_preprocess . $output . $no_theme_preprocess . $suffix;
}

function _cdn_build_css_cache($types, $filename) {
  $data = '';

  // Create the css/ within the files folder.
  $csspath = file_create_path('css');
  file_check_directory($csspath, FILE_CREATE_DIRECTORY);

  if (!file_exists($csspath .'/'. $filename)) {
    // Build aggregate CSS file.
    foreach ($types as $type) {
      foreach ($type as $file => $cache) {
        if ($cache) {
          $contents = drupal_load_stylesheet($file, TRUE);
          // Return the Drupal path to where this CSS file originated from.
          $base = dirname($file) .'/';
          _cdn_build_css_path(NULL, $base);
          // Prefix all paths within this CSS file, ignoring external and absolute paths.
          $data .= preg_replace_callback('/url\([\'"]?(?![a-z]+:|\/+)([^\'")]+)[\'"]?\)/i', '_cdn_build_css_path', $contents);
        }
      }
    }

    // Per the W3C specification at http://www.w3.org/TR/REC-CSS2/cascade.html#at-import,
    // @import rules must proceed any other style, so we move those to the top.
    $regexp = '/@import[^;]+;/i';
    preg_match_all($regexp, $data, $matches);
    $data = preg_replace($regexp, '', $data);
    $data = implode('', $matches[0]) . $data;

    // Create the CSS file.
    file_save_data($data, $csspath .'/'. $filename, FILE_EXISTS_REPLACE);
  }
  return $csspath .'/'. $filename;
}

function _cdn_build_css_path($matches, $base = NULL) {
  static $_base;
  // Store base path for preg_replace_callback.
  if (isset($base)) {
    $_base = $base;
  }

  // Prefix with base and remove '../' segments where possible.
  $path = $_base . $matches[1];
  $last = '';
  while ($path != $last) {
    $last = $path;
    $path = preg_replace('`(^|/)(?!\.\./)([^/]+)/\.\./`', '$1', $path);
  }

  return 'url('. _cdn_css_file_create_url($path) .')';
}

/**
 * Generate CDN file URL without file_create_url() if the core patch has not
 * been applied.
 */
function _cdn_css_file_create_url($path) {
  if (variable_get(CDN_THEME_LAYER_FALLBACK_VARIABLE, FALSE) == TRUE) {
    // Store the current path as the old path, then let cdn_file_url_alter()
    // do its magic by invoking all file_url_alter hooks. When the path hasn't
    // changed and is not already root-relative or protocol-relative, then
    // generate a file URL as Drupal core would: prepend the base path.
    $old_path = $path;
    drupal_alter('file_url', $path);
    if ($path == $old_path && drupal_substr($path, 0, 1) != '/' && drupal_substr($path, 0, 2) != '//') {
      $path = base_path() . $path;
    }
    return $path;
  }
  else {
    return file_create_url($path);
  }
}
