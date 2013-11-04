//My uniaue namespace
var widgets = {}; //global Object container

//DataTable
var oTable;

//data
var dataArray;

//'date' format
var formatDate = d3.time.format.utc("%Y-%m-%d");
var formatTime = d3.time.format.utc("%H:%M:%S");
var formatDatetime = d3.time.format.utc("%Y-%m-%d %H:%M:%S");

//SelectedFields
var selectedFields = [0,2,3];

//Selected Cell
var selectedCell = {};

//Saved searches
var savedSearches = [];
//var savedSearches = [
//	{
//	'id': 1,
//	'dataset_id': 1,
//	'title': 'ss1',
//	'body': 'this is ss1',
//	'data': {
//		'4': [],
//		'5': [],
//		'6': ["Low"],
//		'7': []
//		},
//	'selected_cell': {
//		'c_row': 'row-42',
//		'c_index': 1,
//		'c_value': '2019.8',
//		'c_field': 'Drainage Area (sq_km)'
//		}
//	},
//	{
//	'id': 2,
//	'dataset_id': 1,
//	'title': 'ss2',
//	'body': 'this is ss2',
//	'data': {
//		'2': ["Alberta", "Ontario"],
//		'3': [],
//		'4': [],
//		'6': ["Low"],
//		'7': [[154.2857142857143, 198.0952380952381]]
//		},
//	'selected_cell': {
//		'c_row': 'row-22',
//		'c_index': 4,
//		'c_value': '198',
//		'c_field': 'Low Water Quantity (days)'
//		}
//	},
//	{
//	'id': 3,
//	'dataset_id': 1,
//	'title': 'ss3',
//	'body': 'this is ss3',
//	'data': {
//		'2': ["Alberta", "British Columbia", "Nunavut", "Ontario"],
//		'6': ["Normal"],
//		'9': [[54.2857142857143, 98.0952380952381]]
//		},
//	'selected_cell': {}
//	}
//
//];

