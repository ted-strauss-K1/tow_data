Drupal.behaviors.dataset_tags = function(context) {
  
    //Dataset tags autocomplete. Add commas
    $('#autocomplete ul li div div', context).live('click', function(e) {
       if($('div[id^="edit-taxonomy-tags-"] input[id^="edit-taxonomy-tags-"]').val()) {
            setTimeout(function() {
                var liveValue = $('div[id^="edit-taxonomy-tags-"] input[id^="edit-taxonomy-tags-"]').val();
                $('div[id^="edit-taxonomy-tags-"] input[id^="edit-taxonomy-tags-"]').val(liveValue + ', ');
             }, 150);
       }
    });
    $('div[id^="edit-taxonomy-tags-"] input[id^="edit-taxonomy-tags-"]', context).live('keypress', function(e) {
        if(e.which == 13) {
            if($(this).val()) {
                setTimeout(function() {
                    var liveValue = $('div[id^="edit-taxonomy-tags-"] input[id^="edit-taxonomy-tags-"]').val();
                    if (liveValue.substr(-2) != ', ') {
                        $('div[id^="edit-taxonomy-tags-"] input[id^="edit-taxonomy-tags-"]').val(liveValue + ', ');
                    }
                 }, 150);
            }
        }
    });
  
}