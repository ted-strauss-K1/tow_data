
CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Installation
 * Configuration
 * Multilangual sites
 * Compatible modules
 * De-installation

INTRODUCTION
------------

User Registration Password lets users register with a password on the
registration form when verification mail is required.

By default, users can create accounts directly on the registration form, set
their password and be immediately logged in, or they can create their account,
wait for a verification e-mail, and then create their password.

With this module, users are able to create their account along with their
password and simply activate their account when receiving the verification
e-mail by clicking on the activation link provided via this e-mail.

User Registration Password transforms the checkbox on the
admin/user/settings into a radio list with 3 options.

The first 2 are default Drupal behaviour:
0 Do not require a verification e-mail, and let users set their password on the registration form.
0 Require a verification e-mail, but wait for the approval e-mail to let users set their password.

The newly added option is:
X Require a verification e-mail, but let users set their password directly on the registration form.

The first 2 disable User Registration Password, only the 3rd option activates
the behaviour provided by this module.


INSTALLATION
------------

Installation is like any other module, just place the files in the
sites/all/modules directory and enable the module on the modules page.


CONFIGURATION
-------------

No need to configure the module, in the 7.x-1.x branch we changed the
installer to enable the correct configuration settings for us. This is
also backported to the Drupal 6.x-1.x branch. But if you want to
change something, these steps describe how to configure the module:


On the admin/user/settings page make sure you have selected:

Who can register accounts?
0 Administrators only
X Visitors
0 Visitors, but administrator approval is required

Then select 'Require a verification e-mail, but let users set their password directly on the registration form.' at:

Require e-mail verification when a visitor creates an account
0 Do not require a verification e-mail, and let users set their password on the registration form.
0 Require a verification e-mail, but wait for the approval e-mail to let users set their password.
X Require a verification e-mail, but let users set their password directly on the registration form.

The module is now configured and ready for use.
This is also the only way to configure it correctly.

You do not have to alter any e-mail templates, User Registration Password
overwrites the default 'Account activation e-mail' template from version
7.x-1.3 and from 6.x-1.0. So there is no need to change anything anymore.

If you have previously modified the activation e-mail template before you
installed this module and discovered that it overwrites the default
Activation e-mail template, no worries! The installer saves your changes
to the template to a temporally variable and revives them when you
disable AND uninstall User Registration Password. Your modifications are
revived and you can now copy paste them to a text file and re-install
User Registration Password again and make the changes to the account
activation e-mail template based on your previous version.


MULTILANGUAL SITES
------------------

TODO


COMPATIBLE MODULES
------------------

These modules have been confirmed to work with User Registration Password 6.x-1.x:
- U Create http://drupal.org/project/ucreate
- Invite http://drupal.org/project/invite
- Userplus http://drupal.org/project/userplus


DE-INSTALLATION
---------------

If you want to disable the module temporally, just select the first or second
option on the the admin/user/settings page at:

Require e-mail verification when a visitor creates an account
1 Do not require a verification e-mail, and let users set their password on the registration form.
2 Require a verification e-mail, but wait for the approval e-mail to let users set their password.
3 Require a verification e-mail, but let users set their password directly on the registration form.

This disables the User Registration Password functionality without
disabling / uninstalling it.

If you want to remove the module, just disable and uninstall it as you do
for any other module.


// Readme User Registration Password 6.x-1.x