Drupal.behaviors.mdb_inner_search = function(context) {

    var port = Drupal.settings.tow_mongodb.nodejs_port;
    var dataset_nid = Drupal.settings.tow_mongodb.dataset_nid;
    var user_id = Drupal.settings.tow_mongodb.user_id;
    
    function init() {

        $.ajax({
            url: 'http://' + location.host + ':' + port + '/dataset',
            type: 'GET',
            data: {
                dataset_nid: dataset_nid,
                user_id: user_id
            },
            dataType: 'jsonp',
            async: false,
            jsonpCallback: 'parseJsonp',
            success: function(datamdb) {
                
                $('#block-tow-search_inner_field_list').remove();
                $('#block-tow-saved_searches_description').remove();
                
                FieldTitleArray = {};
                NumericFieldsIndeces = [];
                DateFieldsIndeces = [];
                var HeaderLength = datamdb[0].tables[0].header.length;
                for (var h = 0; h < HeaderLength; h++) {
                    FieldTitleArray[datamdb[0].tables[0].header[h]['name']] = datamdb[0].tables[0].header[h]['title'];
                    if (datamdb[0].tables[0].header[h]['type'] == 'int' || datamdb[0].tables[0].header[h]['type'] == 'float') {
                        NumericFieldsIndeces.push(h);
                    } else if (datamdb[0].tables[0].header[h]['type'] == 'date' || datamdb[0].tables[0].header[h]['type'] == 'datetime' || datamdb[0].tables[0].header[h]['type'] == 'time' || datamdb[0].tables[0].header[h]['type'] == 'timestamp') {
                        DateFieldsIndeces.push(h);
                    }
                }

                dataArray = [];
                var RowsLength = datamdb[0].tables[0].rows.length;
                for (var r = 0; r < RowsLength; r++) {
                    NewRow = {};
                    Object.keys(datamdb[0].tables[0].rows[r]).forEach(function (key, index) {
                        if (key != '_id') {
                            var TowFieldIndex = key.indexOf('_tow_field_');
                            var LastUnderscoreIndex = key.lastIndexOf('_');
                            var type = key.substring(TowFieldIndex + 11, LastUnderscoreIndex);
                            if (type == 'int' || type == 'float') {
                                if (datamdb[0].tables[0].rows[r][key] !== null) {
                                    NewRow[FieldTitleArray[key]] = parseFloat(datamdb[0].tables[0].rows[r][key]);
                                } else {
                                    NewRow[FieldTitleArray[key]] = null;
                                }
                            } else if (type == 'date' || type == 'datetime' || type == 'time') {
                                NewRow[FieldTitleArray[key]] = new Date(datamdb[0].tables[0].rows[r][key]);
                            } else if (type == 'timestamp') {
                                NewRow[FieldTitleArray[key]] = new Date(datamdb[0].tables[0].rows[r][key] * 1000);
                            } else if (type == 'enum') {
                                NewRow[FieldTitleArray[key]] = datamdb[0].tables[0].header[index].enum_options[datamdb[0].tables[0].rows[r][key]];
                            } else {
                                NewRow[FieldTitleArray[key]] = datamdb[0].tables[0].rows[r][key];
                            }
                        }
                    });
                    dataArray.push(NewRow);
                }

                console.log(datamdb);
                //console.log(dataArray);
                
                
                //$(function(){
	
                App = Ember.Application.create({
                    rootElement: '#content-group',
                    LOG_TRANSITIONS: true
                });

                App.ChartView = Ember.View.extend ({
                    didInsertElement: function() {
                        // create widget and append it to this.$()
			
                        var tableColumns = [];
                        var ndx;
                        var gridster;
                        var data;
                        data = dataArray.slice();

                        // load data from a json
                        //data = dataArray;

                        // feed it through crossfilter
                        ndx = crossfilter(data);
                        var all = ndx.groupAll();
                        var step = 25;
					
                        var tableHeader = '';
                        var fieldButtons = ''
				
                        var singleRowObj = data[0];
                        var i = 0;
                        for (var j in singleRowObj) {

                            var isSelected = $.inArray(i, selectedFields);
                            var selected = isSelected != -1 ? 'selected' : '';
                            fieldButtons = fieldButtons + '<button class="' + selected + ' btn btn-mini">' + j + '</button>';
					
                            $('#charts').append('<div class="chart-widget"><p class="widget-title">' + j + '</p><div class="chart-wrapper"><div id="chart' + i + '"><a class="reset" href="javascript:widgets.chart' + i + '.filterAll();dc.redrawAll();" style="display: none; top: 18px">Reset chart filters</a></div><div class="chart-filter"></div></div></div>');
					
                            var isDate = $.inArray(i, DateFieldsIndeces);

                            widgets['chart' + i + '_dim'] = ndx.dimension(function (d) {
                                return d[j];
                            });

                            widgets['chart' + i + '_dim_foo'] = chartDimension(j, datamdb[0].tables[0].header[i]['type']);

                            var isNumeric = $.inArray(i, NumericFieldsIndeces);
                            if (isNumeric == -1 && isDate == -1) {
                                widgets['chart' + i] = dc.rowChart("#chart" + i);
                                $('#chart' + i).addClass('row-chart');
                                widgets['chart' + i + '_grp'] =	widgets['chart' + i + '_dim'].group();
                                var groupSize = widgets['chart' + i + '_grp'].size();
                                var height = groupSize > 9 ? groupSize * 20 : 200;
                                var ticks = groupSize == data.length ? 0 : 4;
                                var colors = groupSize == data.length ? ['#999'] : randomArrayOfColors(groupSize);
						
                                drawRowChart(widgets['chart' + i], height, widgets['chart' + i + '_dim'], widgets['chart' + i + '_grp'], colors, i, ticks);
						
                            } else {
                                var xExtent = d3.extent(data, function(d) {
                                    return d[j];
                                });

                                var stepWidth = (xExtent[1] - xExtent[0])/step;
                                if (stepWidth == 0) {
                                    widgets['chart' + i] = dc.rowChart("#chart" + i);
                                    $('#chart' + i).addClass('row-chart');
                                    widgets['chart' + i + '_grp'] = widgets['chart' + i + '_dim'].group();
                                    var height = 200;
                                    var ticks = 4;
                                    var colors = randomArrayOfColors(1);
                                    var NumIndex = NumericFieldsIndeces.indexOf(i);
                                    NumericFieldsIndeces.splice(NumIndex, 1);

                                    drawRowChart(widgets['chart' + i], height, widgets['chart' + i + '_dim'], widgets['chart' + i + '_grp'], colors, i, ticks);
                                } else {
                                    widgets['chart' + i] = dc.barChart("#chart" + i);
                                    $('#chart' + i).addClass('bar-chart');
                                    widgets['chart' + i + '_grp'] =	widgets['chart' + i + '_dim'].group(function(v) {
                                        return Math.floor(v / stepWidth) * stepWidth;
                                    });
                                                    
                                    drawBarChart(widgets['chart' + i], widgets['chart' + i + '_dim'], widgets['chart' + i + '_grp'], xExtent, stepWidth, i, datamdb[0].tables[0].header[i]['type']);
                                }
                            }

                            //Remove unselected charts
                            if (isSelected == -1) {
                                widgets['chart' + i + '_dim'].remove();
                                $('#chart' + i).closest('.chart-widget').addClass('hidden');
                            } else {
                                tableHeader = tableHeader + '<th field-index="' + i + '">' + j + '</th>';
                                addFunctionToArray(tableColumns,j, datamdb[0].tables[0].header[i]['type']);
                            }
					
					
                            i++;
                        }

                        $('.dc-data-table').append('<thead><tr class="header">' + tableHeader + '</tr></thead>');

                        $('#charts').prepend('<div class="btn-group" data-toggle="buttons-checkbox">' + fieldButtons + '</div>');
                        $('#charts').prepend('<div class="dc-data-count"><span class="filter-count"></span> selected out of <span class="total-count"></span> records | <a class="reset-all" href="javascript:dc.filterAll(); dc.renderAll();">Reset All</a><p><a class="save-this-search">Save this search</a></p></div>');

                        var savedSearchesOutput = '';
                        for (var ss=0; ss < savedSearches.length; ss++) {
                            var sC = !$.isEmptyObject(savedSearches[ss].selected_cell) ? '<span class="label label-info sc">' + savedSearches[ss].selected_cell.c_field + ': ' + savedSearches[ss].selected_cell.c_value + '</span>' : '';
                            savedSearchesOutput = savedSearchesOutput + '<div class="ss"><a ss-id="' + savedSearches[ss].id + '">' + savedSearches[ss].title +'</a><p>' + savedSearches[ss].body +'</p>' + sC + '</div>'
                        }
//                        $('#charts').prepend('<div class="ss-block"><h3>Searches</h3>' + savedSearchesOutput + '</div>');
			
                        dc.dataCount(".dc-data-count")
                        .dimension(ndx)
                        .group(all);
                                                
                        widgets['dataTable'] = dc.dataTable(".dc-data-table");
                        widgets['dataTable'].dimension(widgets.chart0_dim)
                        .group(function () {
                            return 'All dataset records you\'ve got';
                        })
                        .size(10)
                        .columns(tableColumns)
//                        .on("preRender", function(chart){
//                            if (typeof(oTable) !== 'undefined') {
//                                oTable.fnDestroy();
//                            }
//                        })
//                        .on("postRender", function(chart){
//                            $('tr.dc-table-group').remove();
//							
//                            $('#datatable-1 tbody tr').each(function(index) {
//                                $(this).addClass('row-' + index);
//                            });
//
//                            addDataTables();
//
//                            //Apply selected cell if exists
//                            if (!$.isEmptyObject(selectedCell)) {
//                                var sCTd = $('#datatable-1 tbody tr.' + selectedCell.c_row + ' td:eq(' + selectedCell.c_index + ')');
//                                sCTd.attr('c_row', selectedCell.c_row);
//                                sCTd.attr('c_index', selectedCell.c_index);
//                                sCTd.attr('c_value', selectedCell.c_value);
//                                sCTd.attr('c_field', selectedCell.c_field);
//                                sCTd.addClass('selected-cell');
//                                selectedCell = {};
//                            }
//                        })
//                        .on("preRedraw", function(chart){
//                            if (typeof(oTable) !== 'undefined') {
//                                oTable.fnDestroy();
//                            }
//                        })
//                        .on("postRedraw", function(chart){
//                            $('tr.dc-table-group').remove();
//							
//                            $('#datatable-1 tbody tr').each(function(index) {
//                                $(this).addClass('row-' + index);
//                            });
//							
//                            addDataTables();
//                        })
//                        ;

                        dc.renderAll();

                        /*
				 * Selected fields functionality
				 */
                        //Add field
                        $('.btn-group button:not(.selected)').live('click', function() {
                            var buttonIndex = $(this).index();
                            var buttonText = $(this).text();
                            addFieldToCrossfilter (ndx, buttonIndex, buttonText, data, step, tableColumns, selectedFields, datamdb[0].tables[0].header[buttonIndex]['type']);
					
                            $(this).addClass('selected');
                        });
                        //Remove field
                        $('.btn-group button.selected').live('click', function() {
                            var buttonIndex = $(this).index();
                            removeFieldFromCrossfilter(buttonIndex, tableColumns, selectedFields);
					
                            $(this).removeClass('selected');
                        });
				
                        //Choose saved search. Apply filters
                        //$('333.accordion-group a.accordion-toggle').live('click', function() {
                        //
                        $('#accordion', context).live('shown', function (e) {
                            //Clear all previous filters
                            dc.filterAll();

//                            var ssId = $(this).attr('ss-id');
//                            var result = $.grep(savedSearches, function(e){
//                                return e.id == ssId;
//                            });
//                            if (result.length == 1) {
                              
                              var filters_str = $(e.target).closest('div.accordion-group').find('p.ss-filters').text();
                              var ss_data = $.parseJSON(filters_str);
                              console.log(ss_data);
                                var newSelectedFields = [];
                                for (var sf in ss_data) {
                                    newSelectedFields.push(parseInt(sf));
                                }
						
                                //Remove waste selected fields
                                var selectedFieldsToRemove = [];
                                $.each(selectedFields, function(index, value) {
                                    var isAlreadySelected = $.inArray(value, newSelectedFields);
							
                                    if (isAlreadySelected == -1) {
                                        selectedFieldsToRemove.push(value);
                                    }
                                });
                                for (var sftr=0; sftr < selectedFieldsToRemove.length; sftr++) {
                                    removeFieldFromCrossfilter(selectedFieldsToRemove[sftr], tableColumns, selectedFields, false);
                                    $('.btn-group button:eq(' + selectedFieldsToRemove[sftr] + ')').removeClass('selected');
                                }
						
                                //Add new selected fields
                                var selectedFieldsToAdd = [];
                                $.each(newSelectedFields, function(index, value) {
                                    var isAlreadySelected = $.inArray(value, selectedFields);
                                    if (isAlreadySelected == -1) {
                                        selectedFieldsToAdd.push(value);
                                    }
                                });
                                for (var sfta=0; sfta < selectedFieldsToAdd.length; sfta++) {
                                    var buttonText = $('.btn-group button:eq(' + selectedFieldsToAdd[sfta] + ')').text();
                                    addFieldToCrossfilter (ndx, selectedFieldsToAdd[sfta], buttonText, data, step, tableColumns, selectedFields, datamdb[0].tables[0].header[selectedFieldsToAdd[sfta]]['type'], false);
                                    $('.btn-group button:eq(' + selectedFieldsToAdd[sfta] + ')').addClass('selected');
                                }
						
                                //Apply filters to saved search selected fields
                                for (var ssf=0; ssf < selectedFields.length; ssf++) {
                                    if (ss_data[selectedFields[ssf]].length !== 0) {
                                        if (ss_data[selectedFields[ssf]].length === 1) {
                                            widgets['chart' + selectedFields[ssf]].filter(ss_data[selectedFields[ssf]][0]);
                                        } else {
                                            for (var mf=0; mf < ss_data[selectedFields[ssf]].length; mf++) {
                                                widgets['chart' + selectedFields[ssf]].filter(ss_data[selectedFields[ssf]][mf]);
                                            }
                                        }
                                    }
                                }
                                //Get selected cell if exists
//                                if (!$.isEmptyObject(ss_data.selected_cell)) {
//                                    $.extend(selectedCell, ss_data.selected_cell);
//                                }
                            //}
					
                            dc.renderAll();
					
                        });
				
                        /*
				//Gridster
				$('#charts div.chart-widget').wrapAll('<ul></ul>');
				$('#charts div.chart-widget:not(.hidden)').each(function(index){
					var row = Math.floor(index/5) + 1;
					var col = (index % 5) + 1;
					$(this).wrap('<li data-row="' + row + '" data-col="' + col + '" data-sizex="1" data-sizey="1"></li>');
				});
				gridster = $('.gridster ul').gridster().data('gridster');
				*/
				
                        //Selected cell
                        $('#datatable-1 tbody tr td').live('click', function() {
                            if ($(this).hasClass('selected-cell')) {
                                $(this).removeAttr('c_row c_index c_value c_field');
                                $(this).removeClass('selected-cell');
                            } else {
                                $('.selected-cell').removeAttr('c_row c_index c_value c_field');
                                $('.selected-cell').removeClass('selected-cell');
                                $(this).addClass('selected-cell');
                                var c_row;
                                $($(this).closest('tr').attr('class').split(' ')).each(function() {
                                    if (this.indexOf('row-') > -1) {
                                        c_row = this;
                                    }
                                });
                                var c_index = $(this).index();
                                var c_value = $(this).text();
                                var c_field = $('table.dataTable thead tr th:eq(' + c_index + ')').text();
                                $(this).attr('c_row', c_row);
                                $(this).attr('c_index', c_index);
                                $(this).attr('c_value', c_value);
                                $(this).attr('c_field', c_field);
                            }
                        });
                        //Remove selection on Esc pressing
                        $(document).keyup(function(e) {
                            if (e.which == 27) {
                                $('.selected-cell').removeAttr('c_row c_index c_value c_field');
                                $('.selected-cell').removeClass('selected-cell');
                            }
                        });
				
			
                        //Send to ember handlebar
                        //this.$().appendTo("body");
			
                        //Clear filters
                        $('a.reset').live('click', function() {
                            $(this).closest('.dc-chart').siblings('.chart-filter').html('');
                        });
                        $('a.reset-all').live('click', function() {
                            $('.chart-widget .chart-filter').html('');
                        });
			
                        //Hide last filter for row charts
                        $('.chart-widget .dc-chart.row-chart svg g g').live('click', function() {
                            if ($(this).children('rect').attr('class') == '' && $(this).closest('svg').siblings('a.reset').is(':hidden')) {
                                $(this).closest('.dc-chart').siblings('.chart-filter').html('');
                            }
                        });
			
                        //Save this search
                        $('.save-this-search').live('click', function() {
                            var savedSearchFilters = {};
                            $('.btn-group button.selected').each(function() {
                                var selectedFieldIndex = $(this).index();
                                savedSearchFilters[selectedFieldIndex] = widgets['chart' + selectedFieldIndex].filters();
                            });
                            console.log(JSON.stringify(savedSearchFilters));
                        });
			
                    }
                });
	
	

	
            //});




                
            },
            error: function(jqXHR) {
                console.log(jqXHR);
            }
        });
    }

    function parseJsonp(json) {
        return json;
    }

    init();
};

