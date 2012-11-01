
------------------------------------------------
HTTP PARALLEL REQUEST & THREADING LIBRARY MODULE
------------------------------------------------


CONTENTS OF THIS FILE
---------------------

 * About HTTPRL
 * Requirements
 * Configuration
 * API Overview
 * Technical Details
 * Code Examples


ABOUT HTTPRL
------------

http://drupal.org/project/httprl

HTTPRL is a flexible and powerful HTTP client implementation. Correctly handles
GET, POST, PUT or any other HTTP requests & the sending of data. Issue blocking
or non-blocking requests in parallel. Set timeouts, max simultaneous connection
limits, chunk size, and max redirects to follow. Can handle data with
content-encoding and transfer-encoding headers set. Correctly follows
redirects. Option to forward the referrer when a redirect is found. Cookie
extraction and parsing into key value pairs.


REQUIREMENTS
------------

Requires PHP 5. The following functions must be available on the server:
 * stream_socket_client
 * stream_select
 * stream_set_blocking
 * stream_get_meta_data
 * stream_socket_get_name
Some hosting providers disable these functions; but they do come standard with
PHP 5.


CONFIGURATION
-------------

Settings page is located at:
6.x: admin/settings/httprl
7.x: admin/config/development/httprl

 * IP Address to send all self server requests to. If left blank it will use the
   same server as the request. If set to -1 it will use the host name instead of
   an IP address. This controls the output of httprl_build_url_self().


API OVERVIEW
------------

Issue HTTP Requests:
httprl_build_url_self()
 - Helper function to build an URL for asynchronous requests to self.
httprl_request()
 - Queue up a HTTP request in httprl_send_request().
httprl_send_request()
 - Perform many HTTP requests.

Create and use a thread:
httprl_queue_background_callback()
 - Queue a special HTTP request (used for threading) in httprl_send_request().

Other Functions:
httprl_background_processing()
 - Output text, close connection, continue processing in the background.
httprl_strlen()
 - Get the length of a string in bytes.
httprl_glue_url()
 - Alt to http_build_url().
httprl_get_server_schema()
 - Return the server schema (http or https).
httprl_pr()
 - Pretty print data.
httprl_fast403()
 - Issue a 403 and exit.

TECHNICAL DETAILS
-----------------

Using stream_select() HTTPRL will send http requests out in parallel. These
requests can be made in a blocking or non-blocking way. Blocking will wait for
the http response; Non-Blocking will close the connection not waiting for the
response back. The API for httprl is similar to the Drupal 7 version of
drupal_http_request().


CODE EXAMPLES
-------------

Request this servers own front page & the node page.

    <?php
    // Build URL to point to front page of this server.
    $url_front = httprl_build_url_self();
    $url_node = httprl_build_url_self('node');
    // Queue up the requests.
    httprl_request($url_front);
    httprl_request($url_node);
    // Execute requests.
    $request = httprl_send_request();

    // Echo out the results.
    echo httprl_pr($request);
    ?>


Request 10 URLs in a non blocking manner on this server. Checkout watchdog as
this should generate 10 404s and the $request object won't contain much info.

    <?php
    // Set the blocking mode.
    $options = array(
      'blocking' => FALSE,
    );
    // Queue up the requests.
    $max = 10;
    for ($i=1; $i <= $max; $i++) {
      // Build URL to a page that doesn't exist.
      $url = httprl_build_url_self('asdf-asdf-asdf-' . $i);
      httprl_request($url, $options);
    }
    // Execute requests.
    $request = httprl_send_request();

    // Echo out the results.
    echo httprl_pr($request);
    ?>


print 'My Text'; cut the connection by sending the data over the wire and do
processing in the background.

    <?php
    httprl_background_processing('My Text');
    // Everything after this point does not affect page load time.
    ?>


Use a callback in the event loop to do processing on the request. In this case
we are going to use httprl_pr() as the callback function.

    <?php
    // Setup return variable.
    $x = '';
    // Setup options array.
    $options = array(
      'method' => 'HEAD',
      'callback' => array(
        array(
          'function' => 'httprl_pr',
          'return' => &$x,
        ),
      ),
    );
    // Build URL to point to front page of this server.
    $url_front = httprl_build_url_self();
    // Queue up the request.
    httprl_request($url_front, $options);
    // Execute request.
    $request = httprl_send_request();

    // Echo returned value from function callback.
    echo $x;
    ?>


Use a background callback in the event loop to do processing on the request.
In this case we are going to use httprl_pr() as the callback function. A
background callback creates a new thread to run this function in.

    <?php
    // Setup return variable.
    $x = '';
    // Setup options array.
    $options = array(
      'method' => 'HEAD',
      'background_callback' => array(
        array(
          'function' => 'httprl_pr',
          'return' => &$x,
        ),
      ),
    );
    // Build URL to point to front page of this server.
    $url_front = httprl_build_url_self();
    // Queue up the request.
    httprl_request($url_front, $options);
    // Execute request.
    $request = httprl_send_request();

    // Echo returned value from function callback.
    echo $x;
    ?>


