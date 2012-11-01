Drupal.behaviors.record_datepicker = function(context) {
  //Check each input date/datetime
  $('div.form-item input.form-text').each(function() {
    var inputType = $(this).closest('tr').children('td').eq(1).text();
	inputTypeToCheck = $.trim(inputType);
  
  //date: hide hh:mm:ss, apply datepicker
	if (inputTypeToCheck == 'date') {
    var curFormat = $(this).val();
    var coolFormat = curFormat.substr(0,10);
    $(this).val(coolFormat);
	  
    $(this).datepicker({ 
		dateFormat: "yy-mm-dd"
      });
    }
	//datetime: apply datetimepicker
  else if (inputTypeToCheck == 'datetime') {
	  
    $(this).datetimepicker({ 
		dateFormat: "yy-mm-dd",
		timeFormat: "hh:mm:ss",
      });
    }
	
  });
  
}