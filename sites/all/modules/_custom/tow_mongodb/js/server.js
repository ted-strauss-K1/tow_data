/**
 * Starts a server listening requests on port defined in the settings received from drupal.
 * The server returns JSON-formatted responses. The response includes either a full dataset if the search inside 
 * MongoDB is successful, or NULL otherwise.
 * 
 * @param {object} settings
 *  Drupal settings.
 * @param {object} pgClient
 *  PostgreSQL connection client.
 * 
 * @returns {undefined}
 */
startServer = function(settings, pgClient) {
    var http = require('http');
    var URL = require('url');

    // Port to listen requests on.
    var port = settings.tow_nodejs_port;

    // Create server.
    http.createServer(function(request, response) {

        var url = URL.parse(request.url, true);
        var path = url.pathname;

        // Set the server to allow requests to dataset section only.
        if (path !== '/dataset') {
            responseAccessDenied(response);
        }
        else {
            towAccessDatasetCheck(request, response, pgClient, url, settings);
        }
    }).listen(port);
};

/**
 * Checks user access to dataset and propagates an appropriate action according to check result.
 * 
 * @param {object} request
 *  Request object.
 * @param {object} response
 *  Reponse object.
 * @param {object} pgClient
 *  PostgreSQL connection client.
 * @param {object} url
 *  Request url parsed to an object.
 * @param {object} settings
 *  Drupal settings.
 *  
 * @returns {undefined}
 */
towAccessDatasetCheck = function(request, response, pgClient, url, settings) {
    $dataset_nid = url.query.dataset_nid;
    $user_id = url.query.user_id;

    // Check user on having access to any dataset first.
    sql = "SELECT DISTINCT(1) AS has_access FROM users_roles ur JOIN permission p ON ur.rid = p.rid WHERE p.perm LIKE '%edit any dataset%' AND ur.uid = $1";
    args = [$user_id];
    query = pgClient.query(sql, args);
    query.on('end', function(result) {

        // If there are no rows returned, then continue checking, get dataset otherwise.
        if (result.rowCount === 0) {

            // Check if the user is owner.
            sql = "SELECT DISTINCT(1) AS is_owner FROM node n WHERE n.nid = $1 AND n.uid = $2";
            args = [$dataset_nid, $user_id];
            query = pgClient.query(sql, args);
            query.on('end', function(result) {

                // If there are no rows returned, then continue checking, get dataset otherwise.
                if (result.rowCount === 0) {

                    // Check dataset access type.
                    sql = "SELECT cad.field_access_type_value AS access_type FROM node n JOIN content_type_dataset cad ON n.nid = cad.nid WHERE n.nid = $1";
                    args = [$dataset_nid];
                    query = pgClient.query(sql, args);
                    query.on('row', function(row, result) {
                        result.addRow(row);
                    });
                    query.on('end', function(result) {
                        accessType = result.rows[0].access_type;

                        // If access type is 0 then the dataset has an open access so get dataset, continue checking otherwise.
                        if (accessType !== 0) {

                            // Check access is granted for a dataset in case of private or requested access type.
                            sql = "SELECT tar.status AS request_status FROM node n LEFT JOIN tow_access_requests tar ON n.nid = tar.nid WHERE n.nid = $1 AND tar.uid = $2";
                            args = [$dataset_nid, $user_id];
                            query = pgClient.query(sql, args);
                            query.on('row', function(row, result) {
                                result.addRow(row);
                            });
                            query.on('end', function(result) {
                                if (result.rowCount === 0) {
                                    requestStatus = 0;
                                } else {
                                    requestStatus = result.rows[0].request_status;
                                }

                                // If request status is 1 then the dataset access is granted, access denied otherwise.
                                if (requestStatus !== 1) {
                                    responseAccessDenied(response);
                                } else {
                                    getDataset(request, response, url, settings);
                                }
                            });
                        } else {
                            getDataset(request, response, url, settings);
                        }
                    });
                } else {
                    getDataset(request, response, url, settings);
                }
            });
        } else {
            getDataset(request, response, url, settings);
        }
    });
};

/**
 * Gets dataset info according to parameters received from url and settings
 * and writes it to the server response.
 * 
 * @param {object} request
 *  Request object.
 * @param {object} response
 *  Reponse object.
 * @param {object} url
 *  Request url parsed to an object.
 * @param {object} settings
 *  Drupal settings.
 *  
 * @returns {undefined}
 */
getDataset = function(request, response, url, settings) {

    // Connect to MongoDB.
    var connection_url = settings.tow_mongodb_connection_url;
    var user = settings.tow_mongodb_user;
    var password = settings.tow_mongodb_password;
    var database = settings.tow_mongodb_db;

    var databaseUrl;
    if ((connection_url === 'mongodb://localhost:27017') && (user === '') && (password === '')) {
        databaseUrl = database;
    } else {
        connection_url = connection_url.replace('mongodb://', '');
        databaseUrl = user + ':' + password + '@' + connection_url + '/' + database;
        databaseUrl = databaseUrl.replace(':@', '');
    }
    var collections = ['datasets'];
    var db = require('mongojs').connect(databaseUrl, collections);

    // Get data & write it to the response.
    db.datasets.find({_id: url.query.dataset_nid}, function(err, dataset) {
        if (err || !dataset.length) {
            dataset = null;
        }

        response.writeHead(200, {"Content-Type": "application/json"});

        var json = JSON.stringify(dataset);

        response.write('parseJsonp(' + json + ');');
        response.end();
    });

};

/**
 * Makes a 403 Forbidden response.
 * 
 * @param {object} response
 *  Response object.
 *  
 * @returns {undefined}
 */
responseAccessDenied = function(response) {
    response.writeHead(403, {"Content-Type": "application/json"});

    var json = JSON.stringify(null);

    response.write('parseJsonp(' + json + ');');
    response.end();
};

drupal = require('drupal.settings');
drupal.getSettings(startServer);