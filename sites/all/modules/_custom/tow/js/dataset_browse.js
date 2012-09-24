Drupal.behaviors.dataset_browse = function(context) {
  
  

  
  // 'Detail'-info for rows in DataTable.
  // Insert a 'details' column to the table.
  $('.views-table tr:last').remove();

	var nCloneTh = document.createElement( 'th' );
	var nCloneTd = document.createElement( 'td' );
	nCloneTd.innerHTML = '<img src="../../misc/menu-collapsed.png">';

	$('.views-table thead tr').each(function() {
    this.insertBefore(nCloneTh, this.childNodes[0]);
  });

	$('.views-table tbody tr').each(function() {
		this.insertBefore(nCloneTd.cloneNode(true), this.childNodes[0]);
	});
		
	var oTable = $('.views-table').dataTable({
    "aoColumnDefs": [
			{"bSortable": false, "aTargets": [0]},
			{"bVisible": false, "aTargets": [4]}
		],
		"aaSorting": [[1, 'asc']], 
		"bFilter": false,
		"bPaginate": false,
		"bInfo": false 
  });
		
		
	function fnFormatDetails(oTable, nTr) {
    var aData = oTable.fnGetData(nTr);
		var sOut = '<table cellpadding="5" cellspacing="0" border="0" style="padding-left:50px;">';
		sOut += '<tr><td>Description:</td><td>' + aData[4] + '</td></tr>';
		sOut += '</table>';

		return sOut;
	}

	// Initialse DataTables, with no sorting on the 'details' column
			
			/*var oTable = $('#datatable-1').dataTable( {
				"aoColumnDefs": [
					{ "bSortable": false, "aTargets": [ 0 ] }
				],
				"aaSorting": [[1, 'asc']]
			});*/
			
			/* Add event listener for opening and closing details
			 * Note that the indicator for showing which row is open is not controlled by DataTables,
			 * rather it is done here
			 */
			$('.views-table tbody td img').live('click', function () {
				var nTr = $(this).parents('tr')[0];
				if ( oTable.fnIsOpen(nTr) )
				{
					// This row is already open - close it 
					this.src = "../../misc/menu-collapsed.png";
					oTable.fnClose( nTr );
				}
				else
				{
					/* Open this row */
					this.src = "../../misc/menu-expanded.png";
					oTable.fnOpen( nTr, fnFormatDetails(oTable, nTr), 'details' );
				}
			} );
  

  // The same prcodure as above at the top
  // for another table with another amount of columns (dataset page)
  // 'Detail'-info for rows in DataTable.
  // Insert a 'details' column to the table.
if ($('#datatable-1 td img').size() == 0) {
	var nCloneTh = document.createElement( 'th' );
	var nCloneTd = document.createElement( 'td' );
	nCloneTd.innerHTML = '<img src="../../misc/menu-collapsed.png">';

	$('#datatable-1 thead tr').each(function() {
    this.insertBefore(nCloneTh, this.childNodes[0]);
  });

	$('#datatable-1 tbody tr').each(function() {
		this.insertBefore(nCloneTd.cloneNode(true), this.childNodes[0]);
	});
		
	var oTable = $('#datatable-1').dataTable({
    "aoColumnDefs": [
			{"bSortable": false, "aTargets": [0]},
			{"bVisible": false, "aTargets": [5]}
		],
		"aaSorting": [[1, 'asc']], 
		"bFilter": false,
		"bPaginate": false,
		"bInfo": false,
		"bRetrieve": true
  });
		
		
	function fnFormatDetails(oTable, nTr) {
    var aData = oTable.fnGetData(nTr);
		var sOut = '<table cellpadding="5" cellspacing="0" border="0" style="padding-left:50px;">';
		sOut += '<tr><td>' + aData[5] + '</td></tr>';
		sOut += '</table>';

		return sOut;
	}

			/* Add event listener for opening and closing details
			 * Note that the indicator for showing which row is open is not controlled by DataTables,
			 * rather it is done here
			 */
			$('#datatable-1 tbody td img').live('click', function () {
				var nTr = $(this).parents('tr')[0];
				if ( oTable.fnIsOpen(nTr) )
				{
					// This row is already open - close it 
					this.src = "../../misc/menu-collapsed.png";
					oTable.fnClose( nTr );
				}
				else
				{
					/* Open this row */
					this.src = "../../misc/menu-expanded.png";
					oTable.fnOpen( nTr, fnFormatDetails(oTable, nTr), 'details' );
				}
			} );  
			
			
	
	


  //AJAX 'refresh'
	$('div.sample-tables span a').live('click', function(e) {
		e.preventDefault();
		refresh_samples_ajax();
	});
    
	var refresh_samples_ajax = function () {
			var url = 'http://' + window.location.hostname + window.location.pathname + '/ajax/samples';
			url = url.replace('node', 'dataset');
			
		$.ajax({
			  url: url,
			  success: function(data) {
								
				var tableRefresh = data.samples;
				$('div.sample-tables').html(tableRefresh);
				
			  },
			  dataType: 'json'
			});
			
	};

	
	//AJAX access selection
	var confirmToSend;
	var accessToSend;
	var textAccess;
	var textToReplace;
	

	
	$('#edit-access-type').live('change', function() {
		//textToReplace = $('#edit-access-type option[value="'+accessToSend+'"]').text();
		accessToSend = $('#edit-access-type :selected').val();
		
		textAccess = $('#edit-access-type').parent().parent().children('div.form-item:first').text();
		var colon = textAccess.indexOf(":");
		var point = textAccess.indexOf(".");
		textToReplace = textAccess.substring(colon + 2, point);
		
		//var textAccess = $('#edit-access-type').parent().parent().children().children('div.form-item').text();
		
		if (accessToSend == 0) {
			confirmToSend = window.confirm('This will clear all information on granting and denial access to this dataset for other users. Do you want to continue?');
		}
		else {
			confirmToSend = true;
		}
		
		access_selection_ajax();
		
	});
	
	var access_selection_ajax = function () {
			var url = 'http://' + window.location.hostname + window.location.pathname + '/ajax/access';
			url = url.replace('node', 'dataset');
			
		$.ajax({
			  url: url,
			  data: {
				'confirm':confirmToSend,
				'access':accessToSend
			  },
			  success: function(data) {
				$('.administrator-tasks div.item-list').replaceWith(data.admin_tasks);
				$('#edit-access-type').val(data.selected);
				
				var textToInsert = $('#edit-access-type option[value="'+data.selected+'"]').text();
				var finalText = textAccess.replace(textToReplace,textToInsert);
				$('#edit-access-type').parent().parent().children('div.form-item:first').text(finalText);
				
				$('.dt-at span').text(textToInsert.toLowerCase());
			  },
			  dataType: 'json'
			});
			
	};
	
	
	//AJAX bookmarks count
	$(document).bind('flagGlobalAfterLinkUpdate', function(event, data) {
  		refresh_bookmark_ajax();
	});
	
	
	var refresh_bookmark_ajax = function () {
			var url = 'http://' + window.location.hostname + window.location.pathname + '/ajax/bookmark';
			url = url.replace('node', 'dataset');
			
		$.ajax({
			  url: url,
			  success: function(data) {
								
				var bookmarkRefresh = data.bookmark_count;
				$('div.starflag p').text('Bookmarks: ' + bookmarkRefresh);
			  },
			  dataType: 'json'
		});

	};
	
	//Show/hide Browse submenu with table links
  $('ul.browse-submenu').hide();
  $('ul.browse-submenu').prev().toggle(function(e) {
	e.preventDefault();
	$('ul.browse-submenu').show();
  },
  function(e) {
    e.preventDefault();
	$('ul.browse-submenu').hide();
  });
	
}	
	
  
}



