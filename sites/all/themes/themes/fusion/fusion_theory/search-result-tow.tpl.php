<?php
//if ($id == 10) 
  //print_r(get_defined_vars());
//  dpm($result);
?>
<div class="rows">
        	<div class="date_vote">
            	<div class="votes">
                 <?php
					  if ($result['votes']) {
						print '<b>' . $result['votes'] . '</b>' . '</br>' . t('<span>votes</span> ');
					  }
					  else {
						print t('<b>0</b> </br> <span>votes</span>');
					  }
				?>
                <!--<b>420</b><br/><span>Votes</span>-->
                </div>
                <div class="searches">
                	<?php
					  print '<b>' . $result['ssearches'] . '</b>' . '</br>' . t('<span>searches</span> ');
					?>
                <!--<b>18</b><br/><span>Searches</span>-->
                </div>
                <div class="rec_views">
                	<?php
					  if ($result['views']) {
						print $result['views'] . '&nbsp;' . t('Views');
					  }
					  else {
						print t('0 Views');
					  }
					?>
                <!--15,054 Views-->
                </div>
            </div>
            <div class="rec_detail">
            	<div class="title"><a href="<?php print $url; ?>"><?php print $title; ?></a></strong></div>
                <div class="desc"><?php print $result['teaser']; ?></div>
                <div class="tags_btns">
                <?php
				if($result['category'])
				{
					 print '<div class="btn btn-inverse btn-small">' . $result['category'] . '</div>';
				}
				else
				{
					echo "test";
				}
				?>
                	<?php
					//print 'tags: ';
					$result['tags'] = isset($result['tags']) ? $result['tags'] : array();
					foreach ($result['tags'] as $term) {
					  print '<div class="btn btn-small">' . $term . '</div>'; 
					}
				  ?>
                </div>
                
                <div class="posted">
                	<div class="posted_date"><?php print $result['posted_date']; ?></div>
                    <div class="user_img"><?php print $result['picture']; ?></div>
                    <div class="user_name">
                    <span class="name"><?php print $result['user']; ?></span><br/>
                    <span>
					<?php
						  print $result['points']; 
						?>
                    </span>
                    </div>
                </div>
                
                <span class="rec_info"><span class="label"><?php print $result['amount_tables']; ?></span> - <span class="label"><?php print $result['amount_records'];  ?></span> - <span class="label"><?php print t('access: ') . $result['access']; ?></span></span>
                
            </div>
</div>





<?php /*?><dt class="title">
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
        $result['tags'] = isset($result['tags']) ? $result['tags'] : array();
        foreach ($result['tags'] as $term) {
          print '<div class="tags">' . $term . '</div>'; 
        }
      ?>
      
    </div>
</div>

</dd><?php */?>