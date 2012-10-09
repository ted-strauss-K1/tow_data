Drupal.behaviors.Export = function(context) {

  // Hide and show table select dependently on user choice (whether export whole dataset or some tables).
  $('#edit-to-table').addClass('hidden');
  $('#edit-to-way-1-wrapper').change(function() {
    $('#edit-to-table').removeClass('hidden');
  });
  $('#edit-to-way-0-wrapper').change(function() {
    $('#edit-to-table').addClass('hidden');
  });
};