startServer = function(settings) {
    var http = require('http');
    var URL = require('url');

    var port = settings.tow_nodejs_port;

    http.createServer(function(request, response) {

        var connection_url = settings.tow_mongodb_connection_url.replace('mongodb://', '');
        var user = settings.tow_mongodb_user;
        var password = settings.tow_mongodb_password;
        var database = settings.tow_mongodb_db;
        var databaseUrl = user + ':' + password + '@' + connection_url + '/' + database;
        databaseUrl = databaseUrl.replace(':@', '');
        var collections = ['datasets'];
        var db = require('mongojs').connect(databaseUrl, collections);

        var url = URL.parse(request.url, true);
        var path = url.pathname;

        if (path !== '/dataset') {
            response.writeHead(403, {"Content-Type": "application/json"});

            var json = JSON.stringify(null);

            response.write('parseJsonp(' + json + ');');
            response.end();
        }
        else {
            db.datasets.find({_id: url.query.dataset_nid}, function(err, dataset) {
                if (err || !dataset.length) {
                    dataset = null;
                }

                response.writeHead(200, {"Content-Type": "application/json"});

                var json = JSON.stringify(dataset);

                response.write('parseJsonp(' + json + ');');
                response.end();
            });
        }
    }).listen(port);
};

drupal = require('drupal.settings');
drupal.getSettings(startServer);