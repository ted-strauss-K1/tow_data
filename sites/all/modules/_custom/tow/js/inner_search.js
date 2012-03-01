var chart;

Drupal.behaviors.outer_search = function(context) {

	// hide hidden facets
	$('.apachesolr-hidden-facet', context).hide();
	
	// add Show more/fewer link
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
	//simple search
	$('#edit-keywords').change(function() {
		window.location.href = 'http://' + location.host + '/search_dataset/' + $(this).parent().parent().children('#edit-nid').val() + '/' + $(this).val() + location.search;
	});

	//	disabling page reload on submit
	$('.form-submit').live('click', function(e) {
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
				window.location.href = 'http://' + location.host + url;
			} else {
				var negative_url = $(this).parent().children('[name="negative_url"]').val();
				var option = $(this).parent().children('.form-item').children('[name="option"]').val();
				var text = $(this).parent().children('.form-item').children('[name="value"]').val();
				text = text.replace('-','\\-');
				text = text.replace(' ','-');
				if (option.search('-')) {
					url = negative_url;
					option = option.replace('-', '');
				}
				query = option.replace('_', text);
				to_str = query;
				url = url.replace(from_str, to_str);
				window.location.href = 'http://' + location.host + '/' + url;
			}
		}
	});
	
	
		//	disabling page reload on submit
	$('[name="include_empty"]').live('click', function(e) {
		if (e.button == 0) {
			var url = $(this).parent().parent().parent().children('[name="include_empty_url"]').val();
			if (url) 
				window.location.href = 'http://' + location.host + '/' + url;
		}
	});
	
	$('div.tow-inner-search-highcharts-container').each(function(index){
		var id = $(this).attr('id');
		var datastring = $(this).parent().children('form').children('div').children('[name="data"]').val();
		var namestring = $(this).parent().children('form').children('div').children('[name="tooltips"]').val();
		var fieldtype = $(this).parent().children('form').children('div').children('[name="fieldtype"]').val();
		var min = $(this).parent().find('[name="global_min"]').val();
		var max = $(this).parent().find('[name="global_max"]').val();
		datastring = datastring.substring(4, datastring.length - 4);
		datastring = datastring.split(' ], [ ');//$(this).text(id);
		namestring = namestring.substring(3, namestring.length - 3);
		namestring = namestring.split('", "');//$(this).text(id);
		var data = [];
		for (var key in datastring) {
			var val = datastring[key];
			point = val.split(', ');
			data[key] = {
				x:	parseFloat(point[0]), 
				y:	parseInt(point[1]),
				name: namestring[key]
			};
			
		}
		if (fieldtype != 'time' && fieldtype != 'date' && fieldtype != 'datetime' && fieldtype != 'timestamp')
			axistype = 'linear';
		else
			axistype = 'datetime';
		chart = plot_chart(id, data, axistype, fieldtype, parseFloat(min), parseFloat(max));
	});
	
	////////////////////
	
	var isMouseDown = false;
	var	currentHandle = null,
		currentNavigator = null,
		currentSide = null,
		lastHandleLeft = null,
		lastNavLeft = null,
		lastMousePos = null,
		lastNavWidth = null,
		currentContainer = null;
			
	function towHighchartsNavigator(chart, min, max) {

		var x_min = chart.xAxis[0].translate(min, false) + chart.plotLeft,
				x_max = chart.xAxis[0].translate(max, false) + chart.plotLeft;

		var container_id = chart.container.id;

		var chart_height = chart.plotSizeY,
				chart_width = chart.plotSizeX,
				chart_top = chart.plotTop,
				chart_left = chart.plotLeft,
				chart_right = chart.plotLeft + chart_width;

		var nav_width = [x_min - chart_left, chart_right - x_max];
		if (nav_width[0] < 0) nav_width[0] = 0;
		if (nav_width[1] < 0) nav_width[1] = 0;
			
		var nav_left = [chart_left, chart_right - nav_width[1]];
		
		var handle_width = 4,
				handle_height = 20,
				handle_top = chart_top + chart_height/2 - handle_height/2,
				handle_left = [nav_left[0] + nav_width[0] - handle_width/2, nav_left[1] - handle_width/2];

		$('#' + container_id).parent()
				.before('<div class="tow-highcharts-summary" id="' + container_id + '-summary"></div>')
		display_summary(container_id, min, max);
		drawNavigator(0);
		drawNavigator(1);

		function drawNavigator (index) {
			var nav_id = container_id + '-navigator-' + index,
					handle_id = container_id + '-handle-' + index;
			
			$('#' + container_id)
				.after('<div class="tow-highcharts-navigator" id="' + nav_id + '"></div>')
				.after('<div class="tow-highcharts-handle" id="' + handle_id + '"></div>');
			
			$('#' + nav_id)
				.css('height', chart_height + 'px')
				.css('width', nav_width[index] + 'px')
				.css('left', nav_left[index] + 'px')
				.css('top', chart_top + 'px');
			
			$('#' + handle_id)
				.css('height', handle_height + 'px')
				.css('width', handle_width + 'px')
				.css('left', handle_left[index] + 'px')
				.css('top', handle_top + 'px')
				.mousedown(function(e) {
					isMouseDown = true;

	//				currentHandle = $(this).attr('id');
	//				currentNavigator = $(this).next('.tow-highcharts-navigator').attr('id');
					currentContainer = container_id;
					currentSide = index;

					lastMousePos = e.clientX;
					lastHandleLeft = $(this).position().left;
					lastNavLeft = $(this).next('.tow-highcharts-navigator').position().left;
					lastNavWidth = $(this).next('.tow-highcharts-navigator').width();
					
					return false;
				});
		}
		
		$('#' + container_id).parent().mousemove(function(e){
			if (isMouseDown) {
				var span = e.clientX - lastMousePos,
					handleLeft = lastHandleLeft + span,
					navLeft,
					navWidth;
				if (currentSide) {//right navigator active
					navLeft = lastNavLeft + span;
					navWidth = lastNavWidth - span;
					if (navWidth < 0) {
						navLeft = chart_right;
						navWidth = 0;
						handleLeft = chart_right - handle_width/2;
					}
					var navLeftWidth = $('#' + currentContainer + '-navigator-0').width(),
						navBorder = chart_left + navLeftWidth;
					if (navBorder >= navLeft){
						$('#' + currentContainer + '-navigator-1').css('left', navBorder + 'px');
						$('#' + currentContainer + '-navigator-1').css('width', (chart_width - navLeftWidth) + 'px');
						$('#' + currentContainer + '-handle-1').css('left', (navBorder - lastNavLeft + lastHandleLeft) + 'px');
						currentSide = 0;

						navLeft = chart_left;
						navWidth = lastNavLeft + span - chart_left;
						if (navWidth < 0) {
							navWidth = 0;
							handleLeft = chart_left - handle_width/2;
						}

						lastMousePos = e.clientX;
						lastHandleLeft = handleLeft;
						lastNavWidth = navWidth;
						lastNavLeft = navLeft;
					} 
				} else {
					navLeft = lastNavLeft;
					navWidth = lastNavWidth + span;
					if (navWidth < 0) {
						navWidth = 0;
						handleLeft = chart_left - handle_width/2;;
					}
					var navRightWidth = $('#' + currentContainer + '-navigator-1').width(),
						navBorder = chart_right - navRightWidth;
					if (navBorder <= navLeft + navWidth){
						$('#' + currentContainer + '-navigator-0').css('width', (navBorder - chart_left) + 'px');
						$('#' + currentContainer + '-handle-0').css('left', (navBorder - lastNavLeft - lastNavWidth + lastHandleLeft) + 'px');
						currentSide = 1;

						navLeft = chart_left + lastNavWidth + span;
						navWidth = chart_right - navLeft;
						if (navWidth < 0) {
							navLeft = chart_right;
							navWidth = 0;
							handleLeft = chart_right - handle_width/2;
						}
						
						lastMousePos = e.clientX;
						lastHandleLeft = handleLeft;
						lastNavWidth = navWidth;
						lastNavLeft = navLeft;
					}
				}

				$('#' + currentContainer + '-navigator-' + currentSide).css('left', navLeft + 'px');
				$('#' + currentContainer + '-navigator-' + currentSide).css('width', navWidth + 'px');
				$('#' + currentContainer + '-handle-' + currentSide).css('left', handleLeft + 'px');
			}
			return false;
		});
		$(document).mouseup(function(e){
			isMouseDown = false;
			if (currentContainer && chart.container.id == currentContainer) {
				var x_min = $('#' + currentContainer + '-navigator-0').width(),
					x_max = chart_width - $('#' + currentContainer + '-navigator-1').width();
										
				
				var min = chart.xAxis[0].translate(x_min, true);
				var	max = chart.xAxis[0].translate(x_max, true);
			
//				alert(currentContainer + ': [' + min + ' TO ' + max + ']');
				display_summary(currentContainer, min, max);

				currentHandle = null;
				currentNavigator = null;
				currentSide = null;
				lastHandleLeft = null;
				lastNavLeft = null;
				lastMousePos = null;
				lastNavWidth = null;
			}
		});
	}	
	
	
	////////////////////
	
	function plot_chart(id, data, axistype, fieldtype, global_min, global_max) {
		return new Highcharts.Chart({
			chart: {
				renderTo: id,
				type: 'line',
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
				series: {
					marker: {
						enabled: false,
						states: {
							hover: {
								enabled: true
							}
						}
					},
					lineWidth: 1
				}
			},
			xAxis: {
				title: {
					text: null
				},
				min: global_min,
				max: global_max,
//				endOnTick: false,
				tickPixelInterval: 50,
//				minPadding: 0.05,
//        maxPadding: 0.05,
				type: axistype,
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
					tooltip = this.point.name;
					tooltip = tooltip.replace(/\\\\n/g,'<br/>');
					return tooltip;
				},
				crosshairs: true,
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
		},
		function(chart){
			var min = $('#' + chart.container.id).parent().parent().find('[name="selection_min"]').val();
			var max = $('#' + chart.container.id).parent().parent().find('[name="selection_max"]').val();
			var nav = new towHighchartsNavigator(chart, min, max);
		});
	}
}

function display_summary(container, min, max){//alert(container);
	var new_min = limit2display(container, min);
	var new_max = limit2display(container, max);
	var summary = '(from ' + new_min + ' to ' + new_max + ')';

	$('#' + container).parent()
				.siblings('div#' + container + '-summary"').html(summary);

	$('#' + container).parent().parent().find('[name="selection_min"]').val(min);
	$('#' + container).parent().parent().find('[name="selection_max"]').val(max);

	$('#' + container).parent().parent().find('[name="min"]').val(new_min);
	$('#' + container).parent().parent().find('[name="max"]').val(new_max);

}

function limit2display(container, value) {
	var fieldtype = $('#' + container).parent().parent().find('[name="fieldtype"]').val();
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