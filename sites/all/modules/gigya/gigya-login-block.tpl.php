<?php
// $Id: gigya-login-block.tpl.php,v 1.1 2010/08/04 14:52:05 add1sun Exp $

/**
 * @file gigya-login-block.tpl.php
 * Default theme implementation for displaying a gigya login form.
 *
 * Available variables:
 * - $login_div: Contents of the login block.
 * - $title: Title to display, if not suppressed.
 *
 * @see template_preprocess_gigya_login_block()
 */
?>
 
<?php if ($title) : ?>
  <h3><?php print $title; ?></h3>
<?php endif; ?>

<div class="gigya-login"><?php print $login_div; ?></div>
