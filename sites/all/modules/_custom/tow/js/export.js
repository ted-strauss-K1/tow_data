Drupal.behaviors.Export = function(context) {
	  
	// hide and show table select dependently on user choice (whether export whole dataset or some tables)
  $('#edit-to-table').hide();
  $('#edit-to-way-1-wrapper').change(function() {
    $('#edit-to-table').show();
  });
  $('#edit-to-way-0-wrapper').change(function() {
    $('#edit-to-table').hide();
  });
	  
};