
Description
-----------
This module provide easy Content Delivery Network integration for Drupal sites.
It alters file URLs, so that files are downloaded from a CDN instead of your
web server.

Drupal 6 needs to be patched (patch included) to be able to rewrite the file
URLs — the same patch is already included in Drupal 7, so upgrading is easy!
Alternatively, you could use Pressflow [1], which already includes this Drupal
core patch.
If you are okay with having most, but not all files served from a CDN, then
it's even possible to just rely on the (automatically enabled) fallback
mechanism that uses Drupal's powerful theme layer to rewrite as many file URLs
as possible. For complete CDN coverage however, you will have to apply the
Drupal core patch.

It provides two modes: "Origin Pull" and "File Conveyor".

In "Origin Pull" mode, only "Origin Pull" CDNs are supported (hence the name).
These are CDNs that only require you to replace the domain name with another
domain name. The CDN will then automatically fetch (pull) the files from your
server (the origin).

In "File Conveyor" mode, this module integrates with the File Conveyor [2]
daemon. This allows for much more advanced setups: files can be processed
(e.g. optimize images like smush.it [3], minify CSS with YUI Compressor [4],
minify JS with YUI compressor or Google Closure Compiler [5], and it's easy to
add your own!), before they are synced and your CDN doesn't *have* to support
Origin Pull, any push method is fine (supported transfer protocols: FTP,
Amazon S3, Rackspace CloudFiles). File Conveyor is flexible enough to be used
with *any* CDN, thus it enables you to avoid vendor lock-in.

If you're not sure which mode to use, use "Origin Pull". It's easier and more
reliable. The only common CDN today (2011–2012) that doesn't support it is
Rackspace Cloud Files.

_Note:_ It is essential that you understand the key properties of a CDN, most
importantly the differences between an Origin Pull CDN and a Push CDN. A good
(and compact!) reference is the "Key Properties of a CDN" article [6].

The CDN module aims to do only one thing and do it well: altering URLs to
point to files on CDNs.
However, in later versions, it does as much as possible to make CDN
integration frictionless:
    • Any sort of CDN mapping
    • optimal Far Future expiration (http://drupal.org/node/974350)
        - CORS (http://drupal.org/node/982188)
        - signed URLs prevent abuse
        - disabled by default, automatically disabled when in maintenance mode
        - *requires* a CDN or reverse proxy, not Apache/nginx/lighttpd/…!
    • Advanced Help integration to guide you (http://drupal.org/node/1413162)
    • DNS prefetching (http://drupal.org/node/982188)
    • CSS aggregation (http://drupal.org/node/1428530)
    • auto-balance files over multiple CDNs (http://drupal.org/node/1452092)
    • … and many more details that are taken care of automatically

But in some cases, simply altering the URL is not enough, that's where the
AdvAgg module comes in:

    If you've ever had any issues with CSS or JS files not behaving as
    desired, check out AdvAgg. The "Advanced CSS/JS Aggregation" module solves
    all issues that arise from having CSS/JS served from a CDN. Keeping track
    of changes to CSS/JS files, smart aggregate names, 404 protection,
    on-demand generation, works with private file system, Google CDN
    integration, CSS/JS compression, GZIP compression, caching, and smart
    bundling are some of the things AdvAgg does. It's also faster then core's
    file aggregation.

[1] http://pressflow.org/
[2] http://fileconveyor.org/
[3] http://smushit.com/
[4] http://developer.yahoo.com/yui/compressor/
[5] http://code.google.com/closure/compiler/
[6] http://wimleers.com/article/key-properties-of-a-cdn


Supported CDNs
--------------
- Origin Pull mode: any Origin Pull CDN (or alternatively: domains that point
  to your main domain, by using so called "CNAME" DNS records).
- File Conveyor mode: any Origin Pull CDN and any push CDN that supports FTP.
  Support for other transfer protocols is welcomed and encouraged: your
  patches are welcome! Amazon S3, Amazon CloudFront and Rackspace CloudFiles
  are also supported.


Module compatibility
--------------------
This module has been verified to be compatible with the following modules:
- CSS Gzip
  (This module is obsolete when using Origin Pull mode with the Far Future
  expiration setting enabled.)
- ImageCache (version 2.0 beta 12 or later)
- Javascript Aggregator


Installation
------------
1) Place this module directory in your "modules" folder (this will usually be
   "sites/all/modules/"). Don't install your module in Drupal core's "modules"
   folder, since that will cause problems and is bad practice in general. If
   "sites/all/modules" doesn't exist yet, just create it.

2) Optional, but recommended.
   Apply the Drupal core patch (patches/drupal6.patch). Instructions can be
   found at http://drupal.org/patch/apply. However, a quick reminder:
     a) Change the directory to the root directory of your Drupal core:
          cd /htdocs/example.com
     b) Copy the patch to this directory
          cp /htdocs/example.com/sites/all/modules/cdn/patches/drupal6.patch .
     c) Apply the patch:
          patch -p0 < drupal6.patch
   This patch effectively backports hook_file_url_alter() from Drupal 7 to
   Drupal 6. See the notes about this backport if you're interested in the
   details or want to use it in your own module.

   NOTE: if you skip this step, the fallback mechanism will be used!

   NOTE: Pressflow users don't have to apply the Drupal core patch; Pressflow
         already includes this patch! The same applies to Cocomore Drupal
         users.

3) Enable the module.

