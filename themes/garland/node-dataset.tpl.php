<?php
// $Id: node.tpl.php,v 1.5 2007/10/11 09:51:29 goba Exp $
?>
<div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?>">

<?php print $picture ?>

<?php if ($page == 0): ?>
  <h2><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></h2>
<?php endif; ?>

  <div class="content clear-block">
    <div class=field-label">Description:&nbsp;</div>
    <div class="dataset-description"><?php print $content ?></div>
    <?php if ($summary['edit_description']): ?>
      <span class="edit-link"><?php print $summary['edit_description']; ?></span>
    <?php endif; ?>
  </div>

  <div class="clear-block">
    <div class="meta">
    <?php if ($taxonomy): ?>
      <div class="field-label">Categories:&nbsp;</div>
      <div class="terms"><?php print $terms ?></div>
    <?php endif;?>
    <?php if ($summary['edit_categories']): ?>
      <span class="edit-link"><?php print $summary['edit_categories']; ?></span>
    <?php endif; ?>
    </div>
  </div>

  <div class="clear-block">
    <?php if ($summary['tables']): ?>
       <div class="tables-list">
         <div class="field-label">Tables:&nbsp;</div>
         <?php print($summary['tables']); ?>
      </div>
    <?php endif;?>
  </div>
   
  <div class="clear-block"> 
    <?php if ($summary['tables']): ?>
    <div class="sample-tables">
      <span class="field-label">Random sample of data from this set</span>
      <span>&nbsp;(<a href= "<?php print $nid; ?>">Refresh</a> for a new sample)</span>
      
      <div class="random-table">
        <?php
         print($summary['view_table'][0]['title']);
         print($summary['view_table'][0]['table']); 
        ?>
      </div>
      
      <?php if ($summary['view_table'][1]) ?>
        <div class="random-table">
          <?php
           print($summary['view_table'][1]['title']);
           print($summary['view_table'][1]['table']); 
          ?>
        </div>
      <?php endif; ?>
    </div>
  </div>
  
  <div class="clear-block">
    <?php if ($summary['admin_tasks']): ?>
      <div class="administrator-tasks">
        <div class="field-label">Administrator tasks:&nbsp;</div>
        <?php print $summary['admin_tasks']; ?>
      </div>
    <?php endif; ?>
  </div> 
  
</div>