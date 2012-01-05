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
  
  var hash = Math.round(Math.random()*1000000), 
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
    if (cycle > 2000)
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

    var dots,
      nextStage = stage;
	
    if (stage <= 3) {
	
      if (response == 10) {
        $('.progress-table tr.' + stage + ' td.dots').text('..........');
        $('.progress-table tr.' + stage + ' td.status').text('ok');
        $('.progress-table tr.' + stage + ' td.status').addClass('ok');
        nextStage++;
	  }
	    	
      else 
        
    	if (response == -1) {
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
	
    else

      if (stage >= 4 && stage <= 5){
	  
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

          var ind = response.indexOf("/"),
              message = response.slice(0, ind) + ' ',
              tables = response.slice(ind+1);

          ind = tables.indexOf("/");

          var nids = tables.slice(ind+1);
          tables = tables.slice(0, ind);
            
          nids = explode(' ', trim(nids));
          tables = explode(' ', trim(tables));
            
          for(var n=0; n<tables.length; n++) {
            if (nids[n])
        	  message += '<a href="' + location.protocol + '//' + location.host + '/table/' + nids[n] + '">' + tables[n] + '</a>' + '; ';
          }
          
          message = rtrim(message);
          message = rtrim(message, ';');
          $('.progress-table tr.6 td.message').html(message);
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
    
    $('#edit-upload').filedrop({
        
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
        uri = 'upload_selected/' + hash + '/' + dataset + '/' + getChosenTable(),    
        xhr = new XMLHttpRequest()
		fData = new FormData();
    
    xhr.open("POST", uri, true);
			$('.progress-bar').show();
			lpStart(hash, 1, 0);

	fData.append(fileName, file);
    xhr.send(fData);
	
    return true;
  }
  
  function explode( delimiter, string ) { // Split a string by string
	  	    //
	  	    // +   original by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
	  	    // +   improved by: kenneth
	  	    // +   improved by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
	  	 
	  	    var emptyArray = { 0: '' };
	  	 
	  	    if ( arguments.length != 2
	  	        || typeof arguments[0] == 'undefined'
	  	        || typeof arguments[1] == 'undefined' )
	  	    {
	  	        return null;
	  	    }
	  	 
	  	    if ( delimiter === ''
	  	        || delimiter === false
	  	        || delimiter === null )
	  	    {
	  	        return false;
	  	    }
	  	 
	  	    if ( typeof delimiter == 'function'
	  	        || typeof delimiter == 'object'
	  	        || typeof string == 'function'
	  	        || typeof string == 'object' )
	  	    {
	  	        return emptyArray;
	  	    }
	  	 
	  	    if ( delimiter === true ) {
	  	        delimiter = '1';
	  	    }
	  	 
	  return string.toString().split ( delimiter.toString() );
	  }
  
  function trim(str, chars) { 
		return ltrim(rtrim(str, chars), chars); 
	} 
	 
	function ltrim(str, chars) { 
		chars = chars || "\\s"; 
		return str.replace(new RegExp("^[" + chars + "]+", "g"), ""); 
	} 
	 
	function rtrim(str, chars) { 
		chars = chars || "\\s"; 
		return str.replace(new RegExp("[" + chars + "]+$", "g"), ""); 
	}

};