function addFunctionToArray(array,key,date) {
    if(typeof(date)==='undefined') date = false;
    switch (date) {
        case 'date':
            var foo = function(d) {
                return '<span>' + formatDate(d[key]) + '</span>';
            };
            break;
            
        case 'datetime':
        case 'timestamp':
            var foo = function(d) {
                return '<span>' + formatDatetime(d[key]) + '</span>';
            };
            break;
        
        case 'time':
            var foo = function(d) {
                return '<span>' + formatTime(d[key]) + '</span>';
            };
            break;
            
        default:
            var foo = function(d) {
                return '<span>' + d[key] + '</span>';
            };
            break;
    }
    array.push(foo);
}

function filterPrinterFunction(type, key, date) {
    if(typeof(date)==='undefined') date = false;
    var foo = function(d) {
        if (type == 'row') {
            var chartFilters = 'Filters:';
            for(f=0; f < d.length; f++) {
                chartFilters = chartFilters + ' <span class="label">' + d[f] + '</span>';
            }
        } else if (type == 'bar') {
            switch (date) {
                case 'date':
                    var chartFilters = 'Selected Range: from <span class="label">' + formatDate(d[0][0]) + '</span> to <span class="label">' + formatDate(d[0][1]) + '</span>';
                    break;

                case 'datetime':
                case 'timestamp':
                    var chartFilters = 'Selected Range: from <span class="label">' + formatDatetime(d[0][0]) + '</span> to <span class="label">' + formatDatetime(d[0][1]) + '</span>';
                    break;
                    
                case 'time':
                    var chartFilters = 'Selected Range: from <span class="label">' + formatTime(d[0][0]) + '</span> to <span class="label">' + formatTime(d[0][1]) + '</span>';
                    break;

                default:
                    var chartFilters = 'Selected Range: from <span class="label">' + d[0][0].toFixed(2) + '</span> to <span class="label">' + d[0][1].toFixed(2) + '</span>';
                    break;
            }
        }
        $('#chart' + key).siblings('.chart-filter').html('<p>' + chartFilters + '</p>');
    }
    return foo;
}