Use a background callback in the event loop to do processing on the request.
In this case we are going to use print_r() as the callback function. A
background callback creates a new thread to run this function in. The first
argument passed in is the request object, the FALSE tells print_r to echo out
instead of returning a value.

    <?php
    // Setup return & print variable.
    $x = '';
    $y = '';
    // Setup options array.
    $options = array(
      'method' => 'HEAD',
      'background_callback' => array(
        array(
          'function' => 'print_r',
          'return' => &$x,
          'printed' => &$y,
        ),
        FALSE,
      ),
    );
    // Build URL to point to front page of this server.
    $url_front = httprl_build_url_self();
    // Queue up the request.
    httprl_request($url_front, $options);
    // Execute request.
    $request = httprl_send_request();

    // Echo what was returned and printed from function callback.
    echo $x . "<br />\n";
    echo $y;
    ?>


Use 2 threads to load up 4 different nodes.

    <?php
    // List of nodes to load; 241-244.
    $nodes = array(241 => '', 242 => '', 243 => '', 244 => '');
    foreach ($nodes as $nid => &$node) {
      // Setup callback options array.
      $callback_options = array(
        array(
          'function' => 'node_load',
          'return' => &$node,
          // Setup options array.
          'options' => array(
            'domain_connections' => 2, // Only use 2 threads for this request.
          ),
        ),
        $nid,
      );
      // Queue up the request.
      httprl_queue_background_callback($callback_options);
    }
    // Execute request.
    httprl_send_request();

    // Echo what was returned.
    echo httprl_pr($nodes);
    ?>


Run a function in the background. Notice that there is no return or printed key
in the callback options.

    <?php
    // Setup callback options array; call watchdog in the background.
    $callback_options = array(
      array(
        'function' => 'watchdog',
      ),
      'httprl-test', 'background watchdog call done', array(), WATCHDOG_DEBUG,
    );
    // Queue up the request.
    httprl_queue_background_callback($callback_options);

    // Execute request.
    httprl_send_request();
    ?>


Pass by reference example. Example is D7 only; pass by reference works in
D6 & D7.

    <?php
    // Code from system_rebuild_module_data().
    $modules = _system_rebuild_module_data();
    ksort($modules);

    // Show first module before running system_get_files_database().
    echo httprl_pr(current($modules));

    $callback_options = array(
      array(
        'function' => 'system_get_files_database',
        'return' => '',
      ),
      &$modules, 'module'
    );
    httprl_queue_background_callback($callback_options);

    // Execute requests.
    httprl_send_request();

    // Show first module after running system_get_files_database().
    echo httprl_pr(current($modules));
    ?>


Request multiple URLs with one httprl_request() call. These URLs will all have
the same options.

    <?php
    // Set the blocking mode.
    $options = array(
      'method' => 'HEAD',
      'blocking' => FALSE,
    );
    // Queue up the requests.
    $max = 10;
    $urls = array();
    for ($i=1; $i <= $max; $i++) {
      // Build URL to a page that doesn't exist.
      $urls[] = httprl_build_url_self('asdf-asdf-asdf-' . $i);
    }
    // Queue up the requests.
    httprl_request($urls, $options);
    // Execute requests.
    $request = httprl_send_request();

    // Echo out the results.
    echo httprl_pr($request);
    ?>


Get 2 results from 2 different queries at the hook_boot bootstrap level in D6.

    <?php
    // Run 2 queries and get the result.
    $x = db_result(db_query_range("SELECT filename FROM {system} ORDER BY filename ASC", 0, 1));
    $y = db_result(db_query_range("SELECT filename FROM {system} ORDER BY filename DESC", 0, 1));
    echo $x . "<br \>\n" . $y . "<br \>\n";
    unset($x, $y);


    // Run above 2 queries and get the result via a background callback.
    $args = array(
      // First query.
      array(
        'type' => 'function',
        'call' => 'db_query_range',
        'args' => array('SELECT filename FROM {system} ORDER BY filename ASC', 0, 1),
      ),
      array(
        'type' => 'function',
        'call' => 'db_result',
        'args' => array('last' => NULL),
        'return' => &$x,
      ),

      // Second Query.
      array(
        'type' => 'function',
        'call' => 'db_query',
        'args' => array('SELECT filename FROM {system} ORDER BY filename DESC', 0, 1),
      ),
      array(
        'type' => 'function',
        'call' => 'db_result',
        'args' => array('last' => NULL),
        'return' => &$y,
      ),
    );
    $callback_options = array(array('return' => ''), &$args);
    // Queue up the request.
    httprl_queue_background_callback($callback_options);
    // Execute request.
    httprl_send_request();

    // Echo what was returned.
    echo httprl_pr($x, $y);
    ?>


