/**
 * Leaves displayed only those operators and input fields that are applicable
 * for specific field type
 * 
 * @param type
 *            type of the field
 */
function tow_display_filter_operator(type) {
	$(
			'#edit-filters-filter-items-wrapper-filter-items-operator option:[value*="' + type + '"]')
			.show();
	$(
			'#edit-filters-filter-items-wrapper-filter-items-operator option:not([value*="' + type + '"])')
			.hide();

	operator_value = $(
			'#edit-filters-filter-items-wrapper-filter-items-operator option:[value*="' + type + '"]')
			.val();

	$('#edit-filters-filter-items-wrapper-filter-items-operator').val(
			operator_value);
	// show corresponding input field depending on operator value
	tow_display_input_fields_for_operator($(
			'#edit-filters-filter-items-wrapper-filter-items-operator').val());
}

/**
 * Leaves displayed only those input elements that are applicable for specific
 * operator
 * 
 * @param operator_value
 */
function tow_display_input_fields_for_operator(operator_value) {
	// display input field if required
	if (operator_value.match("text") || operator_value.match("char")
			|| operator_value.match("float") || operator_value.match("int")) {
		// display only one input field,
		$('#edit-filters-filter-items-wrapper-filter-items-value-1-wrapper')
				.show();
		// hide all date and enum fields
		if (operator_value.match("int-between")) {
			$('#edit-filters-filter-items-wrapper-filter-items-value-2-wrapper')
					.show();
		} else {
			$('#edit-filters-filter-items-wrapper-filter-items-value-2-wrapper')
					.hide();
		}
		$('#edit-filters-filter-items-wrapper-filter-items-date-within-wrapper')
				.hide();
		$(
				'#edit-filters-filter-items-wrapper-filter-items-date-value-1-wrapper')
				.hide();
		$(
				'#edit-filters-filter-items-wrapper-filter-items-date-value-1-day-wrapper')
				.hide();
		$(
				'#edit-filters-filter-items-wrapper-filter-items-date-value-2-wrapper')
				.hide();
		$(
				'#edit-filters-filter-items-wrapper-filter-items-date-value-2-day-wrapper')
				.hide();
		$('#edit-filters-filter-items-wrapper-filter-items-enum-list-wrapper')
				.hide();

	} else if (operator_value.match("date")) {
		$('#edit-filters-filter-items-wrapper-filter-items-value-1-wrapper')
				.hide();
		$('#edit-filters-filter-items-wrapper-filter-items-value-2-wrapper')
				.hide();
		if (operator_value.match("date-within")) {
			$(
					'#edit-filters-filter-items-wrapper-filter-items-date-within-wrapper')
					.show();
		} else {
			$(
					'#edit-filters-filter-items-wrapper-filter-items-date-within-wrapper')
					.hide();
		}
		$(
				'#edit-filters-filter-items-wrapper-filter-items-date-value-1-wrapper')
				.show();
		$(
				'#edit-filters-filter-items-wrapper-filter-items-date-value-1-day-wrapper')
				.show();
		if (operator_value.match("date-between")) {
			$(
					'#edit-filters-filter-items-wrapper-filter-items-date-value-2-wrapper')
					.show();
			$(
					'#edit-filters-filter-items-wrapper-filter-items-date-value-2-day-wrapper')
					.show();
		} else {
			$(
					'#edit-filters-filter-items-wrapper-filter-items-date-value-2-wrapper')
					.hide();
			$(
					'#edit-filters-filter-items-wrapper-filter-items-date-value-2-day-wrapper')
					.hide();
		}

		$('#edit-filters-filter-items-wrapper-filter-items-enum-list-wrapper')
				.hide();
	} else if (operator_value.match("enum")) {

		$('#edit-filters-filter-items-wrapper-filter-items-value-1-wrapper')
				.hide();
		$('#edit-filters-filter-items-wrapper-filter-items-value-2-wrapper')
				.hide();
		$('#edit-filters-filter-items-wrapper-filter-items-date-within-wrapper')
				.hide();
		$(
				'#edit-filters-filter-items-wrapper-filter-items-date-value-1-wrapper')
				.hide();
		$(
				'#edit-filters-filter-items-wrapper-filter-items-date-value-2-wrapper')
				.hide();
		$(
				'#edit-filters-filter-items-wrapper-filter-items-date-value-1-day-wrapper')
				.hide();
		$(
				'#edit-filters-filter-items-wrapper-filter-items-date-value-2-day-wrapper')
				.hide();
		$('#edit-filters-filter-items-wrapper-filter-items-enum-list-wrapper')
				.show();
		if (operator_value.match("enum-anyof")) {
			// alert('match');
			$('#edit-filters-filter-items-wrapper-filter-items-enum-list')
					.attr('multiple', 'multiple');
		} else {
			// alert('no match');
			$('#edit-filters-filter-items-wrapper-filter-items-enum-list')
					.removeAttr('multiple');
		}

	}
}
/**
 * Leave displayed only those operators that correspond to the field_value
 * 
 * @param field_value
 */
