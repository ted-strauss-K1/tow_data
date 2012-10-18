Drupal.behaviors.document_preview = function(context) {
    
  //$("#edit-body").attr('id','wmd-input').
  //$().ready(function() { 
  $("#edit-body").addClass('wmd-panel');
  $('#edit-body').before('<div id="wmd-button-bar" class="wmd-panel"></div><br/>').after('<div id="wmd-preview" class="wmd-panel"></div><br/><div id="wmd-output" class="wmd-panel">');
  //});

  
}



