if (Drupal.jsEnabled) {
  $(document).ready(function() {
  	// Find the calendars and set the DOM up properly
    $("input[@type='text'].yuicalendar").each(function () {
      var id = $(this).attr('id');
      var form = this.form;
      var iconDiv = document.createElement('div');
      var calDiv = document.createElement('div');
	  var autoDiv = document.createElement('div');
	  
      // Calendar icon
      $(iconDiv)
        .html(' ... ')
        .attr('id', id + '-icon')
        .addClass('yuicalendar-icon');
      $('#' + id + '-day')
        .after(iconDiv);

      // Calendar
      $(calDiv)
        .attr('id', id + '-calendar');
      $(iconDiv)
        .after(calDiv);

      // Time Auto Complete
	  $(autoDiv)
	    .attr("id", id + "-time-container");
	  $('#' + id + "-time")
	    .after(autoDiv);

      YAHOO.util.Event.onContentReady(YAHOO.drupal.calendar.init(id));
    });
  });
}
