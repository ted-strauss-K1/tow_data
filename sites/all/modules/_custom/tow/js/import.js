// Copyright Theory on Wheels

// User interface adjustment for import page
Drupal.behaviors.import = function(context) {
	
  // hide and show table select dependently on user choice (whether import to dataset or to table)
  $('#edit-to-table').hide();
  $('#edit-to-way-1-wrapper').change(function() {
    $('#edit-to-table').show();
  });
  $('#edit-to-way-0-wrapper').change(function() {
    $('#edit-to-table').hide();
  });
  
  //hide progress bar
  $('.progress-bar').hide();
  
  var hash = Math.round(Math.random()*1000000);
  
  // For Firefox >= 3.6 add drag&drop feature 
  if ($.browser.mozilla == true && parseInt($.browser.version.replace(/\./g, "")) > 19200)
	dragFile(hash);
  else
	$('#edit-upload-wrapper div.description').hide();
  
  /*
  var lpInit = function(hash) {
    var hash = Math.round(Math.random()*1000000);
	$.ajax({
	  url: 'http://' + location.host + '/?q=import_progress_init/' + hash,
	  dataType: 'json',
	  success: lpStart(hash, 1, 3)
	});
  };
  */
  
  var lpStart = function(hash, stage, cycle) {
	
	cycle++;
	if (cycle > 100)
	  return;
	
	$.ajax({
	  url: 'http://' + location.host + '/?q=import_progress_get/' + hash + '/' + stage,
	  dataType: 'json',
	  success: function (data, textStatus) {
		lpOnComplete(hash, stage, data.response, cycle);
	  }
    });
  };
  
  var lpOnComplete = function(hash, stage, response, cycle) {

	var dots;
	var nextStage = stage;
	
	if (stage <= 3) {
	
      if (response == 10) {
	    $('.progress-table tr.' + stage + ' td.dots').text('..........');
	    $('.progress-table tr.' + stage + ' td.status').text('ok');
	    $('.progress-table tr.' + stage + ' td.status').addClass('ok');
	    nextStage++;
	  }
	    	
      else if (response == -1) {
	    $('.progress-table tr.' + stage + ' td.status').text('error');
	    $('.progress-table tr.' + stage + ' td.status').addClass('error');
	    nextStage = 4;
	  }
	  
      else {
	    dots = '';    
	    for (var i = 1; i <= response; i++)
	      dots += '.';
	    $('.progress-table tr.' + stage + ' td.dots').text(dots);
      }
    
	} 
	
	else if (stage >= 4 && stage <= 5){
	  
	  if (response != false) {	    
		
		if (response !== '-1') {
		  $('.progress-table tr.' + stage + ' td.message').text(response);
		  $('.progress-table tr.' + stage).show();
		}
	    nextStage++;
	  }
	}
	
	else {
	  
	  if (response != false) {
	    $('.progress-table tr.6 td.message').text(response);
	    $('.progress-table tr.6 td.status').text('ok');
	    $('.progress-table tr.6 td.status').addClass('ok');
	    $('.progress-table tr.6').show();
	    $('.progress-table tr.7').show();
	    return;
	  }
	}
	
	setTimeout(
	  function() {
	    lpStart(hash, nextStage, cycle);
	  }, 1000);
  
  };
  
  function dragFile(hash) {
	$('#edit-upload').filedrop({
      url: 'upload_dragged',
      paramname: 'files',
      data: { 
        hash: hash, 			// send POST variables
        table: function() {
		  if ($('#edit-to-way-1').attr('checked') == 'checked')
		    return $('#edit-to-table').val();
		  else
			return false;
	    },
        dataset: location.pathname.replace(/\import/g, "").replace(/\//g, "")         
      },
      uploadStarted: function(i, file, len){
    	  $('.progress-bar').show();
    	  lpStart(hash, 1, 0);
      },
      maxfiles: 1,
      maxfilesize: 10
    });
  };
  
};

