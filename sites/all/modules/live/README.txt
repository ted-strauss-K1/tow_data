About
-----
Introduces live previews and other interactive features into a Drupal site. Live previews for comments and nodes are securely handled by performing permission checks and passing the content through filters as per the permitted input format chosen. http://groups.drupal.org/ is one of the most active website using live module(for comments).

Install
-----
1. Copy the live-6.x-X.X.tar.gz contents to sites/SITENAME/modules directory (usually sites/all/modules).

2. Enable the Live module at the modules administration page at http://www.example.com/drupal/?q=admin/build/modules.

3. Enable live previews for comments or the content types you want to use it on, in the live module's configuration at http://www.example.com/drupal/?q=admin/settings/live.

4. Set the permissions for accessing live previews for various user roles at http://www.example.com/drupal/?q=admin/user/permissions.

5. Your module is now set up and ready to be used.

Troubleshooting
-----
Q. Why are the node previews looking scrambled or rendering incorrectly?
A. Previews of node on "default" setup of content types are fully possible. Note that it has only been tested with Page and Story content types, and hence currently supported with. CCK is not supported since it would need a lot of customizations. For full support for your custom content types, or some complex validation you might want to hack into live.node.inc and the companion JS file.

Q. What does "Error requesting data!" notice mean?
A. This error may occur when either the user is not permitted to use the chosen input format, or is not permitted to use live previews, or if the client is unable to connect to the server. Another possible reason could be incompatibility with old version of Global Redirect module (http://drupal.org/project/globalredirect); the solution to which is to upgrade to the latest release(at least version 6.x-1.0). If the error still pops unexpectedly, it might be a bug worth reporting in the live project's issues, along with incompatible module/feature, if any.

Sponsors
-----
Interestingly, this module sprouted out of my own interest. :)

Credits
-----
Actively developed and maintained by Gurpartap Singh <gurpartap@gmail.com>.

Support
-----
If you experience a problem with live module, file a request or issue on the websnapr queue at http://drupal.org/project/issues/live. DO NOT POST IN THE FORUMS. Posting in the issue queues is a direct line of communication with the module authors.
