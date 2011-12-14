<pre>
<?php
//if ($id == 10) 
//  print_r(get_defined_vars());
?>
</pre>
<?php 
if ($result['node']->type == 'dataset') { 
?>
  <dt class="title">
    <a href="<?php print $url; ?>"><?php print $title; ?></a>
  </dt>
  <dd>
    <div class="statistics">
      
      <div class="tables-records-savedsearches">
        <div class="amount-tables">
          <?php
            print $result['amount_tables']; 
          ?>
        </div>
        <div class="amount-records">
          <?php
            print $result['amount_records']; 
          ?>
        </div>
      </div>
      
      <div class="views-bookmarks">
        <div class="amount-views">
          <?php
            print $result['amount_views']; 
          ?>
        </div>
      </div>
    </div>

  </dd>
<?php  }
else {
?>
  <dd>
    <div class="found-tables">
      <div class="found-table">
        <?php
          print($result['title']); 
          print($result['table']); 
        ?>
      </div>
    </div>
  </dd>
<?php  
}
?>