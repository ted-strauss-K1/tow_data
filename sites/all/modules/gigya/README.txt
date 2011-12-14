// $Id: README.txt,v 1.5 2010/08/09 12:39:33 add1sun Exp $

Credits
=======
Development of this module has been sponsored by Gigya.
The 3.x version development was lead by azink (http://drupal.org/user/518662)
with assistance from add1sun (http://drupal.org/user/65088) of
Lullabot (http://www.lullabot.com).

Authentication
==============
In order to properly authenticate users, the Gigya module
must be set up. You can configure it through Administer->Site
configuration->Gigya Socialize Settings (admin/settings/gigya).
Make sure you have the following
  * Site API Key
  * Your user private key from Gigya
  * Selected social networks

You can further customize the Gigya UI elements by looking at the
subsections for Login and Connect. If Profile or Content Profile module
is enabled, you can select which Gigya social network information should 
be mapped to your local profile fields. These profile mappings are only
applicable to fields which are displayed on the user registration form.

Please refer to http://wiki.gigya.com/050_Socialize_Plugins/020_Drupal
for more info.

Configure Share UI of Gigya Socialize
Please refer to http://wiki.gigya.com/050_Socialize_Plugins/020_Drupal
for more info.

Q: I connected all my social networks to my drupal account, and now when
I connect with Gigya and link to drupal, they all disappear!
A: This is normal operation. Unfortunately you'll have to re-connect
to your social networks if you use Gigya to login rather than Drupal.