4) Visit "admin/settings/cdn" to learn about the various settings.

5) Go to your CDN provider's control panel and set up a "CDN instance" (Amazon
   CloudFront calls this a "distribution"). There, you will have to specify
   the origin server (Amazon CloudFront calls this a "custom origin"), which
   is simply the domain name of your Drupal site.
   The CDN will provide you with a "delivery address", this is the address
   that we'll use to download files from the CDN instead of the Drupal server.
   Suppose this is `http://d85nwn7m5gl3y.cloudfront.net`.
   (It acts like a globally distributed, super fast proxy server.)

   Relevant links:
   - Amazon CloudFront: http://docs.amazonwebservices.com/AmazonCloudFront/latest/DeveloperGuide/CreatingDistributions.html?r=4212

6) Optionally, you can create a CNAME alias to the delivery address on your
   DNS server. This way, it's not immediately obvious from the links in the
   HTMl that you're using an external service (that's why it's also called a
   vanity domain name).
   However, if you're going to use your CDN in HTTPS mode, then using vanity
   domains will break things (because SSL certificates are bound to domain
   names).

7) Enter the domain name (`http://d85nwn7m5gl3y.cloudfront.net`, or the vanity
   domain/CNAME if you used that instead) at admin/settings/cdn/details. If
   you want to support HTTPS transparently, it is recommended to enter it as
   `//d85nwn7m5gl3y.cloudfront.net` instead — this is a protocol-relative URL.

8) Go to "admin/reports/status". The CDN module will report its status here.

9) Enable the display of statistics at "admin/settings/cdn", browse your site
   with your root/admin (user id 1) account. The statistics will show which
   files are served from the CDN!


Cross-Origin Resource Sharing (CORS)
------------------------------------
By integrating a CDN, and depending on your actual configuration, resources
might be served from (a) domain(s) different than your site's domain. This
could cause browsers to refuse to use certain resources since they violate the
same-origin policy. This primarily affects font and JavaScript files.

To circumvent this, you can configure your server to serve those files with an
additional Access-Control-Allow-Origin header, containing a space-separated
list of domains that are allowed to make cross-domain use of a resource. Note
that this will only work if your CDN provider does not strip this header.

For server-specific instructions on adding this header, see
http://www.w3.org/wiki/CORS_Enabled#At_the_HTTP_Server_level...

