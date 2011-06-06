// Copyright Theory on Wheels

// User interface adjustment for import page
Drupal.behaviors.Import = function(context) {
	
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
  
  var 	hash = Math.round(Math.random()*1000000),
  		dataset = location.pathname.replace(/\import/g, "").replace(/\//g, ""),
  		lock = false;
  
  // For Firefox >= 3.6 add drag&drop feature 
  if ($.browser.mozilla == true && parseInt($.browser.version.replace(/\./g, "")) > 19200)
	dragFile(hash);
  else
	$('#edit-upload-wrapper div.description').hide();
  
  $('#edit-upload').change(function(){
	  
	  if (lock)
		  return;
	  lock = true;
	  
	  var files = this.files; 
	  if (files.length != 1)
		  return;
	  fileUpload(hash, files[0]);
  });
  
  $('.get-it').click(function(){
	  
	  if (lock)
		  return;
	  lock = true;
	  
	  var grab_url = $('#edit-url').val(); 
	  if (grab_url != "") {
		  var table = getChosenTable(); 
		  $.ajax({
			  url: 'http://' + location.host + '/?q=import_grab/' + hash + '/' + dataset + '/' + table,
			  data: {grab_url : grab_url},
			  beforeSend: function() {
		    	  $('.progress-bar').show();
		    	  lpStart(hash, 1, 0);
			  }
		  });
	  }
  });
  
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
  
  function getChosenTable() {
	if ($('#edit-to-way-1').attr('checked') == 'checked')
      return $('#edit-to-table').val();
    else
      return false;
  }
  
  function dragFile(hash) {
    $('#edit-upload').filedrop(
      {
        url: 'upload_dragged',
        paramname: 'files',
        data: { 
          hash: hash, 			// send POST variables
          table: function() {
    	    return getChosenTable();
    	  },
          dataset: dataset         
        },
		uploadStarted: function(i, file, len){
    	  $('.progress-bar').show();
    	  lpStart(hash, 1, 0);
		},
		maxfiles: 1,
		maxfilesize: 10,
		beforeEach: function(file) {
			if (lock == false) {
				lock = true;
				return true;
			}
			else
				return false;
		}
    });
  };
  
  
  function fileUpload(hash, file) {	    
    var fileName = file.name,
    fileSize = file.size,
    fileData = file.getAsBinary(), // works on TEXT data ONLY.
    boundary = "xxxxxxxxx",
    uri = 'upload_selected/' + hash + '/' + dataset + '/' + getChosenTable(),
    
    xhr = new XMLHttpRequest();
    xhr.open("POST", uri, true);
    xhr.setRequestHeader("Content-Type", "multipart/form-data, boundary="+boundary); // simulate a file MIME POST request.
    
    xhr.setRequestHeader("Content-Length", fileSize);

    xhr.onreadystatechange = function() {
      if (xhr.readyState == 1) {
    	  $('.progress-bar').show();
    	  lpStart(hash, 1, 0);
      }
      if (xhr.readyState == 4) {
        if ((xhr.status >= 200 && xhr.status <= 200) || xhr.status == 304) {
	  	         
          if (xhr.responseText != "") {
            alert(xhr.responseText); // display response.
          }
        }
      }
    };    
    
    var body = "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name='files'; filename='" + fileName + "'\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";
    body += fileData + "\r\n";
    body += "--" + boundary + "--";

    xhr.send(body);
    
    return true;
  }
  
};