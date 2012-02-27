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
			var from_str = field + ':' + field + '_placeholder';
			var to_str;
			if (type == 'int' || type == 'float' || type == 'time' || type == 'date' || type == 'datetime' || type == 'timestamp') {
				var min = $(this).parent().children('.form-item').children('[name="min"]').val();
				var max = $(this).parent().children('.form-item').children('[name="max"]').val();
				
				switch (type) {
					case 'time':
						to_str = field + ':[0001-01-01T' + min + 'Z TO 0001-01-01T' + max + 'Z]';
						break;
					case 'int':
					default:
						to_str = field + ':[' + min + ' TO ' + max + ']';
						break;
				}
				url = url.replace(from_str, to_str);
				window.location.href = 'http://' + location.host + '/' + url;
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
				to_str = field + ':' + query;
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
		chart = plot_chart(id, data, axistype, fieldtype);
	});
	
	function plot_chart(id, data, axistype, fieldtype) {
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
			xAxis: {
				title: {
					text: null
				},
				labels: {
					// formatter: function() {
						// if (fieldtype == 'time') {
							// var label = Highcharts.dateFormat('%H:%M', this.value);
							// if ((this.value > 0) && (label == '00:00')) {
								// label = '24:00';
							// }
							// return label;
						// } else if (fieldtype == 'time'){
						// } else {
							// return this.value;
						// }
					// }
				},
				endOnTick: false,
				tickPixelInterval: 50,
				minPadding: 0.05,
        maxPadding: 0.05,
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
					return this.point.name;
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
		});
	}
}