Drupal.behaviors.inner_search = function(context) {

    var port = Drupal.settings.tow_mongodb.hodejs_port;
    var dataset_nid = Drupal.settings.tow_mongodb.dataset_nid;

    function init() {

        $.ajax({
            url: 'http://' + location.host + ':' + port + '/dataset',
            type: 'GET',
            data: {
                dataset_nid: dataset_nid
            },
            dataType: 'jsonp',
            async: false,
            jsonpCallback: 'parseJsonp',
            success: function(data) {
                console.log(data);
            },
            error: function(jqXHR) {
                console.log(jqXHR);
            }
        });
    }

    function parseJsonp(json) {
        return json;
    }

    init();
};