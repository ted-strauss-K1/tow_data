Drupal.behaviors.outer_search = function(context) {
    var default_tab = '#newest';
    
    //Add class to 'Data' in primary menu
    $('#block-menu-primary-links ul.menu li.first a').addClass('active');
    
    // Hide hidden facets.
    $('.apachesolr-hidden-facet', context).addClass('hidden');

    // Add Show more/fewer link.
    $('<a href="#" class="apachesolr-showhide"></a>').text(Drupal.t('Show more')).click(function() {
        if ($(this).parent().find('.apachesolr-hidden-facet:visible').length == 0) {
            $(this).parent().find('.apachesolr-hidden-facet').removeClass('hidden');
            $(this).text(Drupal.t('Show fewer'));
        }
        else {
            $(this).parent().find('.apachesolr-hidden-facet').addClass('hidden');
            $(this).text(Drupal.t('Show more'));
        }
        return false;
    }).appendTo($('.item-list:has(.apachesolr-hidden-facet)', context));

    // Run search on hash change (links). Uses hashchangeevent plugin.
    $(window).hashchange(function() {
        hash = location.hash;
        if (!hash) {
            hash = default_tab;
            location.hash = hash;
        }
        if (!do_not_search) {
            url = 'http://' + location.host + '/search_outer_ajax/' + hash;
            do_search(url, hash);
        }
        do_not_search = false;
    });

    // Run search on changing search form.
    $('#edit-keywords, #edit-size, #edit-saved-searches, #edit-access-status, #edit-forum, #edit-docs, #edit-categories').change(function() {
        build_query_and_search();
    });

    // Disabling page reload on submit.
    $('#tow-search-outer-filter-form #edit-submit').live('click', function(e) {
        if (e.button == 0) {
            e.preventDefault();
            $(this).attr('disabled','true');
            build_query_and_search();
        }
    });

    // Building search query from form.
    var build_query_and_search = function () {
        path = '/' + $('#edit-keywords').val();
        query = 'filters=';

        // Subjects.
        // Each consequent search option should be separated from previous by whitespace.
        if ($('#edit-size').val() !== '0') {
            query += ' ' + 'sis_tow_amount_records:[' + $('#edit-size').val() + ' TO *]';
        }

        if ($('#edit-saved-searches').val() !== '0') {
            query += ' ' + 'sis_tow_amount_saved_searches:[' + $('#edit-saved-searches').val() + ' TO *]';
        }

        if ($('#edit-access-status').val() !== 'any') {
            query += ' ' + 'sis_cck_field_access_type:' + $('#edit-access-status').val();
        }

        if ($('#edit-categories').val() !== 'any') {
            query += ' ' + 'tid:' + $('#edit-categories').val();
        }

        if ($('#edit-tags-query').val()) {
            query += $('#edit-tags-query').val();
        }

        if (query == 'filters=') {
            query = '';
        }

        tab = $('#edit-current-tab').val();

        hash = tab + path + (query ? '?' + query : '');

        location.hash = hash;
    }

    // Search request and associated actions.
    var do_search = function (url, hash) {
        $('#content-content').html($('#edit-search-placeholder').val());
        url = url.replace('#', '');
        $.ajax({
            url: url,
            success: function(data) {
                $('#content-content').html(data.results);
                $('#block-tow-search_outer_filter > .inner > .content').html(data.form);
                $('#block-tow-search_outer_number_of_results > .inner > .content').html(data.number);
                $('#block-tow-search_outer_tabs > .inner > .content').html(data.tabs);

                $('#edit-keywords, #edit-size, #edit-saved-searches, #edit-access-status, #edit-forum, #edit-docs, #edit-categories').change(function() {
                    build_query_and_search();
                });

                $('.apachesolr-hidden-facet', context).addClass('hidden');

                $('<a href="#" class="apachesolr-showhide"></a>').text(Drupal.t('Show more')).click(function() {
                    if ($(this).parent().find('.apachesolr-hidden-facet:visible').length == 0) {
                        $(this).parent().find('.apachesolr-hidden-facet').removeClass('hidden');
                        $(this).text(Drupal.t('Show fewer'));
                    }
                    else {
                        $(this).parent().find('.apachesolr-hidden-facet').addClass('hidden');
                        $(this).text(Drupal.t('Show more'));
                    }
                    return false;
                }).appendTo($('.item-list:has(.apachesolr-hidden-facet)', context));

                $('a.active, a.apachesolr-facet, a.apachesolr-hidden-facet, a.apachesolr-unclick, a.tow-search-outer-tab').each(function() {
                    var href = $(this).attr('href');
                    href = '/#' + href.substring(1);
                    $(this).attr('href', href);
                });
            },
            dataType: 'json'
        });
    }

    var links_to_hash = function () {
        $('a.active, a.apachesolr-facet, a.apachesolr-hidden-facet, a.apachesolr-unclick, a.tow-search-outer-tab').each(function() {
            var href = $(this).attr('href');
            href = '/#' + href.substring(1);
            $(this).attr('href', href);
        });
    }

    var setFavicon = function () {
        var link = $('link[type="image/x-icon"]').remove().attr("href");
        $('<link href="'+ link +'" rel="shortcut icon" type="image/x-icon" />').appendTo('head');
    }

    // Trigger the first search.
    do_not_search = false;
    if (location.hash !== '' && location.hash !== default_tab && location.hash !== default_tab + '/') {
        $(window).hashchange();
    }
    else {
        do_not_search = true;
        location.hash = default_tab;
        links_to_hash();
        setFavicon();
    }
}