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

//	changing of search form
	$('#block-tow-search_outer_filter').change(function(){
		var url = build_query();
		do_search(url);
	});
//	disabling page reload on submit
	$('#edit-submit').live('click', function(e) {
		e.preventDefault();
		$(this).attr('disabled','true');
		var url = build_query();
		do_search(url);
	});
	
//	disabling tag links
	$('.apachesolr-facet, .apachesolr-unclick, .active').live('click', function(e) {
		e.preventDefault();
		var url = 'http://' + location.host + '/search_outer_ajax' + $(this).attr('href');
		do_search(url);
	});	
	
//	building search query from form
	var build_query = function () {
		var path = '/' + $('#edit-keywords').val();
		var query = 'filters=';

		// subjects
		// each consequent search option should be separated from previous by whitespace
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
		
		return 'http://' + location.host + '/search_outer_ajax/newest' + path + '?' + query;
	
	}
	
//	search and associated actions
	var do_search = function (url) {
		$('#content-content').html($('#edit-search-placeholder').val());
		$.ajax({
			url: url,
			success: function(data) {
				$('#content-content').html(data.results);
				$('#tow-search-outer-filter-form').html(data.form);
				$('#block-tow-search_outer_number_of_results').html(data.number);
			},
			dataType: 'json'
		});	  
	}
}