Drupal.behaviors.record_datepicker = function(context) {
  //Date and datetime datetimepicker
  $('div.form-item input.form-text').each(function() {
    var inputType = $(this).closest('tr').children('td').eq(1).text();
	inputTypeToCheck = $.trim(inputType);
    
	if (inputTypeToCheck == 'date') {
	  $(this).datetimepicker({ 
		dateFormat: "yy-mm-dd",
		timeFormat: "hh:mm:ss",
		hourMin: 0,
		hourMax: 0,
		minuteMin: 0,
	    minuteMax: 0
      });
    }
	else if (inputTypeToCheck == 'datetime') {
	  $(this).datetimepicker({ 
		dateFormat: "yy-mm-dd",
		timeFormat: "hh:mm:ss",
      });
    }
	
  });
  
}