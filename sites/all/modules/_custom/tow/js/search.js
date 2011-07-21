Drupal.behaviors.Search = function(context) {
  
  $('.remove-filter').click(function() {
    $(this).parent().hide();
    //$(this).parent().children('.value-field').children('div').children('input').val('4mpWbjErTjPdVwao');
  });

  $('.clear-form div').click(function(){
    $('input.form-text').val('');
    $(' input.form-checkbox').attr('checked', false);
    $('option:selected').removeAttr('selected');
    
    $('#filter-items-wrapper').children().slice(1).hide();
  });
};