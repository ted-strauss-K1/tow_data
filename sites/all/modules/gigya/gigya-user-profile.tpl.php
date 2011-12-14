<?php
// $Id: gigya-user-profile.tpl.php,v 1.1 2010/08/04 14:52:05 add1sun Exp $

/**
 * @file gigya-user-profile.tpl.php
 * Default theme implementation for displaying gigya user profile information.
 *
 * Available variables:
 * - $bio: Array of network user information.
 * - $photo: User photo.
 * - $name: Formatted user nickname.
 *
 * @see template_preprocess_gigya_user_profile()
 */
?>

<div id="<?php print $bio['provider']; ?>_profile">

  <?php print $photo; ?>

  <?php if ($name) : ?>
    <div class="socialize_nickname">
    <?php print $name; ?>
    </div>
  <?php endif; ?>

</div>
  