If you are unable to add this header, or if your CDN provider ignores it, you
can add the files to the CDN module's blacklist to exclude them being served
by the CDN, or in the case of fonts, you can embed them in stylesheets via
data URIs (see https://developer.mozilla.org/en/data_URIs).

The Far Future expiration functionality takes care of this automatically!


File Conveyor mode
------------------

1) If you want to use File Conveyor mode, install and configure the File
   Conveyor first. You can download it at http://fileconveyor.org/
   Then follow the instructions in the included INSTALL.txt and README.txt.
   Use the sample config.xml file that is included in this module, copy it to
   your File Conveyor installation and modify it to comply with your setup and
   to suit your needs. You will always need to modify this file to suit your
   needs.
   Note: the CDN integration module requires PDO extension for PHP to be
   installed, as well as the PDO SQLite driver.

2) Go to "admin/reports/status". The CDN module will report its status here.
   If you've enabled File Conveyor mode and have set up File Conveyor daemon,
   you will see some basic stats here as well, and you can check here to see
   if File Conveyor is currently running.
   You can also see here if you've applied the patches correctly!


FAQ
---
Q: Is the CDN module truly incompatible with Drupal's aggressive mode caching?
A: No. The CDN module will continue to function as expected. This will only
   prevent per-page statistics for anonymous users from working correctly.
   But in most cases you wouldn't want anonymous users to see these statistics
   anyway.
   Thus, this is a non-issue. Enable aggressive mode caching without worries!

Q: Is the CDN module compatible with Drupal's "private files" functionality?
A: Yes. The CDN module won't break private files, they will continue to work
   the same way. However, it cannot serve private files from a CDN. Not every
   CDN supports protected/secured/authenticated file access, and those that do
   each have their own way of doing this (there is no standard). So private
   files will continue to be served by Drupal, which may or may not be
   acceptable for your use case.

Q: Why are JavaScript files not being served from the CDN?
A: The answer can be found at "admin/settings/cdn/other".

Q: Why are CSS files not being served from the CDN?
A: This may be caused by your theme: http://drupal.org/node/1061588.

Q: Does this module only work with Apache or also with nginx, lighttpd, etc.?
A: This module only affects HTML, so it doesn't matter which web server you
   use!

Q: What does the config.xml file of the CDN module do?
A: Nothing. It only serves as a sample for using File Conveyor. It's used for
   nothing and can safely be deleted.

Q: How to use different CDNs based on the domain name of an i18n site?
A: See http://drupal.org/node/1483962#comment-5744830.


No cookies should be sent to the CDN
------------------------------------
Please note though that you should ensure no cookies are sent to the CDN: this 
would slow down HTTP requests to the CDN (since the requests become larger:
they piggyback the cookie data).
You can achieve this in two ways:
  1) When you are using cookies that are bound to your www subdomain only
     (i.e. not an example.com, but on www.example.com), you can safely use
     another subdomain for your CDN.
  2) When you are using cookies on your main domain (example.com), you'll have 
     to use a completely different domain for the CDN if you don't want 
     cookies to be sent.
     So then you should use the CDN's URL (e.g. myaccount.cdn.com). But now 
     you should be careful to avoid JavaScript issues: you may run into "same 
     origin policy" problems. See admin/settings/cdn/other for details.

