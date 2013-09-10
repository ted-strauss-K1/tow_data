Drupal.behaviors.inner_search = function(context) {

    function init() {
        var port = 1337;
        $.ajax({
            url: 'http://' + location.host + ':' + port + '/',
            data: {},
            dataType: 'json',
            success: function(data) {
                console.log(data);
            },
            error: function(jqXHR) {
                console.log(jqXHR);
            }
        });
    }

    init();
};