function tow_change_operator_values(field_value) {
	// show comparison operators depending on the field type
	if (field_value.match("text") || field_value.match("char")) {
		tow_display_filter_operator('text');
	} else if (field_value.match("int") || field_value.match("float")) {
		tow_display_filter_operator('int');
	} else if (field_value.match("date")) {
		tow_display_filter_operator('date');
	} else if (field_value.match("enum")) {
		tow_display_filter_operator('enum');
	} else {
		// alert('none');
	}
}

Drupal.behaviors.Search = function(context) {

	/*
	  // Hide tables and fields to be chosen for display 
	  $("#edit-tables-select option").each(function() { 
	  // hide tables to be displayed 
	  id = "#edit-fields-select-tables-" + $(this).val() + "-wrapper"; 
	  $(id).hide(); 
	  // hide fields to be displayed 
	  id = 'div[id*="t' + $(this).val() + '"]';
	  $(id).hide(); 
	  // hide constraint fields 
	  id = 'option[value*="t' +  $(this).val() + '"]'; 
	  $(id).hide(); }); 
	  // Hide all operators
	  $('#edit-filters-filter-items-wrapper-filter-items-operator option').hide();
	 */
	
	// -------- Filters Block ---------
	// Remove filter block
	$('.remove-filter').click(function() {
		$(this).parent().hide();
		// $(this).parent().children('.value-field').children('div').children('input').val('4mpWbjErTjPdVwao');
		});
	// When changing filter field:
	$('#edit-filters-filter-items-wrapper-filter-items-field').change(
			function() {
				tow_change_operator_values($(this).val());
			});

	// Clear form button
	$('.clear-form div').click(function() {
		$('input.form-text').val('');
		$(' input.form-checkbox').attr('checked', false);
		$('option:selected').removeAttr('selected');

		$('#filter-items-wrapper').children().slice(1).hide();
	});

	// Behavior on searched table select: show only selected tables and
	// corresponding fields
	// in display block and filters block
	$('#edit-tables-select').change(function() {
		var str = "";
		var id = "";
		// alert($(this).text());

			$("#edit-tables-select option").each(function() {
				// hide tables to be displayed
					id = "#edit-fields-select-tables-" + $(this).val()
							+ "-wrapper";
					$(id).hide();
					// hide fields to be displayed
					id = 'div[id*="t' + $(this).val() + '"]';
					$(id).hide();
					// hide constraint fields
					id = 'option[value*="t' + $(this).val() + '"]';
					$(id).hide();
				});
			$("#edit-tables-select option:selected").each(function() {
				// show tables to be displayed
					id = "#edit-fields-select-tables-" + $(this).val()
							+ "-wrapper";
					$(id).show();
					// show fields to be displayed
					id = 'div[id*="t' + $(this).val() + '"]';
					$(id).show();
					// show constraint fields
					id = 'option[value*="t' + $(this).val() + '"]';
					$(id).show();

				});
		});
};
