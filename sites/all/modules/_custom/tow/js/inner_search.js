Drupal.behaviors.inner_search = function(context) {

    /* ****************************************************************************************************
     * VARIABLES
     * ****************************************************************************************************/

    // Highcharts constants.
    var handleHeight = 15; // Real handle height.
    var handleWidth = 6; // Real handle width.
    var handleBorder = 1; // Real handle border.
    var visibleHandleHeight = 4; // Visible handle height.
    var visibleHandleWidth = 5; // Visible handle width.
    var visibleHandleBorder = 1; // Visible handle border.
    var visibleHandleOffset = 2; // Visible handle offset.
    var visibleBoxBorder = 1; // Box border.

    // Global variables.
    var arrayOfCharts = {}; // An array of Master Charts.
    var arrayOfZooms = {}; // An array of Detail Charts.
    var arrayOfCollapse = {}; // Widgets collapsibility information.
    var firstPageLoad = true;
    var firstPageLoadFieldsCount = 10;

    // Saving/memorizing.
    var currentHash = location.hash; // Current location hash.
    var sortCurrentTitleDirection = ''; // Current direction of sorting by titile.
    var sortCurrentTypeDirection = ''; // Current direction of sorting by type.
    var initialArrayOfWidgets; // Set of search widgets with 'on load' sorting.
    var minCur; // Current active chart minimum.
    var maxCur; // Current active chart maximum.



    /* ****************************************************************************************************
     * BEHAVIORS
     * ****************************************************************************************************/

    /***** Page *****/

    // Hashchange.
    $(window).hashchange(function() {
        if (firstPageLoad) {
            if (window.location.hash == '') {
                var counter = 0;
                var selectedFields = {};
                $('.btn-group.table-field-buttons-1').children().each(function(index) {
                    if (counter < firstPageLoadFieldsCount) {
                        $(this).addClass('selected');
                        var text = $(this).text();
                        var type = $(this).attr('f_type');
                        $('.tow-dataset-field-link').each(function() {
                            if ($(this).text() == text && ($(this).attr('f_type') == type)) {
                                $(this).addClass('selected');
                            }
                        });
                        selectedFields[$(this).text() + '_' + $(this).attr('f_type')] = $(this).text() + '_' + $(this).attr('f_type');
                        counter++;
                    }
                });
                firstPageLoad = false;
                var selectedFieldsToSend = JSON.stringify(selectedFields);
                var url = 'http://' + window.location.hostname + window.location.pathname + '#?' + 'selected_fields=' + selectedFieldsToSend;
                setHash(url);
            }
            else {
                hashChange();
            }
        }
        else {
            hashChange();
        }
    });
    $(window).hashchange();


    /***** Widgets sorting *****/

    // Widgets sorting .
    $('.tow-inner-search-widget-sort a[href*="title"]').live('click', function(e) {
        sortWidgetsLinkClick(e, 'title', sortCurrentTitleDirection);
    });
    $('.tow-inner-search-widget-sort a[href*="type"]').live('click', function(e) {
        sortWidgetsLinkClick(e, 'type', sortCurrentTypeDirection);
    });

    // Sort: reset all. ***Changed to reset all filters***
    $('.search-link-reset-all').live('click', function(e) {
        searchResetAll(e, $(this));
    });



    /***** Search *****/

    // Simple search.
    $('#edit-keywords').change(function() {
        searchSimple($(this));
    });

    // Search button click.
    $('.form-submit.search-button').live('click', function(e) {
        searchSubmit(e, $(this));
    });

    // Reset link click.
    $('.active').live('click', function(e) {
        searchResetButtonClick(e, $(this));
    });

    // Dataset field click.
    $('.tow-dataset-field-link').live('click', function(e) {
        searchSelectField(e, $(this));
    });

    // Dataset field hover.
    $('.tow-dataset-field-link').live({
        mouseenter: function() {
            searchFieldHoverIn($(this));
        },
        mouseleave: function() {
            searchFieldHoverOut($(this));
        }
    });

    // Facet click.
    $('a.apachesolr-facet, a.apachesolr-hidden-facet, .tow-inner-search-selected').live('click', function(e) {
        searchFacetClickUpdate(e, $(this));
    });

    // Hide hidden facets.
    $('.apachesolr-hidden-facet', context).addClass('hidden');

    // Add 'Show more/fewer' link.
    $('<a href="#" class="apachesolr-showhide"></a>')
    .text(Drupal.t('Show more'))
    .click(function(e) {
        searchFacetsMoreFewer(e, $(this));
    })
    .appendTo($('.item-list:has(.apachesolr-hidden-facet)', context));

    // Text fields search.
    $('#block-tow-search_inner_facets .form-text').live('change', function() {
        searchTextFieldChange($(this));
    });

    // AJAX highcharts "on-line" reloading.
    $('.tow-inner-search-highcharts-container .detail-container').live('mouseup', function(e) {
        searchNumericWidgetChangeRange(e, $(this));
    });

    // Including/excluding empty values.
    $('[name="include_empty"]').live('click', function(e) {
        searchIncludeEmptyValues(e, $(this));
    });



    /***** Highcharts *****/

    // Highcharts basic chart behavior.
    $('div.tow-inner-search-highcharts-container').each(function(index) {
        highchartsTestChart($(this));
    });

    // Highcharts bar chart behavior.
    $('div.tow-inner-search-highcharts-bar-container').each(function(index) {
        highchartsTestBarChart($(this));
    });



    /***** Saved Searches *****/

    // Save search button click.
    $('#edit-save-search').live('click', function(e) {
        saveSearch(e);
    });

    // Delete saved search link click.
    $('.saved-search-delete').live('click', function(e) {
        deleteSavedSearch(e, $(this));
    });

    // Saved search tags autocomplete.
    var timer = null;
    $('#edit-ss-tags').live('keyup', function(e) {
        clearTimeout(timer);
        $this = $(this);
        timer = setTimeout(function() {
            ssTagsAutocomplete(e, $this);
        }, 600);
    });

    // Delete comment.
    $('.comment_delete a').live('click', function(e) {
        commentDelete(e, $(this));
    });

    // Replacements of comment links with comment forms.
    $('.comment_edit a').live('click', function(e) {
        commentForm(e, $(this), 'edit');
    });
    $('.comment_reply a').live('click', function(e) {
        commentForm(e, $(this), 'reply');
    });

    // Save comment.
    $('[id^="comment-form"] .form-submit').live('click', function(e) {
        commentSave(e, $(this));
    });



    /* ****************************************************************************************************
     * CALBACKS
     * ****************************************************************************************************/

    /***** Page callbacks *****/

    /**
     * Hash change callback.
     */
    function hashChange() {
        hash = location.hash;
        filtersToSend = getUrlQueryParam(hash, 'filters');
        selectedFieldsTosend = getUrlQueryParam(hash, 'selected_fields');
        if (hash != currentHash) {
            innerSearchProcessing(filtersToSend, selectedFieldsTosend);
        }
        else if (hash == currentHash) {
            $(window).load(function () {
                innerSearchProcessing(filtersToSend, selectedFieldsTosend);
            });
        }
        currentHash = hash;
    }



    /***** Widgets sorting callbacks *****/

    /**
     * Widgets sorting link click.
     */
    function sortWidgetsLinkClick(event, type, negative_direction) {
        event.preventDefault();
        if (negative_direction == 'asc') {
            sortWidgets(type, 'desc');
        }
        else {
            sortWidgets(type, 'asc');
        }
    }

    /**
     * Widgets sorting reset. ***Reset all filters***
     */
    function searchResetAll(event, selector) {
        /*
        event.preventDefault();

        sortCurrentTitleDirection = '';
        sortCurrentTypeDirection = '';

        $('.tow-inner-search-widget-sort a[href*="title"]').text('title');
        $('.tow-inner-search-widget-sort a[href*="type"]').text('type');
        $('#tow-search-inner-hash-form').after(initialArrayOfWidgets);
        */

        var url = selector.attr('href');
        setHash(url);

        event.preventDefault();
    }



    /***** Search callbacks *****/

    /**
     * Simple search callback.
     */
    function searchSimple(selector) {
        var keywords = selector.val();
        keywords = encodeURIComponent(keywords);
        window.location.href = 'http://' + location.host + '/' + keywords + location.search;
    }

    /**
     * Submission of search widget.
     */
    function searchSubmit(event, selector) {

        // Mouse left button click.
        if (event.button == 0) {
            event.preventDefault();

            // Get values.
            var field = selector.parent().children('[name="field"]').val();
            var url = selector.parent().children('[name="url"]').val();
            var type = selector.parent().children('[name="fieldtype"]').val();
            var from_str = field + '_placeholder';
            var to_str;

            // Case int, float, time, date, datetime, timestamp:
            if (type == 'int' || type == 'float' || type == 'time' || type == 'date' || type == 'datetime' || type == 'timestamp') {

                // Min & max.
                var min = selector.parent().find('[name="min"]').val();
                var max = selector.parent().find('[name="max"]').val();

                // Placeholders.
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

                // Update url.
                url = url.replace(from_str, to_str);
            }
            else {

                // Case length:
                if (type == 'length') {
                    var option = selector.parent().children('.form-item').children('[name="option"]').val();
                    var value = parseInt(selector.parent().children('.form-item').children('[name="value"]').val());

                    if (option.indexOf('[* TO #]') > -1) {
                        value -= 1;
                    }
                    else {
                        if (option.indexOf('[# TO *]') > -1) {
                            value += 1;
                        }
                    }

                    // Update url.
                    query = option.replace('#', value);
                    to_str = query;
                    from_str = '%3A' + from_str;
                    url = url.replace(from_str, to_str);
                }

                // Case other types:
                else {
                    var negative_url = selector.parent().children('[name="negative_url"]').val();
                    var option = selector.parent().children('.form-item').children('[name="option"]').val();
                    var text = selector.parent().children('.form-item').children('[name="value"]').val();
                    text = text.replace(/(\+|-|&&|\|\||!|\(|\)|\{|\}|\[|\]|\^|"|~|\*|\?|:|\\\\)/g, '\\$1');
                    text = text.replace(/ /g,'+');
                    text = encodeURIComponent(text);
                    if (option.indexOf('-') > -1) {
                        url = negative_url;
                        option = option.replace('-', '');
                    }

                    // Upadate url.
                    query = option.replace('_', text);
                    to_str = query;
                    url = url.replace(from_str, to_str);
                }
            }

            setHash(url);
        }
    }

    /**
     * Reset search.
     */
    function searchResetButtonClick(event, selector) {
        if (selector.text() == 'Reset') {
            event.preventDefault();

            var myHref = selector.attr('href');
            setHash(myHref);
        }
    }

    /**
     * Dataset field click.
     */
    function searchSelectField(event, selector) {

        // Set field selection upon left mouse click only.
        if (event.button == 2) {
            return false;
        }

        // Ignore disabled fields.
        if (selector.hasClass('disabled') || selector.hasClass('unavailable')) {
            event.preventDefault();
            return false;
        }

        // Initialize necessary variables.
        var url = selector.attr("href");
        var filtersToSend = getUrlQueryParam(url, 'filters');
        var aux = getUrlQueryParam(url, 'selected_fields');
        if (aux == '') {
            aux = '{}';
        }
        var selectedFields = JSON.parse(aux);

        // Deselect selected field.
        if (selector.hasClass('selected')) {
            selector.removeClass('selected');
            var text = selector.text();
            var type = selector.attr('f_type');
            delete selectedFields[text + '_' + type];
            $('.tow-dataset-field-link').each(function() {
                if (($(this).text() == text) && ($(this).attr('f_type') == type)) {
                    var resetClass = text + '-' + type;
                    resetClass = resetClass.replace(' ', '-').toLowerCase();
                    $(this).removeClass('selected');
                    $('.' + resetClass +'.reset').click();
                }
            });
        }

        // Set the field to be selected.
        else {
            selector.addClass('selected');
            var text = selector.text();
            var type = selector.attr('f_type');
            $('.tow-dataset-field-link').each(function() {
                if ($(this).text() == text && ($(this).attr('f_type') == type)) {
                    $(this).addClass('selected');
                }
            });
        }

        // Construct selected fields param.
        $('.tow-dataset-field-link.selected').each(function() {
            selectedFields[$(this).text() + '_' + $(this).attr('f_type')] = $(this).text() + '_' + $(this).attr('f_type');
        });
        selectedFieldsToSend = JSON.stringify(selectedFields);

        // Set hash.
        setHash(url);

        // Stop page processing.
        event.preventDefault();
        return false;
    }

    /**
     * Dataset field hover mouse in.
     */
    function searchFieldHoverIn(selector) {
        var selected = selector.hasClass('selected');
        var disabled = selector.hasClass('unavailable');
        var tableset = selector.attr('tableset');
        var text = selector.text();
        var type = selector.attr('f_type');

        if (!disabled) {
            var lastSelected = 0;

            if (selected) {
                $('.tow-dataset-field-link.selected').each(function() {
                    lastSelected++;
                });
            }
            if (selected && lastSelected < 2) {
                $('.tow-dataset-field-link').each(function() {
                    $(this).addClass('available-hover');
                    $(this).addClass('hover');
                });
            }
            else {
                $('.tow-dataset-field-link').each(function() {
                    $(this).addClass('hover');
                    if ($(this).attr('tableset') == tableset) {
                        if ($(this).hasClass('selected')) {
                            $(this).addClass('selected-hover');
                        }
                        else {
                            $(this).addClass('available-hover');
                        }
                    }
                    else {
                        $(this).addClass('unavailable-hover');
                    }
                });
            }
            if (selected) {
                $('.tow-dataset-field-link').each(function() {
                    if ($(this).attr('f_type') == type && $(this).text() == text) {
                        $(this).removeClass('selected-hover');
                        $(this).addClass('available-hover');
                    }
                });
            }
            else {
                $('.tow-dataset-field-link').each(function() {
                    if ($(this).attr('f_type') == type && $(this).text() == text) {
                        $(this).addClass('selected-hover');
                        $(this).removeClass('available-hover');
                    }
                });
            }
        }
        $('.tow-dataset-field-link.unavailable-hover').each(function() {
            $(this).addClass('disabled');
            $(this).removeClass('hover');
        });
    }

    /**
     * Dataset field hover mouse out.
     */
    function searchFieldHoverOut(selector) {
        $('.tow-dataset-field-link.unavailable-hover').each(function() {
            $(this).removeClass('disabled');
        });
        $('.tow-dataset-field-link').each(function() {
            $(this).removeClass('selected-hover');
            $(this).removeClass('available-hover');
            $(this).removeClass('unavailable-hover');
            $(this).removeClass('hover');
        });
    }

    /**
     * Facet click.
     */
    function searchFacetClickUpdate(event, selector) {
        event.preventDefault();

        var url = selector.attr('href');
        var filtersToSend = getUrlQueryParam(url, 'filters');
        var selectedFieldsToSend = getUrlQueryParam(url, 'selected_fields');
        $('#edit-filters').value = filtersToSend;
        $('#edit-selected-fields').value = selectedFieldsToSend;

        setHash(url);
    }

    /**
     * Show More/Fewer facets.
     */
    function searchFacetsMoreFewer(event, selector) {
        if (selector.parent().find('.apachesolr-hidden-facet:visible').length == 0) {
            selector.parent().find('.apachesolr-hidden-facet').removeClass('hidden');
            selector.text(Drupal.t('Show fewer'));
        }
        else {
            selector.parent().find('.apachesolr-hidden-facet').addClass('hidden');
            selector.text(Drupal.t('Show more'));
        }

        event.preventDefault();
        return false;
    }

    /**
     * Text field search callback.
     */
    function searchTextFieldChange(selector) {
        if (selector.val() != '') {
            var field = selector.parent().parent().children('[name="field"]').val();
            var url = selector.parent().parent().children('[name="url"]').val();
            var type = selector.parent().parent().children('[name="fieldtype"]').val();
            var from_str = field + '_placeholder';
            var to_str;

            if (type == 'length') {
                var option = selector.parent().parent().children('.form-item').children('[name="option"]').val();
                var value = parseInt(selector.parent().parent().children('.form-item').children('[name="value"]').val());

                if (option.indexOf('[* TO #]') > -1) {
                    value -= 1;
                }
                else {
                    if (option.indexOf('[# TO *]') > -1) {
                        value += 1;
                    }
                }

                query = option.replace('#', value);
                to_str = query;
                from_str = '%3A' + from_str;
                url = url.replace(from_str, to_str);

                setHash(url);
            }
            else {
                var negative_url = selector.parent().parent().children('[name="negative_url"]').val();
                var option = selector.parent().parent().children('.form-item').children('[name="option"]').val();
                var text = selector.parent().parent().children('.form-item').children('[name="value"]').val();
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

                setHash(url);
            }
        }
    }

    /**
     * Search on changing the range of numeric widget.
     */
    function searchNumericWidgetChangeRange(event, selector) {

        // Get values.
        var field = selector.closest('.tow-inner-search-widget').find('[name="field"]').val();
        var url = selector.closest('.tow-inner-search-widget').find('[name="url"]').val();
        var type = selector.closest('.tow-inner-search-widget').find('[name="fieldtype"]').val();
        var from_str = field + '_placeholder';
        var to_str;

        // Check the widget is of a numeric field.
        if (type == 'int' || type == 'float' || type == 'time' || type == 'date' || type == 'datetime' || type == 'timestamp') {

            // Get additional values.
            var min = selector.closest('.tow-inner-search-widget').find('[name="min"]').val();
            var max = selector.closest('.tow-inner-search-widget').find('[name="max"]').val();
            var glo_min = selector.closest('.tow-inner-search-widget').find('[name="global_min"]').val();
            var glo_max = selector.closest('.tow-inner-search-widget').find('[name="global_max"]').val();
            var sel_min = selector.closest('.tow-inner-search-widget').find('[name="selection_min"]').val();
            var sel_max = selector.closest('.tow-inner-search-widget').find('[name="selection_max"]').val();
            var check = selector.closest('.tow-inner-search-widget').find('.form-checkbox').attr('checked');
            var result = location.hash.search(field);

            // Check if the range was changed.
            if ((min == minCur && max == maxCur) || (sel_min == glo_min && sel_max == glo_max && result == -1) || (sel_min == glo_min && sel_max == glo_max && !check)) {

                // Return if not.
                return false;
            }

            // Replace old filters with new ones.
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

                // Change url.
                url = url.replace(from_str, to_str);

                // Set hash.
                setHash(url);
            }

            // Update current min & max.
            minCur = min;
            maxCur = max;
        }

        return false;
    }

    /**
     * Including/excluding empty values.
     */
    function searchIncludeEmptyValues(event, selector) {
        if (event.button == 0) {
            var empty = selector.closest('.tow-inner-search-widget').find('[name="include_empty_url"]').val();
            setHash(empty);
        }
    }



    /***** Highcharts callbacks *****/

    /**
     * Test basic chart on rebuilding.
     */
    function highchartsTestChart(selector) {
        var id = selector.attr('id');

        var datastring = selector.parent().children('div').children('[name="data"]').val();
        var namestring = selector.parent().children('div').children('[name="tooltips"]').val();
        var fieldtype = selector.parent().children('div').children('[name="fieldtype"]').val();
        var fieldname = selector.parent().children('div').children('[name="field"]').val();

        var min = selector.parent().find('[name="global_min"]').val();
        var max = selector.parent().find('[name="global_max"]').val();
        var selection_min = selector.parent().find('[name="selection_min"]').val();
        var selection_max = selector.parent().find('[name="selection_max"]').val();
        var yMax = selector.parent().find('[name="max_count"]').val();
        datastring = datastring.substring(4, datastring.length - 4);
        datastring = datastring.split(' ], [ ');
        namestring = namestring.substring(3, namestring.length - 3);
        namestring = namestring.split('", "');
        var data = [];
        for (var key in datastring) {
            var val = datastring[key];
            point = val.split(', ');
            data[key] = {
                x: parseFloat(point[0]),
                y: parseInt(point[1]),
                name: namestring[key]
            };
        }
        if (fieldtype != 'time' && fieldtype != 'date' && fieldtype != 'datetime' && fieldtype != 'timestamp') {
            axistype = 'linear';
        }
        else {
            axistype = 'datetime';
        }
        highchartsPlotChart(id, data, axistype, fieldname, fieldtype, parseFloat(min), parseFloat(max), parseFloat(yMax) * 1.1, parseFloat(selection_min), parseFloat(selection_max));
    }

    /**
     * Plot basic chart.
     */
    function highchartsPlotChart(id, data, axistype, fieldname, fieldtype, globalMin, globalMax, yMax, selectionMin, selectionMax) {
        var masterChart;
        var detailChart;
        var visible_min = globalMin;
        var visible_max = globalMax;
        var isMouseDown = false;
        var currentIndex;
        var lastMousePos;
        var lastPos;
        var currentHandle;
        var currentObj;
        var detailBorder;
        var ratio;
        var screenSelRange = {
            min: null,
            max: null
        };
        var plotSelRange = {
            min: null,
            max: null
        };
        var detailLeft;
        var detailRight;

        var $container = $('#' + id).addClass('pos-rel');
        var $detailContainer = $('<div id="' + id + '-detail" class="detail-container">').addClass('pos-abs t0h128w100').appendTo($container);
        var $masterContainer = $('<div id="' + id + '-master">').addClass('pos-abs t128h40w100').appendTo($container);

        var minorTickInterval;
        var tickInterval;

        if (yMax >= 6) {
            minorTickInterval = Math.floor(yMax/6);
            tickInterval = minorTickInterval * 2;
        }
        else {
            if (yMax >= 4) {
                minorTickInterval = 1;
                tickInterval = 2;
            }
            else {
                tickInterval = 1;
            }
        }

        // Create master and in its callback, create the detail chart.
        createMaster();

        $container.before('<div class="tow-highcharts-summary" id="' + id + '-summary"></div>');
        highchartsChartDisplaySummary();

        // Create the master chart callback.
        function createMaster() {

            /**
             * Create master cselection callback.
             */
            function selectionCallback(event, id, fieldtype, fieldname) {
                $('#' + id +'-waiting').removeClass('hidden');

                var extremesObject = event.xAxis[0];
                var min = extremesObject.min;
                var max = extremesObject.max;

                arrayOfZooms[id] = {
                    'min' : min,
                    'max' : max
                };
                $(window).css('cursor', 'wait');
                var urlZ = 'http://' + location.host + '/search_inner_zoom_ajax';
                var searchRid = $('#tow-search-inner-hash-form #edit-rid').val();
                var searchHash = $('#tow-search-inner-hash-form #edit-hash').val();

                $.ajax({
                    url: urlZ,
                    data: {
                        'op': 'zoom',
                        'rid': searchRid,
                        'hash': searchHash,
                        'visible_min': min,
                        'visible_max': max,
                        'field_type': fieldtype,
                        'field': fieldname
                    },
                    dataType: 'json',
                    success: function(data) {
                        $('#' + id +'-waiting').addClass('hidden');
                        detailChart.series[0].setData(data.data, false);
                        $(window).css('cursor', 'auto');
                        newVisibleRange(min, max);
                    }
                });

                newVisibleBox(min, max, false);

                event.preventDefault();
                return false;
            }

            /**
             * Master chart settings.
             */
            function masterChartSettings() {
                return {
                    chart: {
                        renderTo: id + '-master',
                        zoomType: 'x',
                        borderWidth: 0,
                        marginTop: 3,
                        marginLeft: 25,
                        marginRight: 10,
                        events: {
                            mousedown: function(e) {
                                isMouseDown = true;
                                currentObj = 'master';
                                lastMousePos = e.clientX;
                            },

                            // Listen to the selection event on the master chart to update the
                            // extremes of the detail chart.
                            selection: function(event) {
                                selectionCallback(event, id, fieldtype, fieldname);
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
                        endOnTick: false
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
                            color: '#474747',
                            fillColor: {
                                linearGradient: [0, 0, 0, 70],
                                stops: [
                                [0, '#525252'],
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
                };
            }

            /**
             * Master chart processor.
             */
            function masterChartProcessor(masterChart) {

                masterChart.xAxis[0].minRange = 5 * (globalMax - globalMin) / $('#' + masterChart.container.id).width();
                $('#' + masterChart.container.id).after('<div id="' + masterChart.container.id + '-visible"></div>');
                var yExtremes = masterChart.yAxis[0].getExtremes();
                var left = masterChart.xAxis[0].translate(globalMin, false) + masterChart.plotLeft;
                var width = masterChart.plotWidth;
                var top = masterChart.plotTop - visibleBoxBorder;
                var height = masterChart.plotHeight;
                $('#' + masterChart.container.id).dblclick(function(e) {
                    resetZoom();
                    arrayOfZooms[id] = undefined;
                });

                $('#' + masterChart.container.id + '-visible')
                .addClass('pos-abs bs-solid pe-none')
                .css('border-width', visibleBoxBorder)
                .css('left', left)
                .css('width', width)
                .css('top', top)
                .css('height', height)
                .addClass('hidden')
                .append('<div id="' + masterChart.container.id + '-visible-handle' + '">');

                $('<div>')
                .addClass('pos-abs bs-solid pe-none bc-grey')
                .css('border-width', visibleBoxBorder)
                .css('left', masterChart.plotLeft - 2)
                .css('width', masterChart.plotWidth + 3)
                .css('top', masterChart.plotTop - 2)
                .css('height', masterChart.plotHeight + 3)
                .appendTo('#' + masterChart.container.id);

                $('#' + masterChart.container.id + '-visible-handle')
                .addClass('pos-abs bs-solid bgc-white pe-vis cur-e')
                .css('border-width', visibleHandleBorder)
                .css('right', width - visibleHandleOffset)
                .css('width', visibleHandleWidth)
                .css('top', height - visibleHandleHeight / 2 - visibleHandleBorder)
                .css('height', visibleHandleHeight)
                .mousedown(function(e) {
                    isMouseDown = true;
                    currentObj = 'box';
                    lastMousePos = e.clientX;
                    var zero = masterChart.xAxis[0].translate(0, true);
                    var one = masterChart.xAxis[0].translate(1, true);
                    var extremes = detailChart.xAxis[0].getExtremes();
                    ratio = one - zero;
                    return false;
                });

                createDetail(masterChart);
            }
            masterChart = new Highcharts.Chart(masterChartSettings(), function(masterChart) {
                masterChartProcessor(masterChart);
            });
        }

        /**
         * Create the detail chart.
         */
        function createDetail(masterChart) {

            /**
             * Detail chart settings.
             */
            function detailChartSettings() {
                return {
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
                        plotBorderWidth: 1
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
                        maxPadding: 0.02
                    },
                    yAxis: {
                        title: {
                            text: null
                        },
                        min: 0,
                        max: yMax,
                        gridLineWidth: 0,
                        minorGridLineWidth: 0,
                        labels: {
                            x: -1,
                            y: 4
                        },
                        endOnTick: false,
                        minorTickInterval: minorTickInterval,
                        tickInterval: tickInterval
                    },
                    tooltip: {
                        formatter: function() {
                            tooltip = this.point.name;
                            tooltip = tooltip.replace(/\\\\n/g,'<br/>');
                            tooltip = tooltip.replace(/\\n/g,'<br/>');
                            return tooltip;
                        },
                        style: {
                            fontSize: '7pt',
                            zIndex: 11
                        }
                    },
                    legend: {
                        enabled: false
                    },
                    plotOptions: {
                        series: {
                            color: '#474747',
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
                        data: data
                    }]
                };
            }

            /**
             * Detail chart processor.
             */
            function detailChartProcessor(detailChart) {

                $('<div>')
                .attr('id', id +'-waiting')
                .addClass('pos-abs bgp-center z20 bgr-no')
                .css('left', detailChart.plotLeft)
                .css('top', detailChart.plotTop)
                .css('width', detailChart.plotWidth)
                .css('height', detailChart.plotHeight)
                .addClass('bgi-w')
                .css('background-color', 'rgba(255,255,255, 0.8)')
                .appendTo('#' + detailChart.container.id)
                .addClass('hidden');

                $('<div>')
                .attr('id', id +'-noresult')
                .addClass('pos-abs bgp-center z9 ta-center va-middle fs10')
                .css('left', detailChart.plotLeft)
                .css('top', detailChart.plotTop)
                .css('width', detailChart.plotWidth)
                .css('margin-top', detailChart.plotHeight / 2 - 10)
                .html('No results found')
                .appendTo('#' + detailChart.container.id);

                if (yMax) {
                    $('#' + id +'-noresult').addClass('hidden');
                }

                $('#' + detailChart.container.id).mousedown(function(e) {
                    isMouseDown = true;
                    currentObj = 'detail';
                    lastMousePos = e.clientX;
                    var zero = detailChart.xAxis[0].translate(0, true);
                    var one = detailChart.xAxis[0].translate(1, true);
                    var extremes = detailChart.xAxis[0].getExtremes();
                    ratio = one - zero;
                    visible_min = extremes.min;
                    visible_max = extremes.max;
                })
                .dblclick(function(e) {
                    resetZoom();
                    arrayOfZooms[id] = undefined;
                });
                customHighchartsNavigator(detailChart, masterChart);
            }

            // Create a detail chart referenced by a global variable.
            detailChart = new Highcharts.Chart(detailChartSettings(), function(detailChart) {
                detailChartProcessor(detailChart);
            });
        }

        /**
         * Moves handle defined by currentIndex to screen position screenX,
         * and also moves masks on both charts.
         */
        function moveHandle(screenX, same) {
            var handle_id = detailChart.container.id + '-handle-' + currentIndex;
            $('#' + handle_id).css('left', screenX - handleWidth / 2);
            plotX = detailChart.xAxis[0].translate(screenX - detailChart.plotLeft, true);
            changeMask(detailChart, currentIndex, plotX);
            changeMask(masterChart, currentIndex, plotX);
            if (currentIndex) {
                screenSelRange.max = screenX;
                if (same != true) {
                    plotSelRange.max = plotX;
                }
            }
            else {
                screenSelRange.min = screenX;
                if (same != true) {
                    plotSelRange.min = plotX;
                }
            }
            highchartsChartDisplaySummary();
            if (screenX < detailLeft - 1 || screenX > detailRight + 1) {
                $('#' + handle_id).addClass('hidden');
            }
            else {
                $('#' + handle_id).removeClass('hidden');
            }
        }

        /**
         * Returns new positionf of a handle.
         */
        function newHandlePos(newPos) {
            if (newPos > detailRight) {
                return detailRight;
            }
            if (newPos < detailLeft) {
                return detailLeft;
            }
            if (currentIndex) {
                if (newPos < detailBorder) {
                    moveHandle(detailBorder);
                    currentIndex = 0;
                    return newHandlePos(newPos);
                }
            }
            else {
                if (newPos > detailBorder) {
                    moveHandle(detailBorder);
                    currentIndex = 1;
                    return newHandlePos(newPos);
                }
            }
            return newPos;
        }

        /**
         * New visible range.
         */
        function newVisibleRange(min, max) {

            // Setting extremes on master.
            detailChart.xAxis[0].setExtremes(min, max, true, false);

            // Setting constants.
            visible_min = min;
            visible_max = max;

            detailLeft = detailChart.xAxis[0].translate(min, false) + detailChart.plotLeft;
            detailRight = detailChart.xAxis[0].translate(max, false) + detailChart.plotLeft;

            // Redraw handles.
            currentIndex = 0;
            moveHandle(detailChart.xAxis[0].translate(plotSelRange.min, false) + detailChart.plotLeft, true);
            currentIndex = 1;
            moveHandle(detailChart.xAxis[0].translate(plotSelRange.max, false) + detailChart.plotLeft, true);
        }

        /**
         * New visible box.
         */
        function newVisibleBox(min, max, redrawDetail) {
            var rangewidth = max - min;
            var globalRandewidth = globalMax - globalMin;
            if (min < globalMin) {
                min = globalMin;
                max = globalMin + rangewidth;
            }
            if (max > globalMax) {
                max = globalMax;
                min = globalMax - rangewidth;
            }

            // Move visible box on master.
            var left = masterChart.xAxis[0].translate(min, false) + masterChart.plotLeft;
            var width = masterChart.xAxis[0].translate(max, false) + masterChart.plotLeft - left;
            left = left - 2 * visibleHandleBorder;

            $('#' + masterChart.container.id + '-visible')
            .removeClass('hidden')
            .css('left', left)
            .css('width', width);

            $('#' + masterChart.container.id + '-visible-handle')
            .css('right', visibleHandleOffset);

            if (Math.abs(min - globalMin) <= globalRandewidth / 200 && Math.abs(max - globalMax) <= globalRandewidth / 200) {
                $('#' + masterChart.container.id + '-visible').addClass('hidden');
            }
            if (redrawDetail != false) {
                newVisibleRange(min, max);
            }
        }

        /**
         * Navigator.
         */
        function customHighchartsNavigator(detailChart, masterChart) {

            /**
             * Add mask.
             */
            function addMask(chart) {
                changeMask(chart, 0, selectionMin);
                changeMask(chart, 1, selectionMax);
            }

            /**
             * Draw handle.
             */
            function drawHandle(index, x) {
                var top = $('#' + detailChart.container.id).height();
                var handle_id = detailChart.container.id + '-handle-' + index;
                top = (top - handleHeight) / 2;
                $('#' + detailChart.container.id).after('<div id="' + handle_id + '">');
                $('#' + handle_id)
                .addClass('pos-abs bs-solid bgc-white z10 cur-e bw1')
                .css('top', top)
                .css('left', x - handleWidth/2)
                .css('height', handleHeight)
                .css('width', handleWidth)
                .mousedown(function(e) {
                    isMouseDown = true;
                    currentIndex = index;
                    lastMousePos = e.clientX;
                    currentObj = 'handle';
                    if (index) {
                        detailBorder = screenSelRange.min;
                        lastPos = screenSelRange.max;
                    }
                    else {
                        detailBorder = screenSelRange.max;
                        lastPos = screenSelRange.min;
                    }
                    return false;
                });
            }

            addMask(masterChart);
            addMask(detailChart);

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

            drawHandle(0, screenSelRange.min);
            drawHandle(1, screenSelRange.max);
        }

        /**
         * Change mask.
         */
        function changeMask(chart,index, x) {
            var maskId = 'mask-' + index;
            var from;
            var to;
            if (index) {
                from = x;
                to = globalMax;
            }
            else {
                from = globalMin;
                to = x;
            }
            chart.xAxis[0].removePlotBand(maskId);
            chart.xAxis[0].addPlotBand({
                id: maskId,
                from: from,
                to: to,
                color: 'rgba(127, 127, 127, 0.5)',
                zIndex: 10
            });
        }

        /**
         * Basic chart display summary.
         */
        function highchartsChartDisplaySummary() {
            var newMin = limitToDisplay(fieldtype, plotSelRange.min);
            var newMax = limitToDisplay(fieldtype, plotSelRange.max);
            var summary = '(from ' + newMin + ' to ' + newMax + ')';

            $('#' + id).siblings('div#' + id + '-summary"').html(summary);

            $('#' + id).parent().find('[name="selection_min"]').val(plotSelRange.min);
            $('#' + id).parent().find('[name="selection_max"]').val(plotSelRange.max);

            $('#' + id).parent().find('[name="min"]').val(newMin);
            $('#' + id).parent().find('[name="max"]').val(newMax);
        }

        /**
         * Reset zoom.
         */
        function resetZoom() {
            detailChart.series[0].setData(data, false);
            newVisibleBox(globalMin, globalMax);
        }

        // Preserving zoom after AJAX.
        arrayOfCharts[id] = {
            'master': masterChart,
            'detail' : detailChart
        };

        if (arrayOfZooms[id] != undefined) {
            createDetail(arrayOfCharts[id]['master']);
            newVisibleRange(arrayOfZooms[id]['min'], arrayOfZooms[id]['max']);
            newVisibleBox(arrayOfZooms[id]['min'], arrayOfZooms[id]['max'], false);
            customHighchartsNavigator(arrayOfCharts[id]['detail'], arrayOfCharts[id]['master']);
        }

        // Mouse down event.
        $(document).mousemove(function(e) {
            if (isMouseDown) {
                switch (currentObj) {
                    case 'handle':
                        var span = e.clientX - lastMousePos;
                        var newPos = lastPos + span;
                        correctedPos = newHandlePos(newPos);
                        moveHandle(correctedPos);
                        break;
                    case 'box':
                        var span = e.clientX - lastMousePos;
                        var min = visible_min + ratio * span;
                        var max = visible_max + ratio * span;
                        lastMousePos = e.clientX;
                        newVisibleBox(min, max);
                        arrayOfZooms[id] = {
                            'min' : min,
                            'max' : max
                        };
                        return false;
                    case 'detail':
                        var span = e.clientX - lastMousePos;
                        var min = visible_min - ratio * span;
                        var max = visible_max - ratio * span;
                        lastMousePos = e.clientX;
                        newVisibleBox(min, max);
                        arrayOfZooms[id] = {
                            'min' : min,
                            'max' : max
                        };
                        break;
                    case 'master':
                        break;
                    default:
                }
            }
            return false;
        });

        // Mouse up event.
        $(document).mouseup(function(e) {
            isMouseDown = false;
        });
    }

    /**
     * Test bar chart on rebuilding.
     */
    function highchartsTestBarChart(selector) {

        var id = selector.attr('id');
        var datastring = selector.parent().children('form').children('div').children('[name="data"]').val();
        var namestring = selector.parent().children('form').children('div').children('[name="tooltips"]').val();
        var fieldtype = selector.parent().children('form').children('div').children('[name="fieldtype"]').val();
        var fieldname = selector.parent().children('form').children('div').children('[name="field"]').val();
        var selection_min = selector.parent().find('[name="selection_min"]').val();
        var selection_max = selector.parent().find('[name="selection_max"]').val();
        var global_min = selector.parent().find('[name="global_min"]').val();
        var global_max = selector.parent().find('[name="global_max"]').val();

        var active = true;

        if (global_min == selection_min && global_max == selection_max) {
            active = false;
        }

        datastring = datastring.substring(2, datastring.length - 2);
        datastring = datastring.split(', ');
        namestring = namestring.substring(3, namestring.length - 3);
        var names = namestring.split('", "');
        var data = [];
        var selected = [];
        if (fieldtype != 'time' && fieldtype != 'date' && fieldtype != 'datetime' && fieldtype != 'timestamp') {
            axistype = 'linear';
            selection_min = parseFloat(selection_min);
            selection_max = parseFloat(selection_max);
        }
        else {
            axistype = 'datetime';
        }
        for (var key in datastring) {
            var x;
            if (axistype == 'linear') {
                x = parseFloat(names[key]);
            }
            else {
                x = names[key];
            }
            if (active) {
                selected[key] = (selection_min <= x && selection_max >= x);
            }
            else {
                selected[key] = false;
            }
            data[key] = parseInt(datastring[key]);
        }
        chart = highchartsPlotBarChart(id, names, data, selected, axistype, fieldname, fieldtype, parseFloat(selection_min), parseFloat(selection_max));
    }

    /**
     * Plot bar chart.
     */
    function highchartsPlotBarChart(id, categories, data, selected, axistype, fieldname, fieldtype, selectionMin, selectionMax) {

        /*** Functions ***/

        /**
         * Display bar chart summary.
         */
        function highchartsBarChartDisplaySummary() {
            var min = $('#' + id).parent().find('[name="selection_min"]').val(),
            max = $('#' + id).parent().find('[name="selection_max"]').val(),
            summary = '(from ' + min + ' to ' + max + ')';
            $('#' + id).siblings('div#' + id + '-summary"').html(summary);
        }

        /*** Actions ***/

        var container = $('#' + id);
        container.before('<div class="tow-highcharts-summary" id="' + id + '-summary"></div>');
        highchartsBarChartDisplaySummary();

        return new Highcharts.Chart({
            chart: {
                renderTo: id,
                type: 'column',
                spacingLeft: 0,
                spacingRight: 0
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
                            select: function(e) {
                                var current_key = this.x;
                            }
                        }
                    }
                }
            },
            xAxis: {
                title: {
                    text: null
                },
                categories: categories,
                labels: {}
            },
            yAxis: {
                min: 0,
                title: {
                    text: null
                },
                labels: {
                    x: 0,
                    y: -2
                }
            },
            tooltip: {
                formatter: function() {
                    var x = this.x;
                    var y = this.y;
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
        },
        function(barChart) {
            var points = barChart.series[0].data;
            for (var key in points) {
                var point = points[key];
                if (selected[key]) {
                    point.select(true, true);
                }
            }
        });
    }



    /* ****************************************************************************************************
     * FUNCTIONS
     * ****************************************************************************************************/

    /***** AJAX *****/

    /**
     * Inner search processing.
     */
    function innerSearchProcessing(filtersToSend, selectedFieldsToSend) {

        var urlISA = 'http://' + window.location.hostname + window.location.pathname + '/refresh_ajax' + window.location.hash;

        // Disabling other widget choice while AJAX proceed.
        $('#tow-search-inner-hash-form').after('<div id="modalDiv" style="position:absolute;top:0px;left:0px;display:none;cursor:progress;z-index:100;"></div>');

        document.getElementById("modalDiv").style.height = "100%";
        document.getElementById("modalDiv").style.width = "100%";
        document.getElementById("modalDiv").style.display = "block";

        var zoomsToSend = JSON.stringify(arrayOfZooms);

        // Preserving user-selected collapsibility.
        function preserveCollapsibility() {

            $('.tow-inner-search-widget fieldset').each(function() {
                var field = $(this).find('[name="field"]').val();
                var collapsed;

                if ($(this).hasClass('collapsed')) {
                    collapsed = true;
                }
                else {
                    collapsed = false;
                }

                arrayOfCollapse[field] = {
                    'collapsed': collapsed
                };
            });
        }

        function updateOnInnerSearchSuccess(data) {

            $('div#block-tow-search_inner_facets div.content').html(data.widgets);
            $('div#block-tow-search_inner_field_list div.content').html(data.fields);
            if (data.save_search !== null) {
                $('div#block-tow-saved_searches_description div.content').html(data.save_search);
            }
            if (data.save_this_search !== null) {
                $('div#block-tow-saved_searches_save_search div.content').html(data.save_this_search);
            }

            // Returns collapsibility.
            Drupal.behaviors.collapse();

            // Returning user-selected collapsibility.
            $('.tow-inner-search-widget fieldset').each(function() {
                var field = $(this).find('[name="field"]').val();
                if ((typeof arrayOfCollapse[field] != "undefined") && (arrayOfCollapse[field]['collapsed'] == true)) {
                    $(this).addClass('collapsed');
                }
            });

            initialArrayOfWidgets = $('.tow-inner-search-widget').get();
            $('.apachesolr-hidden-facet', context).addClass('hidden');

            $('<a href="#" class="apachesolr-showhide"></a>')
            .text(Drupal.t('Show more'))
            .click(function(e) {
                searchFacetsMoreFewer(e, $(this));
            })
            .appendTo($('.item-list:has(.apachesolr-hidden-facet)', context));

            if (sortCurrentTitleDirection != '') {
                sortWidgets('title', sortCurrentTitleDirection);
            }
            if (sortCurrentTypeDirection != '') {
                sortWidgets('type', sortCurrentTypeDirection);
            }

            $('a.apachesolr-facet, a.apachesolr-hidden-facet, .tow-inner-search-selected').live('click', function(e) {
                searchFacetClickUpdate(e, $(this));
            });
            $('div.tow-inner-search-highcharts-container').each(function(index) {
                highchartsTestChart($(this));
            });

            $('div.content-content').html(data.search);

            //Wrap every three widgets
            var divs = $("div.tow-inner-search-widget");
            for(var i = 0; i < divs.length; i+=3) {
                divs.slice(i, i+3).wrapAll("<div class='three-in-one'></div>");
            }

            // Number of rows in searchtable.
            var numberOfRows = $('#datatable-1 tbody tr').size();
            $('#tow-search-inner-save-search-form').children('div').children('[name="rows_amount"]').val(numberOfRows);

            var oTable = $('#datatable-1').dataTable({
                "sDom": 'C<"clear">rti',
                "oColVis": {
                    "fnLabel": function (index, title, th) {
                        return (index + 1) + '. ' + title;
                    }
                },
                "sScrollY": "",
                "bStateSave": true,
                "bScrollCollapse": true,
                "bPaginate": false,
                "bFilter": true,
                "bSort": true,
                "bInfo": false,
                "bRetrieve": true,
                "sScrollX": "100%",
                "sScrollXInner": "100%",
                "sWidth": ''
            });

            $(".dataTables_scroll").jScrollPane({
                showArrows: true
            }); /* This is a scroller implementation */
            var jsp_element = $(".dataTables_scroll").jScrollPane({
                showArrows: true
            });
            var jsp_api = jsp_element.data('jsp');
            var scrollableX = jsp_api.getIsScrollableH();
            
            $("#block-tow-search_inner_field_list").addClass("tab-pane active");
            $("#block-tow-search_inner_facets").addClass("tab-pane");

            var access = $(".dataTables_scrollHead");

            $('#InnerSearchTab').click(function() {
                setTimeout(function () {
                    pos = access.offset();
                }, 200);
            });
            var pos = access.offset();
            $(window).scroll(function() {

                if (pos != null && $(this).scrollTop() > pos.top - $(window).height() + $(".dataTables_scrollHead").height() + 25) {
                    $(".jspHorizontalBar").addClass('fixed_jspHorizontalBar');

                } else {
                    $(".jspHorizontalBar").removeClass('fixed_jspHorizontalBar');
                }

                if (pos != null && $(this).scrollTop() > pos.top) {
                    access.addClass('fixed_head');
                    access.stop().animate({
                        marginTop: $(window).scrollTop() - pos.top
                    },0);

                } else {
                    access.removeClass('fixed_head');
                }
            });
            /* Here is a function, that allows a horizontal scrolling via mouse scroll (+ jquery.mousewheel.min.js */

            if (scrollableX) {
                $(".dataTables_scrollBody").mousewheel(function(event, delta) {
                    var delta = delta * 30;
                    var test = parseInt($('.jspPane').css('left'), 10);
                    var lefttable = test + delta + "px";
                    $(".jspPane").css('left',lefttable);
                    event.preventDefault();
                });
            }

        }

        $.ajax({
            url: urlISA,
            data: {
                'filters' : filtersToSend,
                'zoom' : zoomsToSend,
                'selected_fields' : selectedFieldsToSend
            },
            beforeSend: function() {
                preserveCollapsibility();
            },
            success: function(data) {
                updateOnInnerSearchSuccess(data);
            },
            dataType: 'json'
        });
    }

    /**
     * Saving a search.
     */
    function saveSearch(event) {
        var urlSS = 'http://' + window.location.hostname + window.location.pathname + '/ajax/save_search';
        var filters = $('#edit-filters').val();
        var selectedFields = $('#edit-selected-fields').val();
        var rowsAmount = $('#edit-rows-amount').val();
        var sSComment = $('#edit-ss-comment').val();

        /**
         * Saved search creation.
         */
        function saveSearchAjaxSuccess(data) {
            if (!data.saved) {
                alert(data.message);
            }
            else {
                $('#block-tow-saved_searches_list div.content').html(data.saved_searches);

                //Returns voting AJAX
                Drupal.behaviors.CToolsAJAX();

                $.hrd.noty({
                    'type' : 'success',
                    'text' : 'You posted a search'
                });
            }
        }

        // AJAX.
        $.ajax({
            url: urlSS,
            data: {
                'filters' : filters,
                'rows_amount' : rowsAmount,
                'ss_comment' : sSComment,
                'selected_fields' : selectedFields
            },
            success: function(data) {
                saveSearchAjaxSuccess(data);
            },
            dataType: 'json'
        });

        event.preventDefault();
        return false;
    }

    /**
     * Deleting a saved search.
     */
    function deleteSavedSearch(event, selector) {
        var urlSSD = 'http://' + window.location.hostname + window.location.pathname + '/ajax/delete_search';
        var confirmToSend = window.confirm('Are you sure you want to delete ' + selector.parent().children().find('.title').children().attr('title') + '? This action cannot be undone.');
        var ssNidToSend = selector.attr('href').split('/')[2];

        /**
         * Saved search deletion.
         */
        function deleteSearchAjaxSuccess(data) {
            $('#block-tow-saved_searches_list div.content').html(data.saved_searches);

            //Returns voting AJAX
            Drupal.behaviors.CToolsAJAX();

            $.hrd.noty({
                'type' : 'success',
                'text' : 'You have deleted a saved search'
            });
        }

        // AJAX.
        $.ajax({
            url: urlSSD,
            data: {
                'confirm' : confirmToSend,
                'ss_nid' : ssNidToSend
            },
            success: function(data) {
                deleteSearchAjaxSuccess(data);
            },
            dataType: 'json'
        });

        event.preventDefault();
        return false;
    }

    /**
     * Delete comment with AJAX.
     */
    function commentDelete(event, selector) {
        var urlCD = 'http://' + window.location.hostname + '/ajax/comment/delete';
        var confirmToSend = window.confirm('Are you sure you want to delete this comment? This action cannot be undone.');

        var cid = selector.attr('href').split('/')[3];

        var dataset = window.location.pathname.split('/')[2];

        /**
         * Comment deleting.
         */
        function deleteCommentSuccess(data) {
            $('#block-tow-saved_searches_list div.content').html(data.saved_searches);

            //Returns voting AJAX
            Drupal.behaviors.CToolsAJAX();
        }

        // AJAX.
        $.ajax({
            url: urlCD,
            data: {
                'cid' : cid,
                'dataset_nid' : dataset,
                'confirm' : confirmToSend
            },
            success: function(data) {
                deleteCommentSuccess(data);
            },
            dataType: 'json'
        });

        event.preventDefault();
        return false;
    }

    /**
     * Comment form with AJAX.
     */
    function commentForm(event, selector, op) {
        var urlCF = 'http://' + window.location.hostname + '/ajax/comment/form';

        var nid, cid, pid;
        if (op == 'edit') {
            cid = selector.attr('href').split('/')[3];
            nid = '';
            pid = '';
        }
        if (op == 'reply') {
            cid = '';
            nid = selector.attr('href').split('/')[3];
            pid = selector.attr('href').split('/')[4];
        }

        /**
         * Saved search creation.
         */
        function commentFormSuccess(data) {
            selector.parent().parent().parent().html(data.form);
        }

        // AJAX.
        $.ajax({
            url: urlCF,
            data: {
                'cid' : cid,
                'pid' : pid,
                'nid' : nid,
                'op' : op
            },
            success: function(data) {
                commentFormSuccess(data);
            },
            dataType: 'json'
        });

        event.preventDefault();
        return false;
    }

    /**
     * Save comment with AJAX.
     */
    function commentSave(event, selector) {
        var urlSC = 'http://' + window.location.hostname + '/ajax/comment/save';

        var nid, cid, pid, formBuildId, formToken, subject, action;
        formBuildId = selector.parent().parent().find('[name="form_build_id"]').val();
        formToken = selector.parent().parent().find('[name="form_token"]').val();

        action = selector.parent().parent().attr('action').split('/');

        if (action[2] == 'reply') {
            cid = '';
            nid = action[3];
            pid = (action[4]) ? action[4] : '';
        }
        if (action[2] == 'edit') {
            cid = action[3];
            nid = '';
            pid = '';
        }

        subject = selector.parent().parent().find('[name="comment"]').val();

        var dataset = window.location.pathname.split('/')[2];

        /**
         * Comment saving.
         */
        function saveCommentSuccess(data) {
            $('#block-tow-saved_searches_list div.content').html(data.saved_searches);

            //Returns voting AJAX
            Drupal.behaviors.CToolsAJAX();
        }

        // AJAX.
        $.ajax({
            url: urlSC,
            data: {
                'subject' : subject,
                'cid' : cid,
                'pid' : pid,
                'nid' : nid,
                'form_build_id' : formBuildId,
                'form_token' : formToken,
                'dataset_nid' : dataset
            },
            success: function(data) {
                saveCommentSuccess(data);
            },
            dataType: 'json'
        });

        event.preventDefault();
        return false;
    }

    /**
     * Widgets sort according to sort type and sort direction.
     */
    function sortWidgets(type, direction) {

        // Update current direction.
        if (type == 'title') {
            sortCurrentTitleDirection = direction;
            sortCurrentTypeDirection = '';
        }
        else {
            sortCurrentTitleDirection = '';
            sortCurrentTypeDirection = direction;
        }

        // Case title sorting.
        if (type == 'title') {

            // Set link text.
            $('.tow-inner-search-widget-sort a[href*="type"]').text('type');
            $('.tow-inner-search-widget-sort a[href*="title"]').text('title (' + direction + ')');

            // Get widgets.
            var sortArray = $('.tow-inner-search-widget').get();

            // Sort widgets.
            sortArray.sort(function(a, b) {
                var keyA = $(a).text().toLowerCase();
                var keyB = $(b).text().toLowerCase();

                // Ascending.
                if (direction == 'asc') {
                    if (keyA > keyB) {
                        return -1;
                    }
                    if (keyA < keyB) {
                        return 1;
                    }
                    return 0;
                }

                // Descending.
                else {
                    if (keyA < keyB) {
                        return -1;
                    }
                    if (keyA > keyB) {
                        return 1;
                    }
                    return 0;
                }
            });
        }

        // Case type sorting.
        if (type == 'type') {

            // Set link text.
            $('.tow-inner-search-widget-sort a[href*="title"]').text('title');
            $('.tow-inner-search-widget-sort a[href*="type"]').text('type (' + direction + ')');

            // Get widgets.
            var sortArray = $('.tow-inner-search-widget').get();

            // Sort widgets.
            sortArray.sort(function(a,b) {

                // Get sorting params.
                var keyA = $(a).find('input[value="text"], input[value="char"], input[value="bool"], input[value="code"], input[value="enum"], input[value="date"], input[value="datetime"], input[value="float"], input[value="int"], input[value="table"], input[value="time"], input[value="timestamp"]').attr('value');
                var keyB = $(b).find('input[value="text"], input[value="char"], input[value="bool"], input[value="code"], input[value="enum"], input[value="date"], input[value="datetime"], input[value="float"], input[value="int"], input[value="table"], input[value="time"], input[value="timestamp"]').attr('value');

                // Ascending.
                if (direction == 'asc') {
                    if (keyA > keyB) {
                        return -1;
                    }
                    if (keyA < keyB) {
                        return 1;
                    }
                    return 0;
                }

                // Descending.
                else {
                    if (keyA < keyB) {
                        return -1;
                    }
                    if (keyA > keyB) {
                        return 1;
                    }
                    return 0;
                }
            });

        }

        // Output sorted widgets.
        $.each(sortArray, function(i, form) {
            $('#tow-search-inner-hash-form').after(form);
        });
    }

    /**
     * Set hash.
     */
    function setHash(url) {
        filtersToSend = getUrlQueryParam(url, 'filters');
        selectedFieldsToSend = getUrlQueryParam(url, 'selected_fields');
        if ((filtersToSend.length == 0) && (selectedFieldsToSend.length == 0)) {
            hash = '';
        }
        else {
            hash = '?' + 'filters=' + filtersToSend + '&' + 'selected_fields=' + selectedFieldsToSend;
            if (filtersToSend.length == 0) {
                hash = '?' + 'selected_fields=' + selectedFieldsToSend;
            }
            if (selectedFieldsToSend.length == 0) {
                hash = '?' + 'filters=' + filtersToSend;
            }
        }
        location.hash = hash;
    }

    /**
     * SS tags
     */
    function ssTagsAutocomplete(event, selector) {
        var urlSSTags = 'http://' + window.location.hostname + '/tags';
        var ssTag = selector.val();
        var iOLC = ssTag.lastIndexOf(',');
        if (iOLC != -1) {
            var textAfterLastComma = ssTag.substring(iOLC + 1);
            ssTag = textAfterLastComma.replace(/^\s+/,'');
        }

        if (ssTag == ''/* || event.keyCode == 8*/) {
            $('.ss-tags-html').html('');
            return false;
        }
        $.ajax({
            url: urlSSTags,
            data: {
                'tag' : ssTag
            },
            success: function(data) {
                var htmlToInsert = $(data).find('div.view-content');
                $('.ss-tags-html').html(htmlToInsert);
            }
        });
        return false;
    }

}

/**
 * Converts numeric limits to displayable format.
 */
function limitToDisplay(fieldtype, value) {
    switch (fieldtype) {
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

/**
 * Returns datetime string representation of the timestamp given.
 */
function datetime(timestamp) {
    var date = new Date(timestamp);
    var hours = ('0' + date.getUTCHours()).slice(-2);
    var minutes = ('0' + date.getUTCMinutes()).slice(-2);
    var seconds = ('0' + date.getUTCSeconds()).slice(-2);
    var day = ('0' + date.getUTCDate()).slice(-2);
    var month = ('0' + (date.getUTCMonth() + 1)).slice(-2);
    var year = date.getUTCFullYear();
    return year + '-' + month + '-' + day + ' ' + hours + ':' + minutes + ':' + seconds;
}

/**
 * Returns date string representation of the timestamp given.
 */
function date(timestamp) {
    var date = new Date(timestamp);
    var day = ('0' + date.getUTCDate()).slice(-2);
    var month = ('0' + (date.getUTCMonth() + 1)).slice(-2);
    var year = date.getUTCFullYear();
    return year + '-' + month + '-' + day;
}

/**
 * Returns time string representation of the timestamp given.
 */
function time(timestamp) {
    var date = new Date(timestamp);
    var hours = ('0' + date.getUTCHours()).slice(-2);
    var minutes = ('0' + date.getUTCMinutes()).slice(-2);
    var seconds = ('0' + date.getUTCSeconds()).slice(-2);
    return hours + ':' + minutes + ':' + seconds;
}

/**
 * Get all url's query parametres.
 */
function getUrlQueryParams(url) {
    var vars = [], hash;
    var hashes = url.slice(url.indexOf('?') + 1).split('&');
    for(var i = 0; i < hashes.length; i++) {
        hash = hashes[i].split('=');
        vars[hash[0]] = decodeURIComponent(hash[1]);
    }
    return vars;
}

/**
 * Get url's query parametre.
 */
function getUrlQueryParam(url, arg) {
    var result;
    var params = getUrlQueryParams(url);

    if (!params[arg]) {
        result = '';
    }
    else {
        result = params[arg];
    }

    return result;
}
