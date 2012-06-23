Drupal.behaviors.outer_search = function(context) {
  var handleHeight = 15,
    handleWidth = 6,
    handleBorder = 1,
    visibleHandleHeight = 4,
    visibleHandleWidth = 5,
    visibleHandleBorder = 1,
    visibleHandleOffset = 2,
    visibleBoxBorder = 1,
    arrayOfCharts = {},
	arrayOfZooms = {},
	arrayOfCollapse = {};

  // hide hidden facets
  $('.apachesolr-hidden-facet', context).hide();
  
  // add Show more/fewer link
  var show_more_fewer = function() {
    $('<a href="#" class="apachesolr-showhide"></a>').text(Drupal.t('Show more')).click(function() {
    if ($(this).parent().find('.apachesolr-hidden-facet:visible').length == 0) {
    $(this).parent().find('.apachesolr-hidden-facet').show();
    $(this).text(Drupal.t('Show fewer'));
    }
    else {
    $(this).parent().find('.apachesolr-hidden-facet').hide();
    $(this).text(Drupal.t('Show more'));
    }
    return false;
    }).appendTo($('.item-list:has(.apachesolr-hidden-facet)', context));
  };  
  
  show_more_fewer();
  
    
  //simple search
  $('#edit-keywords').change(function() {
    var keywords = $(this).val();//.replace(/(\+|-|&&|\|\||!|\(|\)|\{|\}|\[|\]|\^|"|~|\*|\?|:|\\)/g, '\\$1');
    keywords = encodeURIComponent(keywords);
    window.location.href = 'http://' + location.host + '/search_dataset/' + $(this).parent().parent().children('#edit-nid').val() + '/' + keywords + location.search;
  });

  //  disabling page reload on submit
  $('.form-submit.search-button').live('click', function(e) {
    if (e.button == 0) {
      e.preventDefault();
      var field = $(this).parent().children('[name="field"]').val();
      var url = $(this).parent().children('[name="url"]').val();
      var type = $(this).parent().children('[name="fieldtype"]').val();
      var from_str = field + '_placeholder';
      var to_str;
      if (type == 'int' || type == 'float' || type == 'time' || type == 'date' || type == 'datetime' || type == 'timestamp') {
        var min = $(this).parent().find('[name="min"]').val();
        var max = $(this).parent().find('[name="max"]').val();
        
        switch (type) {
          case 'date':
            to_str = '[' + min + 'T00:00:00Z TO ' + max + 'T00:00:00Z]';
          break;
          case 'time':
            to_str = '[0001-01-01T' + min + 'Z TO 0001-01-01T' + max + 'Z]';
          break;
          case 'datetime':
            to_str = '[' + min.replace(' ', 'T') + 'Z TO ' + max.replace(' ', 'T') + 'Z]';
          break;
          default:
            to_str = '[' + min + ' TO ' + max + ']';
            break;
        }
        
    url = url.replace(from_str, to_str);
        
    var equal = url.indexOf("=");
	var amper = url.indexOf("&");
    var selectedHref = url.substring(equal + 1, amper);
	filtersToSend = decodeURIComponent(selectedHref);
	hash = '?filters=' + selectedHref;
    location.hash = hash; 
      
    //window.location.href = 'http://' + location.host + url;
    
      } else if (type == 'length') {
        var option = $(this).parent().children('.form-item').children('[name="option"]').val();
        var value = parseInt($(this).parent().children('.form-item').children('[name="value"]').val());
        if (option.indexOf('[* TO #]') > -1)
          value -= 1;
        else if (option.indexOf('[# TO *]')  > -1)
          value += 1;
        
        query = option.replace('#', value);
        to_str = query;
        from_str = '%3A' + from_str;
        url = url.replace(from_str, to_str);
        
    var equal = url.indexOf("=");
	var amper = url.indexOf("&");
	var selectedHref = url.substring(equal + 1, amper);
	filtersToSend = decodeURIComponent(selectedHref);
	hash = '?filters=' + selectedHref;
    location.hash = hash; 
    
    
   // window.location.href = 'http://' + location.host + url;
      } else {
        var negative_url = $(this).parent().children('[name="negative_url"]').val();
        var option = $(this).parent().children('.form-item').children('[name="option"]').val();
        var text = $(this).parent().children('.form-item').children('[name="value"]').val();
        text = text.replace(/(\+|-|&&|\|\||!|\(|\)|\{|\}|\[|\]|\^|"|~|\*|\?|:|\\\\)/g, '\\$1');
        text = text.replace(/ /g,'+');
        text = encodeURIComponent(text);
        if (option.indexOf('-') > -1) {
          url = negative_url;
          option = option.replace('-', '');
        }
        query = option.replace('_', text);
        to_str = query;
        url = url.replace(from_str, to_str);
        
    var equal = url.indexOf("=");
	var amper = url.indexOf("&");
    var selectedHref = url.substring(equal + 1, amper);
	filtersToSend = decodeURIComponent(selectedHref);
	hash = '?filters=' + selectedHref;
    location.hash = hash; 
    
    //window.location.href = 'http://' + location.host + url;
      }
    }
  });
  
  
  
// //PD START
  
  //Reset
  
	$('.active').live('click', function(e) {
    if ($(this).text() == 'Reset') {
	  e.preventDefault();
	  
	  var myHref = $(this).attr('href');
      
	  
        var equal = myHref.indexOf("=");
		var selectedHref = myHref.substr(equal + 1);
		if (equal != -1) {
			filtersToSend = decodeURIComponent(selectedHref);
			hash = '?filters=' + selectedHref;
			location.hash = hash; 
		}	
	  
	  else {
		location.hash = '';
	   } 
    }
  });
	
	
	


// sorting by title
  var initialArray;
  var title_sort;
  var type_sort;
  var min_cur;
  var max_cur;
    
 //var sortFn = function() {
  
  
  
  var sortA = $('.tow-inner-search-widget').get();
  
  
  var div_to_put = $('#tow-search-inner-hash-form');
  
  var title_sort1 = function () {
    title_sort = 'asc';
    type_sort = '';
    $('.tow-inner-search-widget-sort a[href*=type]').text('type');
    $('.tow-inner-search-widget-sort a[href*=title]').text('title (asc)');
    var sortArray = $('.tow-inner-search-widget').get();
      sortArray.sort(function(a,b){
        var keyA = $(a).text().toLowerCase();
        var keyB = $(b).text().toLowerCase();

        if (keyA > keyB) return -1;
        if (keyA < keyB) return 1;
        return 0;
      });
      $.each(sortArray, function(i, form){
        $('#tow-search-inner-hash-form').after(form);
      });
  };
  
  var title_sort2 = function () {
    title_sort = 'desc';
    type_sort = '';
    $('.tow-inner-search-widget-sort a[href*=type]').text('type');
    $('.tow-inner-search-widget-sort a[href*=title]').text('title (desc)');
    var sortArray = $('.tow-inner-search-widget').get();
      sortArray.sort(function(a,b){
        var keyA = $(a).text().toLowerCase();
        var keyB = $(b).text().toLowerCase();

        if (keyA < keyB) return -1;
        if (keyA > keyB) return 1;
        return 0;
      });
      $.each(sortArray, function(i, form){
        $('#tow-search-inner-hash-form').after(form);
      });
  };    

  
  $('.tow-inner-search-widget-sort a[href*=title]').live('click',function(e){
    e.preventDefault();
    if (title_sort == 'asc') {
      title_sort2();
    }
    else {title_sort1();}
    });
  
  
// sorting by type
  
  var type_sort1 = function () {
    title_sort = '';
    type_sort = 'asc';
    $('.tow-inner-search-widget-sort a[href*=title]').text('title');
    
    $('.tow-inner-search-widget-sort a[href*=type]').text('type (asc)');
    var sortArray = $('.tow-inner-search-widget').get();
      sortArray.sort(function(a,b){
        var keyA = $(a).find('input[value=text], input[value=char], input[value=bool], input[value=code], input[value=enum], input[value=date], input[value=datetime], input[value=float], input[value=int], input[value=table], input[value=time], input[value=timestamp]').attr('value');
        
        var keyB = $(b).find('input[value=text], input[value=char], input[value=bool], input[value=code], input[value=enum], input[value=date], input[value=datetime], input[value=float], input[value=int], input[value=table], input[value=time], input[value=timestamp]').attr('value');
        
        if (keyA > keyB) return -1;
        if (keyA < keyB) return 1;
        return 0;
      });
      $.each(sortArray, function(i, form){
        $('#tow-search-inner-hash-form').after(form);
      });
  };
      
  
  var type_sort2 = function () {
    title_sort = '';
    type_sort = 'desc';
    $('.tow-inner-search-widget-sort a[href*=title]').text('title');
    
    $('.tow-inner-search-widget-sort a[href*=type]').text('type (desc)');
    var sortArray = $('.tow-inner-search-widget').get();
      sortArray.sort(function(a,b){
        var keyA = $(a).find('input[value=text], input[value=char], input[value=bool], input[value=code], input[value=enum], input[value=date], input[value=datetime], input[value=float], input[value=int], input[value=table], input[value=time], input[value=timestamp]').attr('value');
        var keyB = $(b).find('input[value=text], input[value=char], input[value=bool], input[value=code], input[value=enum], input[value=date], input[value=datetime], input[value=float], input[value=int], input[value=table], input[value=time], input[value=timestamp]').attr('value');

        if (keyA < keyB) return -1;
        if (keyA > keyB) return 1;
        return 0;
      });
      $.each(sortArray, function(i, form){
        $('#tow-search-inner-hash-form').after(form);
      });
  };

  
  $('.tow-inner-search-widget-sort a[href*=type]').live('click',function(e){
    e.preventDefault();
        if (type_sort == 'asc') {
      type_sort2();
    }
    else {type_sort1();}
    });
  
  
// sort reset all
  $('.sort-link-reset').live('click', function(e) {
    title_sort = '';
    type_sort = '';
    e.preventDefault();
    $('.tow-inner-search-widget-sort a[href*=title]').text('title');
    $('.tow-inner-search-widget-sort a[href*=type]').text('type');
    
	//$('.tow-inner-search-widget').remove();
	$('#tow-search-inner-hash-form').after(initialArray);
  });
 //};
 //sortFn(); 
  
 
//  AJAX search
  
  url = 'http://' + window.location.hostname + window.location.pathname + '/refresh_ajax';
  var filtersToSend;
  
  var href_substr = function () {
    $('a.apachesolr-facet, a.apachesolr-hidden-facet, .tow-inner-search-selected').live('click', function(e){
      e.preventDefault();
      var myHref = $(this).attr('href');
      
	  var equal = myHref.indexOf("=");
	  
	  if (equal != -1) {
		  var selectedHref = myHref.substr(equal + 1);
		  filtersToSend = decodeURIComponent(selectedHref);
		  hash = '?filters=' + selectedHref;
		  location.hash = hash; 
	  }
	  else {
		location.hash = '';
	  }
	  
      $('#edit-filters').value = filtersToSend;
	  
	  
		
	   
	  //inner_search_ajax(); 
    });
  };
  
  href_substr();
  
  var inner_search_ajax = function () {
    
	//disabling other widget choice while AJAX proceed
		$('#tow-search-inner-hash-form').after('<div id="modalDiv" style="position:absolute;top:0px;left:0px;display:none;cursor:progress;z-index:100;"></div>');
		
		document.getElementById("modalDiv").style.height="100%";
		document.getElementById("modalDiv").style.width="100%";
		document.getElementById("modalDiv").style.display="block";
		
		
			var jsonToSend = JSON.stringify(arrayOfZooms);
					
	$.ajax({
      url: url,
      data: {
			'filters':filtersToSend,
			'zoom':jsonToSend
			 },
      beforeSend: function() {
		//preserving user-selected collapsibility
		$('.tow-inner-search-widget fieldset').each(function() {
			var field = $(this).find('[name="field"]').val();
			var collapsed;
			if($(this).hasClass('collapsed')) {
				collapsed = true;
			}
			else collapsed = false;
			
			arrayOfCollapse[field] = {'collapsed': collapsed};
		
		});
	  
	  
	  },
	  
	  success: function(data) {
		/*function getUrlVars() {
			var vars = {};
			var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
				vars[key] = value;
			});
			return vars;
		}		  
		
		var first = getUrlVars()["filters"];
		var second = getUrlVars()["zoom"];
 
		alert(first);
		alert(second);		  */
		
		
		
		
        var html = data.block;
		$('div#block-tow-search_inner_facets div.content').html(html);
		
		//returns collapsibility
		Drupal.behaviors.collapse();
		
		//returning user-selected collapsibility
		$('.tow-inner-search-widget fieldset').each(function() {
			var field = $(this).find('[name="field"]').val();
			if (arrayOfCollapse[field]['collapsed'] == true) {
				$(this).addClass('collapsed');
			}
		});	
		
		initialArray = $('.tow-inner-search-widget').get();
        $('.apachesolr-hidden-facet', context).hide();

        show_more_fewer();
		
		
		if (title_sort == 'asc') {title_sort1();}
		if (title_sort == 'desc') {title_sort2();}
		if (type_sort == 'asc') {type_sort1();}
		if (type_sort == 'desc') {type_sort2();}
		
		
        href_substr();
		testChart();
		
	
	// Запускаем таймер в начале выполнения сценария
	//	var start = new Date();
	
	// Код, время которого необходимо измерить	 
		
		var tableMain = data.search;
		$('div.content-content').html(tableMain);
		
		//number of rows in searchtable
		var numberOfRows = $('#datatable-1 tbody tr').size();
		$('#tow-search-inner-save-search-form').children('div').children('[name="rows_amount"]').val(numberOfRows);	
		
		
        $('#datatable-1').dataTable({
		"sDom": 'C<"clear">frti',
		"oColVis": {
		  "fnLabel": function ( index, title, th ) {
		    return (index+1) +'. '+ title;
		  }
		},

		
		/*"sDom": '<"H"fr>tC<"F"ip>',
		"bJQueryUI": true,
		"bPaginate": false,
		"sScrollY": "200px",
		"bScrollCollapse": true,
		"bScrollAutoCss": false,
		"aoColumnDefs": [
			{ "bVisible": false, "aTargets": [ 2 ] }
		],
		"oColVis": {
			"buttonText": "&nbsp;",
			"bRestore": true,
			"sAlign": "left"
		},
		"fnDrawCallback": function (o) {
			// Position the ColVis button as needed 
			var nColVis = $('div.ColVis', o.nTableWrapper)[0];
			nColVis.style.width = o.oScroll.iBarWidth+"px";
			nColVis.style.top = ($('div.dataTables_scroll', o.nTableWrapper).position().top)+"px";
			nColVis.style.height = ($('div.dataTables_scrollHead table', o.nTableWrapper).height())+"px";
		}
		});*/
		
		"sScrollY": "300",
		
		//"oScroller": {
			//"rowHeight": 50
			//"displayBuffer": 9
		//},
		/*"aaData": [
			 Reduced data set 
			[ "Trident", "Internet Explorer 4.0", 4 ],
			[ "Trident", "Internet Explorer 5.0", 5 ],
			[ "Trident", "Internet Explorer 4.0", 4 ],
			
			],*/
		//"bServerSide": true,
		
		//"sAjaxSource": window.location.pathname + '/refresh_ajax?filters=' + filtersToSend,
		//"sAjaxDataProp": "data.search",
        //"sDom": "frtiS",
		//"bStateSave": true,
        //"bDeferRender": true,
		//"bRetrieve": true,
		//"bScrollInfinite": true
		"bScrollCollapse": true,
		"bPaginate": false,
		"bFilter": true,
        "bSort": true,
        "bInfo": false,
        "sScrollX": "100%"
		
       
	    });
		//oTable.fnClearTable();
		/*oTable.fnAddData([
			
			[ "Trident", "Internet Explorer 4.0", 4 ],
			[ "Trident", "Internet Explorer 5.0", 5 ],
			[ "Trident", "Internet Explorer 4.0", 4 ]
			
			]);
		*/
//for (var i = 0; i < 10000; i++) {/* Do nothing */}
// Еще один таймер в конце
//var end = new Date();
// Вычисляем разницу в ms
//var result = end.getTime() - start.getTime();
// Вывод результата
//alert(result + 'ms');
	   
				
		
      },
        
      dataType: 'json'
    });
  
  };
  

//hashchange
  
  $(window).hashchange(function(){
  
  hash = location.hash;
  var equal = hash.indexOf("=");
  var selectedHref = hash.substr(equal + 1);
  filtersToSend = decodeURIComponent(selectedHref);
	
  inner_search_ajax();
     
  });
  $(window).hashchange();

  
//AJAX text fields search
  
 
  
  $('.form-text').live('change', function(){
	
	if ($(this).val() != '') { 
    
     
      var field = $(this).parent().parent().children('[name="field"]').val();
      var url = $(this).parent().parent().children('[name="url"]').val();
      var type = $(this).parent().parent().children('[name="fieldtype"]').val();
      var from_str = field + '_placeholder';
      var to_str;
      
	  if (type == 'length') {
        var option = $(this).parent().parent().children('.form-item').children('[name="option"]').val();
        var value = parseInt($(this).parent().parent().children('.form-item').children('[name="value"]').val());
        if (option.indexOf('[* TO #]') > -1)
          value -= 1;
        else if (option.indexOf('[# TO *]')  > -1)
          value += 1;
        
        query = option.replace('#', value);
        to_str = query;
        from_str = '%3A' + from_str;
        url = url.replace(from_str, to_str);
        
    var equal = url.indexOf("=");
	var amper = url.indexOf("&");
	var selectedHref = url.substring(equal + 1, amper);
	filtersToSend = decodeURIComponent(selectedHref);
	hash = '?filters=' + selectedHref;
    location.hash = hash; 
           
      } else {
        var negative_url = $(this).parent().parent().children('[name="negative_url"]').val();
        var option = $(this).parent().parent().children('.form-item').children('[name="option"]').val();
        var text = $(this).parent().parent().children('.form-item').children('[name="value"]').val();
        text = text.replace(/(\+|-|&&|\|\||!|\(|\)|\{|\}|\[|\]|\^|"|~|\*|\?|:|\\\\)/g, '\\$1');
        text = text.replace(/ /g,'+');
        text = encodeURIComponent(text);
        if (option.indexOf('-') > -1) {
          url = negative_url;
          option = option.replace('-', '');
        }
        query = option.replace('_', text);
        to_str = query;
        url = url.replace(from_str, to_str);
        
    var equal = url.indexOf("=");
	var amper = url.indexOf("&");
    var selectedHref = url.substring(equal + 1, amper);
	filtersToSend = decodeURIComponent(selectedHref);
	hash = '?filters=' + selectedHref;
    location.hash = hash; 
       }
    }
  });
  
//AJAX highcharts "on-line" reloading
	 $('.tow-inner-search-highcharts-container .detail-container').live('mouseup', function(e) {
	 
	  var field = $(this).closest('.tow-inner-search-widget').find('[name="field"]').val();
      var url = $(this).closest('.tow-inner-search-widget').find('[name="url"]').val();
      var type = $(this).closest('.tow-inner-search-widget').find('[name="fieldtype"]').val();
      var from_str = field + '_placeholder';
      var to_str;
      if (type == 'int' || type == 'float' || type == 'time' || type == 'date' || type == 'datetime' || type == 'timestamp') {
        var min = $(this).closest('.tow-inner-search-widget').find('[name="min"]').val();
        var max = $(this).closest('.tow-inner-search-widget').find('[name="max"]').val();
		var glo_min = $(this).closest('.tow-inner-search-widget').find('[name="global_min"]').val();
		var glo_max = $(this).closest('.tow-inner-search-widget').find('[name="global_max"]').val();
        var sel_min = $(this).closest('.tow-inner-search-widget').find('[name="selection_min"]').val();
		var sel_max = $(this).closest('.tow-inner-search-widget').find('[name="selection_max"]').val();
		var check = $(this).closest('.tow-inner-search-widget').find('.form-checkbox').attr('checked');
		
		
		//var regV = /\[\*\sTO\s\*\]/g;
		var result = location.hash.search(field);
		//alert(result);
		
		
		
		
		if ((min == min_cur && max == max_cur) || (sel_min == glo_min && sel_max == glo_max && result == -1) || (sel_min == glo_min && sel_max == glo_max && !check)) {
				return false;
			}
		
		else {
		
        switch (type) {
          case 'date':
            to_str = '[' + min + 'T00:00:00Z TO ' + max + 'T00:00:00Z]';
          break;
          case 'time':
            to_str = '[0001-01-01T' + min + 'Z TO 0001-01-01T' + max + 'Z]';
          break;
          case 'datetime':
            to_str = '[' + min.replace(' ', 'T') + 'Z TO ' + max.replace(' ', 'T') + 'Z]';
          break;
          default:
            to_str = '[' + min + ' TO ' + max + ']';
            break;
        }
        
    url = url.replace(from_str, to_str);
        
    var equal = url.indexOf("=");
	var amper = url.indexOf("&");
    var selectedHref = url.substring(equal + 1, amper);
	filtersToSend = decodeURIComponent(selectedHref);
	hash = '?filters=' + selectedHref;
    location.hash = hash; 
		}
				
		
		
		min_cur = min;
		max_cur = max;
		
	}
  });
 


/*
document.getElementById("<%=твой_буттон.ClientID%>").onclick=function(){
document.getElementById("modalDiv").style.height="100%";
document.getElementById("modalDiv").style.width="100%";
document.getElementById("modalDiv").style.display="block";
}//
*/
// // PD END
  
  
 
    //  disabling page reload on submit
  $('[name="include_empty"]').live('click', function(e) {
    if (e.button == 0) {
	  //e.preventDefault();
	  var empty = $(this).closest('.tow-inner-search-widget').find('[name="include_empty_url"]').val();
	    	
    var equal = empty.indexOf("ers=");
	var amper = empty.indexOf("&");
	var selectedHref = empty.substring(equal + 4, amper);
		if (equal != -1) {
			filtersToSend = decodeURIComponent(selectedHref);
			hash = '?filters=' + selectedHref;
			location.hash = hash; 
		}	
	  
	     else {
		    location.hash = '';
	      }// window.location.href = 'http://' + location.host + url;
    }
  });
  
 var testChart = function() {
  $('div.tow-inner-search-highcharts-container').each(function(index){
    var id = $(this).attr('id');
    
    // This for the old block structure (without fieldsets).
    /*
    var datastring = $(this).parent().children('form').children('div').children('[name="data"]').val();
    var namestring = $(this).parent().children('form').children('div').children('[name="tooltips"]').val();
    var fieldtype = $(this).parent().children('form').children('div').children('[name="fieldtype"]').val();
    var fieldname = $(this).parent().children('form').children('div').children('[name="field"]').val();
    */
    
    // A new block (with fieldsets).
    var datastring = $(this).parent().children('div').children('[name="data"]').val();
    var namestring = $(this).parent().children('div').children('[name="tooltips"]').val();
    var fieldtype = $(this).parent().children('div').children('[name="fieldtype"]').val();
    var fieldname = $(this).parent().children('div').children('[name="field"]').val();
    
    var min = $(this).parent().find('[name="global_min"]').val();
    var max = $(this).parent().find('[name="global_max"]').val();
    var selection_min = $(this).parent().find('[name="selection_min"]').val();
    var selection_max = $(this).parent().find('[name="selection_max"]').val();
    var yMax = $(this).parent().find('[name="max_count"]').val();
    datastring = datastring.substring(4, datastring.length - 4);
    datastring = datastring.split(' ], [ ');//$(this).text(id);
    namestring = namestring.substring(3, namestring.length - 3);
    namestring = namestring.split('", "');//$(this).text(id);
    var data = [];
    for (var key in datastring) {
      var val = datastring[key];
      point = val.split(', ');
      data[key] = {
        x:  parseFloat(point[0]), 
        y:  parseInt(point[1]),
        name: namestring[key]
      };
      
    }
    if (fieldtype != 'time' && fieldtype != 'date' && fieldtype != 'datetime' && fieldtype != 'timestamp')
      axistype = 'linear';
    else
      axistype = 'datetime';
    
		plotChart(id, data, axistype, fieldname, fieldtype, parseFloat(min), parseFloat(max), parseFloat(yMax) * 1.1, parseFloat(selection_min), parseFloat(selection_max));
	
  });
	
  };

  
  testChart();
  
  
  function plotChart(id, data, axistype, fieldname, fieldtype, globalMin, globalMax, yMax, selectionMin, selectionMax) {
    var masterChart,
      detailChart;
    //arrayOfCharts = {'master' : masterChart, 'detail' : detailChart};
  /*updateData = function (id) {
    detailChart.series[0].setData(data.data, false);
    masterChart.series.data = newData;
    detailChart.series.data = newData;
    masterChart.redraw();
  }*/
    var visible_min = globalMin,
      visible_max = globalMax;
    
    var isMouseDown = false,
      currentIndex,
      lastMousePos,
      lastPos,
      currentHandle,
      currentObj,
      detailBorder,
      ratio;
      
    var screenSelRange = {min: null, max: null},
      plotSelRange = {min: null, max: null};
      
    var  detailLeft,
      detailRight;

    var $container = $('#' + id)
      .css('position', 'relative');

    var $detailContainer = $('<div id="' + id + '-detail" class="detail-container">')
      .css({ position: 'absolute', top: 0, height: 128, width: '100%'})
      .appendTo($container);

    var $masterContainer = $('<div id="' + id + '-master">')
      .css({ position: 'absolute', top: 128, height: 40, width: '100%'})
      .appendTo($container);
    
    var minorTickInterval, 
      tickInterval;
    if (yMax >=6) {
      minorTickInterval = Math.floor(yMax/6);
      tickInterval = minorTickInterval * 2;
    } else if (yMax >= 4) {
      minorTickInterval = 1;
      tickInterval = 2;
    } else 
      tickInterval = 1;

      // create master and in its callback, create the detail chart
    createMaster();

    $container
      .before('<div class="tow-highcharts-summary" id="' + id + '-summary"></div>'); 
    displaySummary();
    
    // create the master chart
    function createMaster() {
        masterChart = new Highcharts.Chart({
            chart: {
                renderTo: id + '-master',
                zoomType: 'x',
                borderWidth: 0,
                marginTop: 3,
                marginLeft: 25,
                marginRight: 10,
                events: {
                
                    mousedown: function(e){
                        isMouseDown = true;
                        currentObj = 'master';
                        lastMousePos = e.clientX;
                    },

                    // listen to the selection event on the master chart to update the
                    // extremes of the detail chart
                    selection: function(event) {
                      $('#' + id +'-waiting').show();
                      var extremesObject = event.xAxis[0],
                          min = extremesObject.min,
                          max = extremesObject.max;
                        
						arrayOfZooms[id] = {'min' : min, 'max' : max};
						
                        $(window).css('cursor', 'wait');
                        var url = 'http://' + location.host + '/search_inner_zoom_ajax?XDEBUG_SESSION_START=ajax';
                        var searchRid = $('#tow-search-inner-hash-form #edit-rid').val();
                        var searchHash = $('#tow-search-inner-hash-form #edit-hash').val();
                        $.ajax({
                          url: url,
                          data: {
                            'op': 'zoom',
                            'rid' : searchRid,
                            'hash' : searchHash,
                            'visible_min'  : min,
                            'visible_max' : max,
                            'field_type' : fieldtype,
                            'field' : fieldname,
                          },
                          dataType: 'json',
                          success: function(data) {
                            //detailChart.yAxis[0].setExtremes(0, 1.1 * data.yMax, false);
                            $('#' + id +'-waiting').hide();
                            detailChart.series[0].setData(data.data, false);
                            $(window).css('cursor', 'auto');
                            newVisibleRange(min, max);
							
							                            
                          },
                          
                        });    
                        newVisibleBox(min, max, false);
                        					
                        return false;
                    }
                }
            },
            title: {
                text: null
            },
            xAxis: {
                type: axistype,
                showLastLabel: false,
                showFirstLabel: false,
                gridLineWidth: 0,
                tickLength: 0,
                min: globalMin,
                max: globalMax,
                title: {
                    text: null
                },
                labels: {
                  enabled: false
                }
            },
            yAxis: {
                gridLineWidth: 0,
                labels: {
                    enabled: false
                },
                title: {
                    text: null
                },
                min: 0,
                max: yMax,
                showFirstLabel: false,
                endOnTick: false,
            },
            tooltip: {
                formatter: function() {
                    return false;
                }
            },
            legend: {
                enabled: false
            },
            credits: {
                enabled: false
            },
            plotOptions: {
                series: {
                    color: '#4169E1',
                    fillColor: {
                        linearGradient: [0, 0, 0, 70],
                        stops: [
                            [0, '#4572A7'],
                            [1, 'rgba(0,0,0,0)']
                        ]
                    },
                    lineWidth: 1,
                    marker: {
                        enabled: false
                    },
                    shadow: false,
                    states: {
                        hover: {
                            lineWidth: 1
                        }
                    },
                    enableMouseTracking: false
                }
            },

            series: [{
                type: 'area',
                data: data
            }]

        }, function(masterChart) {
            masterChart.xAxis[0].minRange = 5 * (globalMax - globalMin) / $('#' + masterChart.container.id).width();
            $('#' + masterChart.container.id).after('<div id="' + masterChart.container.id + '-visible"></div>');
            var yExtremes = masterChart.yAxis[0].getExtremes(),
              left = masterChart.xAxis[0].translate(globalMin, false) + masterChart.plotLeft,
              width = masterChart.plotWidth,
              top = masterChart.plotTop - visibleBoxBorder,
              height = masterChart.plotHeight;
              $('#' + masterChart.container.id).dblclick(function(e){
                resetZoom();
				arrayOfZooms[id] = undefined;
              });
            $('#' + masterChart.container.id + '-visible')
              .css('position', 'absolute')
              .css('border-style', 'solid')
              .css('border-width', visibleBoxBorder)
              .css('left', left)
              .css('width', width)
              .css('top', top)
              .css('height', height)
              .css('pointer-events', 'none')
              .hide()
              .append('<div id="' + masterChart.container.id + '-visible-handle' + '">');

              $('<div>')
              .css('position', 'absolute')
              .css('border-style', 'solid')
              .css('border-color', 'grey')
              .css('border-width', visibleBoxBorder)
              .css('left', masterChart.plotLeft - 2)
              .css('width', masterChart.plotWidth + 3)
              .css('top', masterChart.plotTop - 2)
              .css('height', masterChart.plotHeight + 3)
              .css('pointer-events', 'none')
              .appendTo('#' + masterChart.container.id);
              
            $('#' + masterChart.container.id + '-visible-handle')
              .css('position', 'absolute')
              .css('border-style', 'solid')
              .css('border-width', visibleHandleBorder)
              .css('background-color', 'white')
              .css('right', width - visibleHandleOffset)
              .css('width', visibleHandleWidth)
              .css('top', height - visibleHandleHeight/2 - visibleHandleBorder)
              .css('height', visibleHandleHeight)
              .css('pointer-events', 'visible')
              .css('cursor', 'e-resize')
              .mousedown(function(e){
                isMouseDown = true;
                currentObj = 'box';
                lastMousePos = e.clientX;
                var zero = masterChart.xAxis[0].translate(0, true),
                  one =  masterChart.xAxis[0].translate(1, true),
                  extremes = detailChart.xAxis[0].getExtremes();
                ratio = one - zero;
                return false;
              });
              
            createDetail(masterChart);
            
        });
    };

    // create the detail chart
    function createDetail(masterChart) {

        // create a detail chart referenced by a global variable
        detailChart = new Highcharts.Chart({
            chart: {
                renderTo: id + '-detail',
                style: {
                    position: 'absolute'
                },
                 spacingTop: 5,
                spacingRight: 10,
                spacingBottom: 3,
                spacingLeft: 0,
                marginLeft: 25,
                marginRight: 10,
                type: 'spline', 
                plotBorderWidth: 1,
            },
            credits: {
                enabled: false
            },
            title: {
                text: null
            },
            subtitle: {
                text: null
            },
            xAxis: {
                type: axistype,
                min: globalMin,
                max: globalMax,
                tickPixelInterval: 50,
                minPadding: 0.02,
                maxPadding: 0.02,
            },
            yAxis: {
                title: {
                    text: null
                },
                min: 0,
                max: yMax,
                 labels: {
                  x: -1,
                  y: 4
                },
                endOnTick: false,
                minorTickInterval: minorTickInterval,
                tickInterval: tickInterval,            
            },
            tooltip: {
              formatter: function() {
                tooltip = this.point.name;
                tooltip = tooltip.replace(/\\\\n/g,'<br/>');
                tooltip = tooltip.replace(/\\n/g,'<br/>');
                return tooltip;
              },
//              crosshairs: true,
              style: {
                fontSize: '7pt',
                zIndex: 11,
              }
            },
            legend: {
                enabled: false
            },
            plotOptions: {
                  series: {
                    color: '#4169E1',
                    marker: {
                        enabled: false,
                        states: {
                            hover: {
                                enabled: true
                            }
                        }
                    },
                    states: {
                        hover: {
                          lineWidth: 2
                        }
                    }
                },  
/*                 column: {
                  borderWidth: 0,
                  pointWidth: 1,
                  shadow: false,
                }, */
            }, 
            series: [{
              data: data
            }]
        }, function(detailChart){
          
          $('<div>')
            .attr('id', id +'-waiting')
            .css('position', 'absolute')
            .css('left', detailChart.plotLeft)
            .css('top', detailChart.plotTop)
            .css('width', detailChart.plotWidth)
            .css('height', detailChart.plotHeight)
            .css("background-image", "url('http://" + location.host + "/sites/all/modules/_custom/tow/waiting.gif')")
            .css('background-repeat', 'no-repeat')
            .css('background-position', 'center center')
            .css('background-color', 'rgba(255,255,255, 0.8)')
            .css('z-index', 20)
            .appendTo('#' + detailChart.container.id)
            .hide();
          $('<div>')
            .attr('id', id +'-noresult')
            .css('position', 'absolute')
            .css('left', detailChart.plotLeft)
            .css('top', detailChart.plotTop)
            .css('width', detailChart.plotWidth)
            .css('margin-top', detailChart.plotHeight/2-10)
            .css('background-position', 'center center')
            .css('z-index', 9)
            .css('text-align', 'center')
            .css('vertical-align', 'middle')
            .css('font-size', '10')
            .html('No results found')
            .appendTo('#' + detailChart.container.id);
            if (yMax)
              $('#' + id +'-noresult').hide();

          $('#' + detailChart.container.id).mousedown(function(e){
            isMouseDown = true;
            currentObj = 'detail';
            lastMousePos = e.clientX;
            var zero = detailChart.xAxis[0].translate(0, true),
              one =  detailChart.xAxis[0].translate(1, true),
              extremes = detailChart.xAxis[0].getExtremes();
            ratio = one - zero;
            visible_min = extremes.min;
            visible_max = extremes.max;
          })
          .dblclick(function(e){
            resetZoom();
			arrayOfZooms[id] = undefined;
          });

          towHighchartsNavigator(detailChart, masterChart);
        });
    };
    
	
	
	//preserving zoom after AJAX
	 
    arrayOfCharts[id] = {'master': masterChart, 'detail' : detailChart};
	 
    if (arrayOfZooms[id] != undefined) {
		
		//arrayOfCharts[id]['detail'].series[0].setData(data.data, false);
		createDetail(arrayOfCharts[id]['master']);
		
		newVisibleRange(arrayOfZooms[id]['min'], arrayOfZooms[id]['max']);
		newVisibleBox(arrayOfZooms[id]['min'], arrayOfZooms[id]['max'], false);
		
		towHighchartsNavigator(arrayOfCharts[id]['detail'], arrayOfCharts[id]['master']);
	};
	
	
	
	
    function towHighchartsNavigator(detailChart, masterChart){
      add_mask(masterChart);
      add_mask(detailChart);
      
      detailLeft = detailChart.xAxis[0].translate(visible_min, false) + detailChart.plotLeft;
      detailRight = detailChart.xAxis[0].translate(visible_max, false) + detailChart.plotLeft;
      
      screenSelRange = {
        min: detailChart.xAxis[0].translate(selectionMin, false) + detailChart.plotLeft, 
        max: detailChart.xAxis[0].translate(selectionMax, false) + detailChart.plotLeft
      };

      plotSelRange = {
        min: selectionMin, 
        max: selectionMax
      };

      draw_handle(0, screenSelRange.min);
      draw_handle(1, screenSelRange.max);
      
      function add_mask(chart){
        changeMask(chart, 0, selectionMin);
        changeMask(chart, 1, selectionMax);
      }
      
      function draw_handle(index, x){
        var top = $('#' + detailChart.container.id).height(),
          handle_id = detailChart.container.id + '-handle-' + index;
        top = (top - handleHeight)/2;
        $('#' + detailChart.container.id).after('<div id="' + handle_id + '">');
        $('#' + handle_id)
          .css('position', 'absolute')  
          .css('top', top)  
          .css('left', x - handleWidth/2)  
          .css('height', handleHeight)  
          .css('width', handleWidth)
          .css('z-index', 10)
          .css('background-color', 'white')
          .css('border-style', 'solid')
          .css('border-width', '1px')
          .css('cursor', 'e-resize')
          .mousedown(function(e){
            isMouseDown = true;
            currentIndex = index;
            lastMousePos = e.clientX;
            currentObj = 'handle';
            if (index) {
              detailBorder = screenSelRange.min;
              lastPos = screenSelRange.max;
            } else {
              detailBorder = screenSelRange.max;
              lastPos = screenSelRange.min;
            }
            return false;
          });
      }
    };
    

    $(document).mousemove(function(e){
      if (isMouseDown) {
        switch (currentObj){
          case 'handle':
            var span = e.clientX - lastMousePos;
            var newPos = lastPos + span;
            correctedPos = newHandlePos(newPos);
            moveHandle(correctedPos);
            break;
          case 'box':
            var span = e.clientX - lastMousePos,
              min = visible_min + ratio * span,
              max = visible_max + ratio * span;
            
            lastMousePos = e.clientX;
            newVisibleBox(min, max);
			
			arrayOfZooms[id] = {'min' : min, 'max' : max};
			
            return false;
          case 'detail':
            var span = e.clientX - lastMousePos,
              min = visible_min - ratio * span,
              max = visible_max - ratio * span;
            
            lastMousePos = e.clientX;
            newVisibleBox(min, max);
			
			arrayOfZooms[id] = {'min' : min, 'max' : max};
			
            break;
          case 'master':
            break;
          default:
        }
      }
      return false;
    });
    
    $(document).mouseup(function(e){
      isMouseDown = false;
    });

    function resetZoom(){
      detailChart.series[0].setData(data, false);
      newVisibleBox(globalMin, globalMax);
    } 
    function newVisibleBox(min, max, redrawDetail){
      var rangewidth = max - min,
        globalRandewidth = globalMax - globalMin;
      if (min < globalMin){
        min = globalMin;
        max = globalMin + rangewidth;
      }
      if (max > globalMax){
        max = globalMax;
        min = globalMax - rangewidth;
      }
      // move visible box on master
      var left = masterChart.xAxis[0].translate(min, false) + masterChart.plotLeft,
        width = masterChart.xAxis[0].translate(max, false) + masterChart.plotLeft - left;
      left = left - 2*visibleHandleBorder;
      $('#' + masterChart.container.id + '-visible')
        .show()
        .css('left', left)
        .css('width', width);
      $('#' + masterChart.container.id + '-visible-handle')
        .css('right', visibleHandleOffset);      
      
      if (Math.abs(min - globalMin) <= globalRandewidth/200 && Math.abs(max - globalMax) <= globalRandewidth/200)
        $('#' + masterChart.container.id + '-visible').hide();
                  
      if (redrawDetail != false) 
        newVisibleRange(min, max);
    }
    
    function newVisibleRange(min, max){
      // setting extremes on master
      detailChart.xAxis[0].setExtremes(min, max, true, false);
      
      // setting constants
      visible_min = min;
      visible_max = max;

      detailLeft = detailChart.xAxis[0].translate(min, false) + detailChart.plotLeft;
      detailRight = detailChart.xAxis[0].translate(max, false) + detailChart.plotLeft;
      
      //redraw handles
      currentIndex = 0;
      moveHandle(detailChart.xAxis[0].translate(plotSelRange.min, false) + detailChart.plotLeft, true);
      currentIndex = 1;
      moveHandle(detailChart.xAxis[0].translate(plotSelRange.max, false) + detailChart.plotLeft, true);
    }
    
	
	
    function newHandlePos(newPos){
      if (newPos > detailRight)
          return detailRight;
      if (newPos < detailLeft)
          return detailLeft;
      if (currentIndex){
        if (newPos < detailBorder) {
          moveHandle(detailBorder);
          currentIndex = 0;
          return newHandlePos(newPos);
        }
      } else {
        if (newPos > detailBorder) {
          moveHandle(detailBorder);
          currentIndex = 1;
          return newHandlePos(newPos);
        }
      }
      return newPos;
    };
    
    // moves handle defined by currentIndex to screen position screenX, 
    // and also moves masks on both charts
    function moveHandle(screenX, same){
      var handle_id = detailChart.container.id + '-handle-' + currentIndex;
      $('#' + handle_id).css('left', screenX - handleWidth/2);
      plotX = detailChart.xAxis[0].translate(screenX - detailChart.plotLeft, true);
      changeMask(detailChart, currentIndex, plotX);
      changeMask(masterChart, currentIndex, plotX);
      if (currentIndex) {
        screenSelRange.max = screenX;
        if (same != true)
          plotSelRange.max = plotX;
      } else {
        screenSelRange.min = screenX;
        if (same != true)
          plotSelRange.min = plotX;
      }
      displaySummary();
      if (screenX < detailLeft - 1  || screenX > detailRight + 1){
        $('#' + handle_id).hide();}
      else
        $('#' + handle_id).show();
    };
    
    function displaySummary(){
      var new_min = limit2display(fieldtype, plotSelRange.min);
      var new_max = limit2display(fieldtype, plotSelRange.max);
      var summary = '(from ' + new_min + ' to ' + new_max + ')';

      $('#' + id).siblings('div#' + id + '-summary"').html(summary);

      $('#' + id).parent().find('[name="selection_min"]').val(plotSelRange.min);
      $('#' + id).parent().find('[name="selection_max"]').val(plotSelRange.max);

      $('#' + id).parent().find('[name="min"]').val(new_min);
      $('#' + id).parent().find('[name="max"]').val(new_max);

    };
    
    function changeMask(chart,index,x){
      var maskId = 'mask-' + index,
      from,
      to;
      if (index){
        from = x;
        to = globalMax;
      } else {
        from = globalMin;
        to = x;
      }
      chart.xAxis[0].removePlotBand(maskId);
      chart.xAxis[0].addPlotBand({
      id: maskId,
      from: from,
      to: to,
      color: 'rgba(127, 127, 127, 0.5)',
      zIndex: 10,
      });
    }

  }
  
 /* $(window).unload(function(e){
    var url = 'http://' + location.host + '/search_inner_zoom_ajax?XDEBUG_SESSION_START=ajax';
    var searchRid = $('#tow-seach-inner-hash-form #edit-rid').val();
    var searchHash = $('#tow-seach-inner-hash-form #edit-hash').val();
    $.ajax({
      url: url,
      async: false,
      data: {
        'rid' : searchRid,
        'hash' : searchHash,
        'op'  : 'cancel',
      },
    });   
  });*/
/*  
  $('a.sort-link').click(function(e) {
    
    var url = 'http://' + location.host + $(this).attr('href');
    
    var sortWidgets = function (data) {
      var html = data.content;
      console.log(data);
      $('div#block-tow-search_inner_facets div.content').html(html);
    }
    
    var errorHandler = function(jqXHR, exception) {
      if (jqXHR.status === 0) {
        alert('Not connect.\n Verify Network.');
      } 
      else if (jqXHR.status == 404) {
        alert('Requested page not found. [404]');
      } 
      else if (jqXHR.status == 500) {
        alert('Internal Server Error [500].');
      } 
      else if (exception === 'parsererror') {
        alert('Requested JSON parse failed.');
      } 
      else if (exception === 'timeout') {
        alert('Time out error.');
      } 
      else if (exception === 'abort') {
        alert('Ajax request aborted.');
      } 
      else {
        alert('Uncaught Error.\n' + jqXHR.responseText);
      }
    }
    
    $.ajax({
      url: url,
      //async: false,
      type: 'POST',
      complete: sortWidgets,
      error: errorHandler,
      dataType: 'json'
    });

    return false;
  });
*/  
$('div.tow-inner-search-highcharts-bar-container').each(function(index){
    var id = $(this).attr('id');
    var datastring = $(this).parent().children('form').children('div').children('[name="data"]').val();
    var namestring = $(this).parent().children('form').children('div').children('[name="tooltips"]').val();
    var fieldtype = $(this).parent().children('form').children('div').children('[name="fieldtype"]').val();
    var fieldname = $(this).parent().children('form').children('div').children('[name="field"]').val();
    var selection_min = $(this).parent().find('[name="selection_min"]').val();
    var selection_max = $(this).parent().find('[name="selection_max"]').val();
    var global_min = $(this).parent().find('[name="global_min"]').val();
    var global_max = $(this).parent().find('[name="global_max"]').val();
    
    var active = true;
    
    if (global_min == selection_min && global_max == selection_max)
      active = false;
    
    datastring = datastring.substring(2, datastring.length - 2);
    datastring = datastring.split(', ');//$(this).text(id);
    namestring = namestring.substring(3, namestring.length - 3);
    var names = namestring.split('", "');//$(this).text(id);
    var data = [],
      selected = [];
    if (fieldtype != 'time' && fieldtype != 'date' && fieldtype != 'datetime' && fieldtype != 'timestamp') {
      axistype = 'linear';
      selection_min = parseFloat(selection_min);
      selection_max = parseFloat(selection_max);
    } else {
      axistype = 'datetime';
    }
    for (var key in datastring) {
      var x;
       if (axistype == 'linear') 
        x = parseFloat(names[key]);
      else 
        x = names[key];
        
      if (active)
        selected[key] = (selection_min <= x && selection_max >= x);
      else 
        selected[key] = false;
        
      data[key] = parseInt(datastring[key]);
    }
    chart = plotBarchart(id, names, data, selected, axistype, fieldname, fieldtype, parseFloat(selection_min), parseFloat(selection_max));
  });


  function plotBarchart(id, categories, data, selected, axistype, fieldname, fieldtype, selectionMin, selectionMax) {
    var $container = $('#' + id);
    $container
      .before('<div class="tow-highcharts-summary" id="' + id + '-summary"></div>'); 
    displaySummary();
    
      
    return new Highcharts.Chart({
      chart: {
        renderTo: id,
        type: 'column',
        spacingLeft: 0,
        spacingRight: 0,
      },
      title: {
        text: ''
      },
      subtitle: {
        text: ''
      },
      plotOptions: {
        column: {
          pointWidth: 15
        },
        series: {
          allowPointSelect: true,
          point: {
            events: {
              select: function(e){
                var current_key = this.x;
                
              }
            }
          },
        },
      },
      xAxis: {
        title: {
          text: null
        },
        categories: categories,
        labels: {
//          align: 'left',
//          rotation: 45,
        },
      },
      yAxis: {
        min: 0,
        title: {
          text: null,
        },
        labels: {
          x: 0,
          y: -2
        }
      },
      tooltip: {
        formatter: function() {
          var x = this.x,
            y = this.y;
            results = (y == 1) ? Drupal.t('result') : Drupal.t('results');
            return x + ':<br/>' + y + ' ' + results;
        },
        style: {
          fontSize: '7pt'
        }
      },
      legend: {
        enabled: false
      },
      credits: {
        enabled: false
      },
      series: [{
        name: '',
        data: data
      }]
    }, function(barChart){
      var points = barChart.series[0].data;
      for (var key in points) {
      var point = points[key];
        if (selected[key])
          point.select(true, true);
    }
    });
    
    function displaySummary(){
      var min = $('#' + id).parent().find('[name="selection_min"]').val(),
        max = $('#' + id).parent().find('[name="selection_max"]').val(),
        summary = '(from ' + min + ' to ' + max + ')';
      $('#' + id).siblings('div#' + id + '-summary"').html(summary);
    }
  }
}



function limit2display(fieldtype, value) {
  switch (fieldtype){
    case 'int':
      return parseInt(value);
    case 'float':
      return (parseFloat(value)).toFixed(3);
    case 'date':
      return date(parseInt(value));
    case 'timestamp':
    case 'datetime':
      return datetime(parseInt(value));
    case 'time':
      return time(parseInt(value));
  }
}
function datetime(timestamp){
  var date = new Date(timestamp);
  var hours = ('0' + date.getUTCHours()).slice(-2);
  var minutes = ('0' + date.getUTCMinutes()).slice(-2);
  var seconds = ('0' + date.getUTCSeconds()).slice(-2);
  var day = ('0' + date.getUTCDate()).slice(-2);
  var month = ('0' + (date.getUTCMonth() + 1)).slice(-2);
  var year = date.getUTCFullYear();
  return year + '-' + month + '-' + day + ' ' + hours + ':' + minutes + ':' + seconds;
}

function date(timestamp){
  var date = new Date(timestamp);
  var day = ('0' + date.getUTCDate()).slice(-2);
  var month = ('0' + (date.getUTCMonth() + 1)).slice(-2);
  var year = date.getUTCFullYear();
  return year + '-' + month + '-' + day;
}

function time(timestamp){
  var date = new Date(timestamp);
  var hours = ('0' + date.getUTCHours()).slice(-2);
  var minutes = ('0' + date.getUTCMinutes()).slice(-2);
  var seconds = ('0' + date.getUTCSeconds()).slice(-2);
  return hours + ':' + minutes + ':' + seconds;
}

function dpm(message){
  //$('#breadcrumbs-inner').before(message + ' | ');
}