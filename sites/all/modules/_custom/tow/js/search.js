/**
 * Leaves displayed only those operators and input fields that are applicable
 * for specific field type
 *
 * @param type
 *            type of the field
 */
function tow_display_filter_operator(type) {
    $('#edit-filters-filter-items-wrapper-filter-items-operator option:[value*="' + type + '"]').removeClass('hidden');
    $('#edit-filters-filter-items-wrapper-filter-items-operator option:not([value*="' + type + '"])').addClass('hidden');

    operator_value = $('#edit-filters-filter-items-wrapper-filter-items-operator option:[value*="' + type + '"]').val();

    $('#edit-filters-filter-items-wrapper-filter-items-operator').val(operator_value);

    // Show corresponding input field depending on operator value.
    tow_display_input_fields_for_operator($('#edit-filters-filter-items-wrapper-filter-items-operator').val());
}

/**
 * Leaves displayed only those input elements that are applicable for specific
 * operator
 *
 * @param operator_value
 */
function tow_display_input_fields_for_operator(operator_value) {

    // Display input field if required.
    if (operator_value.match("text") || operator_value.match("char") || operator_value.match("float") || operator_value.match("int")) {

        // Display only one input field,
        $('#edit-filters-filter-items-wrapper-filter-items-value-1-wrapper').removeClass('hidden');

        // Hide all date and enum fields.
        if (operator_value.match("int-between")) {
            $('#edit-filters-filter-items-wrapper-filter-items-value-2-wrapper').removeClass('hidden');
        }
        else {
            $('#edit-filters-filter-items-wrapper-filter-items-value-2-wrapper').addClass('hidden');
        }
        $('#edit-filters-filter-items-wrapper-filter-items-date-within-wrapper').addClass('hidden');
        $('#edit-filters-filter-items-wrapper-filter-items-date-value-1-wrapper').addClass('hidden');
        $('#edit-filters-filter-items-wrapper-filter-items-date-value-1-day-wrapper').addClass('hidden');
        $('#edit-filters-filter-items-wrapper-filter-items-date-value-2-wrapper').addClass('hidden');
        $('#edit-filters-filter-items-wrapper-filter-items-date-value-2-day-wrapper').addClass('hidden');
        $('#edit-filters-filter-items-wrapper-filter-items-enum-list-wrapper').addClass('hidden');

    }
    else {
        if (operator_value.match("date")) {
            $('#edit-filters-filter-items-wrapper-filter-items-value-1-wrapper').addClass('hidden');
            $('#edit-filters-filter-items-wrapper-filter-items-value-2-wrapper').addClass('hidden');
            if (operator_value.match("date-within")) {
                $('#edit-filters-filter-items-wrapper-filter-items-date-within-wrapper').removeClass('hidden');
            }
            else {
                $('#edit-filters-filter-items-wrapper-filter-items-date-within-wrapper').addClass('hidden');
            }
            $('#edit-filters-filter-items-wrapper-filter-items-date-value-1-wrapper').removeClass('hidden');
            $('#edit-filters-filter-items-wrapper-filter-items-date-value-1-day-wrapper').removeClass('hidden');
            if (operator_value.match("date-between")) {
                $('#edit-filters-filter-items-wrapper-filter-items-date-value-2-wrapper').removeClass('hidden');
                $('#edit-filters-filter-items-wrapper-filter-items-date-value-2-day-wrapper').removeClass('hidden');
            }
            else {
                $('#edit-filters-filter-items-wrapper-filter-items-date-value-2-wrapper').addClass('hidden');
                $('#edit-filters-filter-items-wrapper-filter-items-date-value-2-day-wrapper').addClass('hidden');
            }
            $('#edit-filters-filter-items-wrapper-filter-items-enum-list-wrapper').addClass('hidden');
        }
        else {
            if (operator_value.match("enum")) {
                $('#edit-filters-filter-items-wrapper-filter-items-value-1-wrapper').addClass('hidden');
                $('#edit-filters-filter-items-wrapper-filter-items-value-2-wrapper').addClass('hidden');
                $('#edit-filters-filter-items-wrapper-filter-items-date-within-wrapper').addClass('hidden');
                $('#edit-filters-filter-items-wrapper-filter-items-date-value-1-wrapper').addClass('hidden');
                $('#edit-filters-filter-items-wrapper-filter-items-date-value-2-wrapper').addClass('hidden');
                $('#edit-filters-filter-items-wrapper-filter-items-date-value-1-day-wrapper').addClass('hidden');
                $('#edit-filters-filter-items-wrapper-filter-items-date-value-2-day-wrapper').addClass('hidden');
                $('#edit-filters-filter-items-wrapper-filter-items-enum-list-wrapper').removeClass('hidden');
                if (operator_value.match("enum-anyof")) {
                    $('#edit-filters-filter-items-wrapper-filter-items-enum-list').attr('multiple', 'multiple');
                }
                else {
                    $('#edit-filters-filter-items-wrapper-filter-items-enum-list').removeAttr('multiple');
                }
            }
        }
    }
}

