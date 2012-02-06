<pre>
<?php
//if ($id == 10) 
  //print_r(get_defined_vars());
//  dpm($result);
?>
</pre>

<dt class="title">
  <a href="<?php print $url; ?>"><?php print $title; ?></a>
</dt>
<dt class="body">
  <?php print $result['teaser']; ?></a>
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

    <div class="dataset-badge">
      <div class="posted-date">
        <?php
          print $result['posted_date']; 
        ?>
      </div>
      <div class="user-link">
        <?php
          print $result['user']; 
        ?>
      </div>
      <div class="user-picture">
        <?php
          print $result['picture']; 
        ?>
      </div>
    </div>

    <div class="taxonomy">
      <div class="category">
        <?php
          print t('category: ') . $result['category'];
        ?>
      </div>
	  
      <?php
		print 'tags: ';
        foreach ($result['tags'] as $term) {
			print '<div class="tags">' . $term . '</div>'; 
		}
      ?>
      
    </div>
</div>

</dd>