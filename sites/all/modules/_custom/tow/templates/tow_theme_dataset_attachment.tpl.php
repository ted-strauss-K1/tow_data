<?php
// $Id: tow_theme_dataset_attachment.tpl.php,v 1.0.0.0 2011/02/28 23:42:15 tow Exp $
?>
<div class="dataset-description">
  <div class="clear-block">
    <div class=field-label">Description:&nbsp;</div>
    <?php print $description ?>
    <span class="edit-link"><?php print $edit_description; ?></span>
  </div>

  <div class="clear-block">
    <div class=field-label">Categories:&nbsp;</div>
    <?php print $categories ?>
    <span class="edit-link"><?php print $edit_categories; ?></span>
  </div>
</div>