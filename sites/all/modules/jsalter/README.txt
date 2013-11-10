When this module is enabled, other modules can implement a hook_js_alter() function to change the javascript that is sent to the page. The signature of this function is:

function <module_name>_js_alter(&$javascript, $scope, &$query_string) {
}

The first parameter is the javascript array structured the same way as what would be returned by calling drupal_add_js(NULL, NULL, $scope). Altering this array will result in different javascript being sent to the page.

The second parameter is the scope, either 'header' or 'footer'.

The third parameter is the string that will be appended to each javascript file's url. Drupal uses this to force browsers to get a new file when it's available rather than erroneously relying on what's in the browser cache. The hook_js_alter function can modify this string.