function chartDimension(key, date) {
    if(typeof(date)==='undefined') date = false;
    switch (date) {
        case 'date':
            var foo = function(d) {
                return formatDate(d[key]);
            };
            break;
            
        case 'datetime':
        case 'timestamp':
            var foo = function(d) {
                return formatDatetime(d[key]);
            };
            break;
            
        case 'time':
            var foo = function(d) {
                return formatTime(d[key]);
            };
            break;
            
        default:
            var foo = function(d) {
                return d[key];
            };
            break;
    }
    
    return foo;
}

function drawRowChart(chartObject, height, chart_dim, chart_grp, colors, index, ticks) {
    chartObject.width(300) // (optional) define chart width, :default = 200
    .height(height) // (optional) define chart height, :default = 200
    .dimension(chart_dim) // set dimension
    .group(chart_grp) // set group
    .margins({
        top: 20, 
        left: 10, 
        right: 10, 
        bottom: 20
    })
    .colors(colors)
    .gap(1)
    .filterPrinter(filterPrinterFunction('row', index))
    .elasticX(true)
    .renderLabel(true)
    .renderTitle(true)
    .xAxis().ticks(ticks);
}

function drawBarChart(chartObject, chart_dim, chart_grp, xExtent, stepWidth, index, type) {
    if(typeof(type)==='undefined' || (type != 'date' && type != 'datetime' && type != 'time' && type != 'timestamp')) type = false;
    var x = (type !== false) ? d3.time.scale.utc().domain(xExtent).range([0, stepWidth]) : d3.scale.linear().domain(xExtent).range([0, stepWidth]).nice();
    //var tickFoo = (date !== false) ? function(v) {return formatDate(new Date(v));} : function(v) {return v;};
    
    chartObject.width(300)
    .height(200)
    .margins({
        top: 10, 
        right: 50, 
        bottom: 30, 
        left: 40
    })
    .dimension(chart_dim)
    .group(chart_grp)
    .elasticY(true)
    .x(x)
    .xUnits(function(){
        return 20;
    })
    .gap(3)
    .filterPrinter(filterPrinterFunction('bar', index, type))
    .renderHorizontalGridLines(true)
    .title(function(v) {
        return v.value;
    })
    .renderTitle(true)
    .on("filtered", function(chart, filter){
        var chartId = chart.root()[0][0].id;
        var chartFilterDiv = $('#' + chartId).siblings('.chart-filter');
        if (filter === null && chartFilterDiv.html() != '') {
            chartFilterDiv.html('');
        }
    })
    .xAxis().ticks(4)/*.tickFormat(tickFoo)*/;
}

