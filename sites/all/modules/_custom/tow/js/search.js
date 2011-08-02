Drupal.behaviors.Search = function(context) {
	$("#edit-tables-select option").each(function () {
		// hide tables to be displayed
		id = "#edit-fields-select-tables-" + $(this).val() + "-wrapper";
		$(id).hide();
		// hide fields to be displayed 
		id = 'div[id*="t' + $(this).val() + '"]';
		$(id).hide();
		// hide constraint fields 		
		id = 'option[value*="t' + $(this).val() + '"]';
		$(id).hide();
	  }); 
	
  $('.remove-filter').click(function() {
    $(this).parent().hide();
    //$(this).parent().children('.value-field').children('div').children('input').val('4mpWbjErTjPdVwao');
  });

  $('.clear-form div').click(function(){
    $('input.form-text').val('');
    $(' input.form-checkbox').attr('checked', false);
    $('option:selected').removeAttr('selected');
    
    $('#filter-items-wrapper').children().slice(1).hide();
  });
  
  $('#edit-tables-select').change(function() {
	  var str = "";
	  var id = "";
	  //alert($(this).text());
	  
	  $("#edit-tables-select option").each(function () {
		// hide tables to be displayed
		id = "#edit-fields-select-tables-" + $(this).val() + "-wrapper";
		$(id).hide();
		// hide fields to be displayed 
		id = 'div[id*="t' + $(this).val() + '"]';
		$(id).hide();
		// hide constraint fields 		
		id = 'option[value*="t' + $(this).val() + '"]';
		$(id).hide();
	  }); 
	  $("#edit-tables-select option:selected").each(function () {
		// show tables to be displayed
		id = "#edit-fields-select-tables-" + $(this).val() + "-wrapper";
		$(id).show();
		// show fields to be displayed 
		id = 'div[id*="t' + $(this).val() + '"]';
		$(id).show();
		// show constraint fields 
		id = 'option[value*="t' + $(this).val() + '"]';
		$(id).show();
		
	  });
  });
};
