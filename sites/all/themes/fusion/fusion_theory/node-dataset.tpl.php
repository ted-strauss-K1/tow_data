<?php
// $Id: node-dataset.tpl.php,v 1.5 2011/04/13 09:51:29 tow Exp $
?>
<div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?>">

<?php print $picture ?>

<!--<?php if ($page == 0): ?>
  <h2><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></h2>
<?php endif; ?>-->

  <div class="content clear-block">
    <div class="dataset-description"><?php print $content ?></div>
    <?php if ($summary['edit_description']): ?>
      <span class="edit-link"><?php print $summary['edit_description']; ?></span>
    <?php endif; ?>
  </div>

  <div class="clear-block">
    <div class="meta">
    <?php if ($taxonomy): ?>
      <div class="field-label">Keywords:&nbsp;</div>
      <div class="terms"><?php print $terms ?></div>
    <?php endif;?>
    <?php if ($summary['edit_categories']): ?>
      <span class="edit-link"><?php print $summary['edit_categories']; ?></span>
    <?php endif; ?>
    </div>
  </div>

  <div class="clear-block">
    <?php if ($summary['tables']): ?>
       <div class="tables-stats">
         <?php 
           print($summary['tables_stats'][0]['title']); 
           print($summary['tables_stats'][0]['data']);
         ?>
      </div>
    <?php endif;?>
  </div>
  
  <div class="clear-block">
    <?php if ($summary['access_widget']): ?>
      <div class="access-widget">
        <?php print $summary['access_widget']; ?>
      </div>
    <?php endif; ?>
  </div>
  
 
  
  <div class="clear-block"> 
    <?php if ($summary['tables']): ?>
    <div class="sample-tables">
      <span class="field-label">Some random samples from this data set</span>
      <span>&nbsp;(<a href= "<?php print $nid; ?>">refresh</a> for a new sample)</span>
      
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
    </div>
	<?php endif; ?>
  </div>
  
  <div class="clear-block">
    <?php if ($summary['saved_searches']): ?>
      <div class="saved-searches">
    	<span class="saved-searches-label">Saved searches (<?php print $summary['saved_searches']['link_to_full_list']?>)</span>     
        <?php print $summary['saved_searches']['short_list']; ?>
      </div>
    <?php endif; ?>
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