function addDataTables() {
    //Numeric column indices
    var arrayOfColumnIndices = [];
    $.each(selectedFields, function(index, value) {
        if ($.inArray(value, NumericFieldsIndeces) !== -1) {
            arrayOfColumnIndices.push(index);
        }
    });

    //Custom sorting function
    $.extend($.fn.dataTableExt.oSort, {
        "formatted_numbers-pre": function ( a ) {
            a = (a==="-") ? 0 : a.replace( /[^\d\-\.]/g, "" );
            return parseFloat( a );
        },

        "formatted_numbers-asc": function ( a, b ) {
            return a - b;
        },

        "formatted_numbers-desc": function ( a, b ) {
            return b - a;
        }
    });
	
    //DataTables initialization
    oTable = $('#datatable-1').dataTable({
        "sDom": 'C<"clear">rti',
        "oColVis": {
            "fnLabel": function (index, title, th) {
                return (index + 1) + '. ' + title;
            }
        },
        "aoColumnDefs": [
        {
            "sType": "formatted_numbers", 
            "aTargets": arrayOfColumnIndices
        }
        ],
        "bDeferRender": true,
        "bAutoWidth": false,
        "bStateSave": true,
        "bPaginate": false,
        "bSort": true,
        "bDestroy": true,
        "bInfo": false,
        "bRetrieve": true,
        "asStripeClasses": []
    });
    //new FixedHeader( oTable );
	
    //ColVis icon
    $(".ColVis_Button.TableTools_Button.ColVis_MasterButton").append('<i class="icon-filter"></i>');
}