/**
 * Leave displayed only those operators that correspond to the field_value
 *
 * @param field_value
 */
function tow_change_operator_values(field_value) {

    // Show comparison operators depending on the field type.
    if (field_value.match("text") || field_value.match("char")) {
        tow_display_filter_operator('text');
    }
    else {
        if (field_value.match("int") || field_value.match("float")) {
            tow_display_filter_operator('int');
        }
        else {
            if (field_value.match("date")) {
                tow_display_filter_operator('date');
            }
            else {
                if (field_value.match("enum")) {
                    tow_display_filter_operator('enum');
                }
            }
        }
    }
}

Drupal.behaviors.Search = function(context) {

    // -------- Filters Block ---------
    // Remove filter block.
    $('.remove-filter').click(function() {
        $(this).parent().addClass('hidden');
    });

    // When changing filter field.
    $('#edit-filters-filter-items-wrapper-filter-items-field').change(function() {
        tow_change_operator_values($(this).val());
    });

    // Clear form button.
    $('.clear-form div').click(function() {
        $('input.form-text').val('');
        $('input.form-checkbox').attr('checked', false);
        $('option:selected').removeAttr('selected');
        $('#filter-items-wrapper').children().slice(1).addClass('hidden');
    });

    // Behavior on searched table select: show only selected tables and
    // corresponding fields in display block and filters block.
    $('#edit-tables-select').change(function() {
        var str = "";
        var id = "";

        $("#edit-tables-select option").each(function() {

            // Hide tables to be displayed.
            id = "#edit-fields-select-tables-" + $(this).val() + "-wrapper";
            $(id).addClass('hidden');

            // Hide fields to be displayed.
            id = 'div[id*="t' + $(this).val() + '"]';
            $(id).addClass('hidden');

            // Hide constraint fields.
            id = 'option[value*="t' + $(this).val() + '"]';
            $(id).addClass('hidden');
        });

        $("#edit-tables-select option:selected").each(function() {

            // Show tables to be displayed.
            id = "#edit-fields-select-tables-" + $(this).val() + "-wrapper";
            $(id).removeClass('hidden');

            // Show fields to be displayed.
            id = 'div[id*="t' + $(this).val() + '"]';
            $(id).removeClass('hidden');

            // Show constraint fields.
            id = 'option[value*="t' + $(this).val() + '"]';
            $(id).removeClass('hidden');

            // Set first field value.
            $('#edit-filters-filter-items-wrapper-filter-items-field').val($(id).val());

            // Set correct operator.
            tow_change_operator_values($('#edit-filters-filter-items-wrapper-filter-items-field').val());
        });
    });

    // When changing operator.
    $('#edit-filters-filter-items-wrapper-filter-items-operator').change(function() {
        tow_display_input_fields_for_operator($(this).val());
    });
};