Drupal 7 no longer sets cookies for anonymous users. To achieve this in Drupal
6, you can use the "No Anonymous Sessions" module by kbahey
(http://drupal.org/project/no_anon).

If you just use the CDN's URL (e.g. myaccount.cdn.com), all cookie issues are
avoided automatically.


When using multiple servers/CDNs: picking one based on advanced criteria
------------------------------------------------------------------------
You only need this when you're using multiple servers/CDNs and you can't rely
on picking a server/CDN based on the file extension, i.e. if you need more
advanced criteria than only file extension.

NOTE: this function is only called for file X if >1 server/CDN is available
for file X.

For this purpose, you can implement the cdn_pick_server() function:
  /**
   * Implementation of cdn_pick_server().
   */
  function cdn_pick_server($servers_for_file) {
    // The data that you get - one nested array per server from which the file
    // can be served:
    //   $servers_for_file[0] = array('url' => 'http://cdn1.com/image.jpg', 'server' => 'cdn1.com')
    //   $servers_for_file[1] = array('url' => 'http://cdn2.net/image.jpg', 'server' => 'cdn2.net')

    $which = your_logic_to_pick_a_server();

    // Return one of the nested arrays.
    return $servers_for_file[$which];
  }

So to get the default behavior (pick the first server found), one would write:
  /**
   * Implementation of cdn_pick_server().
   */
  function cdn_pick_server($servers_for_file) {
    return $servers_for_file[0];
  }

Or if you want to balance the number of files served by each CDN (i.e. on
average, each CDN serves the same amount of files on a page) instead of
picking the CDN based purely on filetype, one could write:
  /**
   * Implementation of cdn_pick_server().
   */
  function cdn_pick_server($servers_for_file) {
    $filename = basename($servers_for_file[0]['url']);
    $unique_file_id = hexdec(substr(md5($filename), 0, 5));
    return $servers_for_file[$unique_file_id % count($servers_for_file)];
  }

Note: if you don't want to create a small module for this function, or if you
      would just like to experiment with this function, you can also enter the
      body of this function at admin/settings/cdn/other — it will work exactly
      the same!
      If you don't know what the "body" of a function is, it's the part
      between the curly brackets:
        function doSomething() {
          BODY
        }
      So, in the case of the cdn_pick_server() function, this is the body that
      you would enter:
        $filename = basename($servers_for_file[0]['url']);
        $unique_file_id = hexdec(substr(md5($filename), 0, 5));
        return $servers_for_file[$unique_file_id % count($servers_for_file)];


Supporting the CDN integration module in your modules and themes
----------------------------------------------------------------
When your module already uses file_create_url() (either because it supports
user-uploaded content or generates files), then there is nothing that you have
to do. It is recommended though to include the file_directory_path() in the
$path you're passing. I.e., this is supported, but not recommended and even
deprecated in Drupal 7 core
  file_create_url('test.jpg');
This will return "sites/default/files/test.jpg" (assuming that you've
configured "sites/default/files" to be your files directory, i.e. the value
that file_directory_path() returns). But it's recommended to do the following
instead, since that will make porting to Drupal 7 easier:
  file_create_url(file_directory_path() . '/test.jpg);

Theme developers: avoid using base_path(). I.e. instead of doing this:
  base_path() . path_to_theme() .'/fix-ie.css"
You should do this:
  file_create_url(path_to_theme() .'/fix-ie.css')


When images generated by ImageCache appear to be broken
-------------------------------------------------------
You're likely using Varnish and an Origin Pull CDN.

Try configuring your Push CDN to access the non-Varnish port (i.e. the port
of the regular web server), typically port 8080. Not all CDNs support this
though.

Related issues:
- http://drupal.org/node/862880
- http://drupal.org/node/570132#comment-4882590


Notes on the backport of hook_file_url_alter() for Drupal 6
-----------------------------------------------------------
An identical backport is impossible because in Drupal 7, there's the new
File API which uses PHP stream wrappers. It's impossible to backport the
entire new File API.
- Issue for Drupal 7 core patch:
    http://drupal.org/node/499156
- The backport of hook_file_url_alter() is based on the patch *before* stream
  wrapper support went in, i.e. the patch in this comment:
    http://drupal.org/node/499156#comment-1866878
    http://drupal.org/files/issues/cdn-integration-499156-62.patch
- Documentation for hook_file_url_alter() for Drupal 7:
    http://api.drupal.org/api/function/hook_file_url_alter/7
- For analogous documentation for the Drupal 6 backport, see the included
    patches/hook_file_url_alter.php


Author
------
Wim Leers ~ http://wimleers.com/

Version 1 of this module (for Drupal 6) was written as part of the bachelor
thesis of Wim Leers at Hasselt University.

http://wimleers.com/tags/bachelor-thesis
http://uhasselt.be/
