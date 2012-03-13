Drupal.behaviors.outer_search = function(context) {
	var handleHeight = 15,
		handleWidth = 6,
		handleBorder = 1,
		visibleHandleHeight = 4,
		visibleHandleWidth = 5,
		visibleHandleBorder = 1,
		visibleHandleOffset = 2,
		visibleBoxBorder = 1;

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
				window.location.href = 'http://' + location.host + url;
			}
		}
	});
	
	
		//	disabling page reload on submit
	$('[name="include_empty"]').live('click', function(e) {
		if (e.button == 0) {
			var url = $(this).parent().parent().parent().children('[name="include_empty_url"]').val();
			if (url) 
				window.location.href = 'http://' + location.host + url;
		}
	});
	
	$('div.tow-inner-search-highcharts-container').each(function(index){
		var id = $(this).attr('id');
		var datastring = $(this).parent().children('form').children('div').children('[name="data"]').val();
		var namestring = $(this).parent().children('form').children('div').children('[name="tooltips"]').val();
		var fieldtype = $(this).parent().children('form').children('div').children('[name="fieldtype"]').val();
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
				x:	parseFloat(point[0]), 
				y:	parseInt(point[1]),
				name: namestring[key]
			};
			
		}
		if (fieldtype != 'time' && fieldtype != 'date' && fieldtype != 'datetime' && fieldtype != 'timestamp')
			axistype = 'linear';
		else
			axistype = 'datetime';
		plotChart(id, data, axistype, fieldtype, parseFloat(min), parseFloat(max), parseFloat(yMax) * 1.1, parseFloat(selection_min), parseFloat(selection_max));
	});
	
	
	function plotChart(id, data, axistype, fieldtype, globalMin, globalMax, yMax, selectionMin, selectionMax) {
		var masterChart,
			detailChart;
		
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
			
		var	detailLeft,
			detailRight;

		var $container = $('#' + id)
			.css('position', 'relative');

		var $detailContainer = $('<div id="' + id + '-detail">')
			.css({ position: 'absolute', top: 0, height: 160, width: '100%'})
			.appendTo($container);

		var $masterContainer = $('<div id="' + id + '-master">')
			.css({ position: 'absolute', top: 160, height: 40, width: '100%'})
			.appendTo($container);
		
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
                        var extremesObject = event.xAxis[0],
                            min = extremesObject.min,
                            max = extremesObject.max;
												
												newVisibleBox(min, max);
												
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
							})
							.dblclick(function(e){
								newVisibleBox(globalMin, globalMax);
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
								marginRight: 10
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
										states: {
												hover: {
														lineWidth: 2
												}
										}
                }
            },
            series: [{
//                pointStart: detailStart,
                data: data
            }]
        }, function(detailChart){
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
					});
					towHighchartsNavigator(detailChart, masterChart);
				});
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
					.css('z-index', 0)
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
						return false;
					case 'detail':
						var span = e.clientX - lastMousePos,
							min = visible_min - ratio * span,
							max = visible_max - ratio * span;
						
						lastMousePos = e.clientX;
						newVisibleBox(min, max);
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
		
		function newVisibleBox(min, max){
			var rangewidth = max - min;
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
			left = left - visibleHandleBorder;
			$('#' + masterChart.container.id + '-visible')
				.show()
				.css('left', left)
				.css('width', width);
			$('#' + masterChart.container.id + '-visible-handle')
				.css('right', visibleHandleOffset);			
			if (min == globalMin && max == globalMax)
				$('#' + masterChart.container.id + '-visible').hide();
									
			// setting extremes on master
			detailChart.xAxis[0].setExtremes(min, max, true, false);
			
			// setting constants
			visible_min = min;
			visible_max = max;

			detailLeft = detailChart.xAxis[0].translate(min, false) + detailChart.plotLeft;
			detailRight = detailChart.xAxis[0].translate(max, false) + detailChart.plotLeft;
			
			//redraw handles
			currentIndex = 0;
			moveHandle(detailChart.xAxis[0].translate(plotSelRange.min, false) + detailChart.plotLeft);
			currentIndex = 1;
			moveHandle(detailChart.xAxis[0].translate(plotSelRange.max, false) + detailChart.plotLeft);
			
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
		function moveHandle(screenX){
			var handle_id = detailChart.container.id + '-handle-' + currentIndex;
			$('#' + handle_id).css('left', screenX - handleWidth/2);
			plotX = detailChart.xAxis[0].translate(screenX - detailChart.plotLeft, true);
			changeMask(detailChart, currentIndex, plotX);
			changeMask(masterChart, currentIndex, plotX);
			if (currentIndex) {
				screenSelRange.max = screenX;
				plotSelRange.max = plotX;
			} else {
				screenSelRange.min = screenX;
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
			color: 'rgba(0, 0, 0, 0.2)',
			});
		}

	}
	
$('div.tow-inner-search-highcharts-bar-container').each(function(index){
		var id = $(this).attr('id');
		var datastring = $(this).parent().children('form').children('div').children('[name="data"]').val();
		var namestring = $(this).parent().children('form').children('div').children('[name="tooltips"]').val();
		var fieldtype = $(this).parent().children('form').children('div').children('[name="fieldtype"]').val();
		datastring = datastring.substring(2, datastring.length - 2);
		datastring = datastring.split(', ');//$(this).text(id);
		namestring = namestring.substring(3, namestring.length - 3);
		var names = namestring.split('", "');//$(this).text(id);
		var data = [];
		for (var key in datastring) {
			var val = datastring[key];
			data[key] = parseInt(val);
		}
		chart = plotBarchart(id, names, data);
	});


	function plotBarchart(id, categories, data) {
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
            allowPointSelect: true
        }
				// series: {
					// marker: {
						// enabled: false,
						// states: {
							// hover: {
								// enabled: true
							// }
						// }
					// },
					// lineWidth: 1
				// }
			 },
			xAxis: {
				title: {
					text: null
				},
				categories: categories
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
				// formatter: function() {
					// tooltip = this.point.name;
					// tooltip = tooltip.replace(/\\\\n/g,'<br/>');
					// return tooltip;
				// },
				// crosshairs: true,
				// style: {
					// fontSize: '7pt'
				// }
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
	$('#content-messages-inner').before(message + ' | ');
}