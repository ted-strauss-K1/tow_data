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

	//	disabling page reload on submit
	$('.form-submit').live('click', function(e) {
		if (e.button == 0) {
			e.preventDefault();
			var field = $(this).parent().children('[name="field"]').val();
			var url = $(this).parent().children('[name="url"]').val();
			var type = $(this).parent().children('[name="fieldtype"]').val();
			var min = $(this).parent().children('.form-item').children('[name="min"]').val();
			var max = $(this).parent().children('.form-item').children('[name="max"]').val();
			var from_str = field + ':' + field + '_placeholder';
			var to_str;
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
		}
	});
}