function randomArrayOfColors(n) {
    var array = [];
    var letters = '0123456789ABCDEF'.split('');
    for (var c = 0; c < n; c++) {
        var color = '#';
        for (var i = 0; i < 6; i++ ) {
            color += letters[Math.round(Math.random() * 15)];
        }
        array.push(color);
    }
    return array;
}

function removeFieldFromCrossfilter(buttonIndex, tableColumns, selectedFields, redraw) {
    if(typeof(redraw)==='undefined') redraw = true;
    widgets['chart' + buttonIndex].filterAll();
    $('#chart' + buttonIndex).closest('.dc-chart').siblings('.chart-filter').html('');
    widgets['chart' + buttonIndex + '_dim'].remove();
    $('#chart' + buttonIndex).closest('.chart-widget').addClass('hidden');

    $('.btn-group button.selected').each(function(index) {
        if ($(this).index() == buttonIndex) {
            tableColumns.splice(index,1);
            selectedFields.splice(index, 1);
            $('th[field-index="'+ buttonIndex + '"]').remove();
            return false;
        }
    });

    if (redraw === true) {
        dc.renderAll();
    }
}

function addFieldToCrossfilter (ndx, buttonIndex, buttonText, data, step, tableColumns, selectedFields, type, redraw) {
    if(typeof(redraw)==='undefined') redraw = true;
    widgets['chart' + buttonIndex + '_dim'] = ndx.dimension(function (d) {
        return d[buttonText];
    });
    var groupSize = widgets['chart' + buttonIndex + '_grp'].size();
    var height = groupSize > 9 ? groupSize * 20 : 200;
    var ticks = groupSize == data.length ? 0 : 4;
    var colors = groupSize == data.length ? ['#999'] : randomArrayOfColors(groupSize);
    if ($('#chart' + buttonIndex).hasClass('row-chart')) {
        widgets['chart' + buttonIndex + '_grp'] = widgets['chart' + buttonIndex + '_dim'].group();
        widgets['chart' + buttonIndex] = dc.rowChart('#chart' + buttonIndex);
        drawRowChart(widgets['chart' + buttonIndex], height, widgets['chart' + buttonIndex + '_dim'], widgets['chart' + buttonIndex + '_grp'], colors, buttonIndex, ticks);
    } else if ($('#chart' + buttonIndex).hasClass('bar-chart')) {
        var xExtent = d3.extent(data, function(d) {
            return d[buttonText];
        });
        var stepWidth = (xExtent[1] - xExtent[0])/step;
        widgets['chart' + buttonIndex + '_grp'] = widgets['chart' + buttonIndex + '_dim'].group(function(v) {
            return Math.floor(v / stepWidth) * stepWidth;
        });
        widgets['chart' + buttonIndex] = dc.barChart('#chart' + buttonIndex);
        drawBarChart(widgets['chart' + buttonIndex], widgets['chart' + buttonIndex + '_dim'], widgets['chart' + buttonIndex + '_grp'], xExtent, stepWidth, buttonIndex, type);
    }

    var columnAdded = false;
    $('.btn-group button.selected').each(function(index) {
        if ($(this).index() > buttonIndex) {
            tableColumns.splice(index, 0, function(d) {
                return '<span>' + d[buttonText] + '</span>';
            });
            selectedFields.splice(index, 0, buttonIndex);
            $('th[field-index="'+ $(this).index() + '"]').before('<th field-index="' + buttonIndex + '">' + buttonText + '</th>');
            columnAdded = true;
            return false;
        }
    });

    if (columnAdded === false) {
        tableColumns.push(function(d) {
            return '<span>' + d[buttonText] + '</span>';
        });
        selectedFields.push(buttonIndex);
        $('.dc-data-table tr.header').append('<th field-index="' + buttonIndex + '">' + buttonText + '</th>');
    }

    $('#chart' + buttonIndex).closest('.chart-widget').removeClass('hidden');

    if (redraw === true) {
        dc.renderAll();
    }
}