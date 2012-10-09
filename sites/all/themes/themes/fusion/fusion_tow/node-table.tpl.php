<?php
// $Id: node-table.tpl.php,v 1.5 2007/10/11 09:51:29 tow Exp $
?>
<div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?>">
  
  <?php if ($page == 0): ?>
    <h2><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></h2>
  <?php endif; ?>

  <?php if ($structure['clone']): ?>
    <div class="clear-block">
      <div class="clone-table">
        <?php print $structure['clone']; ?>
      </div>
      <div class="delete-table">
        <?php print $structure['delete']; ?>
      </div>
    </div>
  <?php endif; ?>  
    
  <div class="clear-block">
    <div class=field-label">Description:&nbsp;</div>
    <div class="table-description"><?php print $content ?></div>
    <?php if ($structure['edit_description']): ?>
      <span class="edit-link"><?php print $structure['edit_description']; ?></span>
    <?php endif; ?>
  </div>
  
  <div class="clear-block">
    <?php if ($structure['table_stats']): ?>
      <div class="table-statistics">
        <?php print $structure['table_stats']; ?>
      </div>
    <?php endif; ?>
  </div>
  
  <div class="clear-block">
    <?php if ($structure['structure']): ?>
      <div class="table-structure">
        <?php print $structure['structure']; ?>
      </div>
    <?php endif; ?>
  </div>
  
  <?php if ($structure['field_add']): ?>
    <div class="clear-block">
      <?php print $structure['field_add']; ?>
    </div>
  <?php endif; ?>

</div>