Get 2 results from 2 different queries at the hook_boot bootstrap level in D7.

    <?php
    $x = db_select('system', 's')
      ->fields('s', array('filename'))
      ->orderBy('filename', 'ASC')
      ->range(0, 1)
      ->execute()
      ->fetchField();
    $y = db_select('system', 's')
      ->fields('s', array('filename'))
      ->orderBy('filename', 'DESC')
      ->range(0, 1)
      ->execute()
      ->fetchField();
    echo $x . "<br \>\n" . $y . "<br \>\n";
    unset($x, $y);

    // Run above 2 queries and get the result via a background callback.
    $args = array(
      // First query.
      array(
        'type' => 'function',
        'call' => 'db_select',
        'args' => array('system', 's',),
      ),
      array(
        'type' => 'method',
        'call' => 'fields',
        'args' => array('s', array('filename')),
      ),
      array(
        'type' => 'method',
        'call' => 'orderBy',
        'args' => array('filename', 'ASC'),
      ),
      array(
        'type' => 'method',
        'call' => 'range',
        'args' => array(0, 1),
      ),
      array(
        'type' => 'method',
        'call' => 'execute',
        'args' => array(),
      ),
      array(
        'type' => 'method',
        'call' => 'fetchField',
        'args' => array(),
        'return' => &$x,
      ),

      // Second Query.
      array(
        'type' => 'function',
        'call' => 'db_select',
        'args' => array('system', 's',),
      ),
      array(
        'type' => 'method',
        'call' => 'fields',
        'args' => array('s', array('filename')),
      ),
      array(
        'type' => 'method',
        'call' => 'orderBy',
        'args' => array('filename', 'DESC'),
      ),
      array(
        'type' => 'method',
        'call' => 'range',
        'args' => array(0, 1),
      ),
      array(
        'type' => 'method',
        'call' => 'execute',
        'args' => array(),
      ),
      array(
        'type' => 'method',
        'call' => 'fetchField',
        'args' => array(),
        'return' => &$y,
      ),
    );
    $callback_options = array(array('return' => ''), &$args);
    // Queue up the request.
    httprl_queue_background_callback($callback_options);
    // Execute request.
    httprl_send_request();

    // Echo what was returned.
    echo httprl_pr($x, $y);
    ?>


Run a cache clear at the DRUPAL_BOOTSTRAP_FULL level as the current user in a
non blocking background request.

    <?php
    // Normal way to do this.
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
    module_load_include('inc', 'system', 'system.admin');
    system_clear_cache_submit();


    // How to do it in a non blocking background request.
    $args = array(
      array(
        'type' => 'function',
        'call' => 'drupal_bootstrap',
        'args' => array(DRUPAL_BOOTSTRAP_FULL),
      ),
      array(
        'type' => 'function',
        'call' => 'module_load_include',
        'args' => array('inc', 'system', 'system.admin'),
      ),
      array(
        'type' => 'function',
        'call' => 'system_clear_cache_submit',
        'args' => array('', ''),
      ),
      array(
        'type' => 'function',
        'call' => 'watchdog',
        'args' => array('httprl-test', 'background cache clear done', array(), WATCHDOG_DEBUG),
      ),
    );

    // Pass the current session to the sub request.
    if (!empty($_COOKIE[session_name()])) {
      $options = array('headers' => array('Cookie' => session_name() . '=' . $_COOKIE[session_name()] . ';'));
    }
    else {
      $options = array();
    }
    $callback_options = array(array('options' => $options), &$args);

    // Queue up the request.
    httprl_queue_background_callback($callback_options);
    // Execute request.
    httprl_send_request();
    ?>


Hit 4 different URLs, Using at least 2 that has a status code of 200 and
erroring out on the others that didn't return. Data is truncated as well.

    // Array of URLs to get.
    $urls = array(
      'http://google.com/',
      'http://bing.com/',
      'http://yahoo.com/',
      'http://www.duckduckgo.com/',
      'http://www.drupal.org/',
    );

    // Process list of URLs.
    $options = array(
      'alter_all_streams_function' => 'need_two_good_results',
      'callback' => array(array('function' => 'limit_data_size')),
    );
    // Queue up the requests.
    httprl_request($urls, $options);

    // Execute requests.
    $requests = httprl_send_request();

    // Print what was done.
    echo httprl_pr($requests);

    function need_two_good_results($id, &$responses, &$streams, $current_time) {
      static $counter = 0;
      foreach ($responses as $id => &$result) {
        // Skip if we got a 200.
        if ($result->code == 200) {
          $counter = $counter + 1;
          continue;
        }
        if ($result->status == 'Done.') {
          continue;
        }

        if ($counter >= 2) {
          // Set the code to request was aborted.
          $result->code = HTTPRL_REQUEST_ABORTED;
          $result->status = 'Done.';
          $result->options['timeout'] = $result->options['timeout'] - $current_time;

          // Close the file pointer and remove from the stream from the array.
          fclose($streams[$id]);
          unset($streams[$id]);
        }
      }
    }

    function limit_data_size(&$result) {
      // Only use the first and last 256 characters in the data array.
      $result->data = substr($result->data, 0, 256) . "\n\n ... \n\n" . substr($result->data, strlen($result->data)-256);
    }


