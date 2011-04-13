<?php
// $Id: node-table.tpl.php,v 1.5 2007/10/11 09:51:29 tow Exp $
?>
<div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?>">
  
  <?php if ($page == 0): ?>
    <h2><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></h2>
  <?php endif; ?>
  
  <div class="clear-block">
    <div class=field-label">Description:&nbsp;</div>
    <div class="table-description"><?php print $content ?></div>
  </div>
  
  <div class="clear-block">
    <?php if ($structure): ?>
      <div class="table-structure">
        <?php print $structure; ?>
      </div>
    <?php endif; ?>
  </div>

</div>