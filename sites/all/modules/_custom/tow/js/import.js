/**
 * @file
 *
 * User interface adjustment for import page.
 */

Drupal.behaviors.Import = function(context) {
    //Create modal window
    $('#content-content', context).prepend('<div id="myModal" class="modal hide fade"><div class="modal-header"><button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button><h3>Import data</h3></div><div class="modal-body"><p>One fine body…</p><div class="progress progress-striped active"><div class="bar" style="width: 0%;"></div></div></div><div class="modal-footer"><a class="cancel-modal" href="#" data-dismiss="modal" aria-hidden="true">Cancel</a></div></div>');
    //Cancel downloading after Cancel link was clicked
    $('#myModal a.cancel-modal, #myModal button.close', context).live('click', function() {
        $("#myModal").remove();
        window.location.href = window.location.href.replace('import', 'node');
    });

    // Hide and show table select dependently on user choice (whether import to dataset or to table).
    $('#edit-to-table').addClass('hidden');
    $('#edit-to-way-1-wrapper').change(function() {
        $('#edit-to-table').removeClass('hidden');
    });
    $('#edit-to-way-0-wrapper').change(function() {
        $('#edit-to-table').addClass('hidden');
    });

    // Hide progress bar.
    $('.progress-bar').addClass('hidden');
    
    var hash;
    var dataset = location.pathname.replace(/\import/g, "").replace(/\//g, "");
    var lock = false;

    if ($('#edit-hash').val() != '0') {
        $(document).ready(function() {
            hash = $('#edit-hash').val();
            fileUploaded(hash);
            return;
        });
    }
    else {
        hash = Math.round(Math.random() * 1000000);
        $('#edit-hash').val(hash);
    }

    // For Firefox >= 3.6 add drag&drop feature.
    if ($.browser.mozilla == true && parseInt($.browser.version.replace(/\./g, "")) > 19200) {
        dragFile(hash);
    }
    else {
        $('#edit-upload-wrapper div.description').addClass('hidden');
    }

    $('#edit-upload').change(function() {

        if (lock) {
            return;
        }
        lock = true;

        var files = this.files;
        if (files != undefined) {
            if (files.length != 1) {
                return;
            }
            fileUpload(hash, files[0]);
        }
    });

    $('.get-it').click(function() {

        if (lock) {
            return;
        }
        lock = true;

        var grab_url = $('#edit-url').val();

        if (grab_url != "") {
            var table = getChosenTable();
            $.ajax({
                url: 'http://' + location.host + '/import_grab/' + hash + '/' + dataset + '/' + table,
                data: {
                    grab_url : grab_url
                },
                beforeSend: function() {
                    $('.progress-bar').removeClass('hidden');
                    lpStart(hash, 1, 0);
                }
            });
        }

    });

    var lpStart = function(hash, stage, cycle) {

        cycle++;
        if (cycle > 2000) {
            return;
        }

        $.ajax({
            url: 'http://' + location.host + '/import_progress_get/' + hash + '/' + stage,
            dataType: 'json',
            success: function (data, textStatus) {
                lpOnComplete(hash, stage, data.response, cycle);
            }
        });
    };

    var lpOnComplete = function(hash, stage, response, cycle) {

        var dots;
        var nextStage = stage;

        if (stage <= 3) {

            if (response == 10) {
                $('.progress-table tr.' + stage + ' td.dots').text('..........');
                $('.progress-table tr.' + stage + ' td.status').text('ok');
                $('.progress-table tr.' + stage + ' td.status').addClass('ok');
                nextStage++;
            }
            else {
                if (response == -1) {
                    $('.progress-table tr.' + stage + ' td.status').text('error');
                    $('.progress-table tr.' + stage + ' td.status').addClass('error');
                    nextStage = 4;
                }
                else {
                    $('#myModal').modal('show');
                    if (stage == 1) {
                        $('div.modal-body p').text('Uploading your file to the server...');
                        $('div.progress div.bar').css('width', response*10/3 + '%');
                    } else if (stage == 2) {
                        $('div.modal-body p').text('Validating files...');
                        if (response == 0) {
                            $('div.progress div.bar').css('width', '33%');
                        } else {
                            $('div.progress div.bar').css('width', 33 + response*10/3 + '%');
                        }
                        
                    } else if (stage == 3) {
                        $('div.modal-body p').text('Creating content...');
                        if (response == 0) {
                            $('div.progress div.bar').css('width', '66%');
                        } else {
                            $('div.progress div.bar').css('width', 66 + response*10/3 + '%');
                        }
                    }
                    
                    dots = '';
                    for (var i = 1; i <= response; i++) {
                        dots += '.';
                    }
                    $('.progress-table tr.' + stage + ' td.dots').text(dots);
                }
            }

        }
        else {
            $('div.progress div.bar').css('width', '100%');

            if (stage >= 4 && stage <= 5){

                if (response != false) {

                    if (response !== '-1') {
                        $('.progress-table tr.' + stage + ' td.message').text(response);
                        $('.progress-table tr.' + stage).removeClass('hidden');
                    }
                    nextStage++;
                }
            }
            else {
                if (response != false) {

                    var ind = response.indexOf("/");
                    var message = response.slice(0, ind) + ' ';
                    var tables = response.slice(ind + 1);

                    ind = tables.indexOf("/");

                    var nids = tables.slice(ind + 1);
                    tables = tables.slice(0, ind);

                    nids = explode(' ', trim(nids));
                    tables = explode(' ', trim(tables));

                    for(var n = 0; n<tables.length; n++) {
                        if (nids[n]) {
                            message += '<a href="' + location.protocol + '//' + location.host + '/table/' + nids[n] + '">' + tables[n] + '</a>' + '; ';
                        }
                    }

                    message = rtrim(message);
                    message = rtrim(message, ';');
                    $('.progress-table tr.6 td.message').html(message);
                    $('.progress-table tr.6 td.status').text('ok');
                    $('.progress-table tr.6 td.status').addClass('ok');
                    $('.progress-table tr.6').removeClass('hidden');
                    $('.progress-table tr.7').removeClass('hidden');
                    
                    $('#myModal').modal('hide');

                    return;
                }
            }
        }
        setTimeout(function() {
            lpStart(hash, nextStage, cycle);
        }, 1000);

    };

    function getChosenTable() {

        if ($('#edit-to-way-1').attr('checked') == 'checked') {
            return $('#edit-to-table').val();
        }
        else {
            return false;
        }
    }

    function dragFile(hash) {

        $('#edit-upload').filedrop({
            url: 'upload_dragged',
            paramname: 'files',
            data: {
                hash: hash, // send POST variables
                table: function() {
                    return getChosenTable();
                },
                dataset: dataset
            },
            uploadStarted: function(i, file, len){
                $('.progress-bar').removeClass('hidden');
                lpStart(hash, 1, 0);
            },
            maxfiles: 1,
            beforeEach: function(file) {
                if (lock == false) {
                    lock = true;
                    return true;
                }
                else {
                    return false;
                }
            }
        });
    };

    function fileUpload(hash, file) {

        var fileName = file.name;
        var uri = 'upload_selected/' + hash + '/' + dataset + '/' + getChosenTable() + '';
        var xhr = new XMLHttpRequest();

        xhr.open("POST", uri, true);
        $('.progress-bar').removeClass('hidden');
        lpStart(hash, 1, 0);

        if (typeof FormData != 'undefined'){
            var fData = new FormData();
            fData.append('files', file);
            xhr.send(fData);
        }
        else {
            if (xhr.sendAsBinary) {
                var fReader = new FileReader();
                fReader.addEventListener(
                    'load',
                    function() {
                        var boundaryString = 'xxxxxxxxxxxx';
                        var boundary = '--' + boundaryString;
                        var requestbody = '';

                        requestbody += boundary + '\r\n';
                        requestbody += 'Content-Disposition: form-data; name="files"; filename="' + file.name + '"' + '\r\n'; // Parameter name � upfile.
                        requestbody += 'Content-Type: application/octet-stream' + '\r\n';
                        requestbody += '\r\n';
                        requestbody += fReader.result; // Binary file content.
                        requestbody += '\r\n';
                        requestbody += boundary;

                        xhr.setRequestHeader("Content-type", 'multipart/form-data; boundary="' + boundaryString + '"');
                        xhr.setRequestHeader("Connection", "close");
                        xhr.setRequestHeader("Content-length", requestbody.length);
                        xhr.sendAsBinary(requestbody);
                    },
                    false
                    );
                fReader.readAsBinaryString(file);
            }
            else {
                $('input#edit-submit').removeClass('hidden');
            }
        }

        return true;
    }

    cycle = 0;

    function fileUploaded(hash) {
        $('.progress-bar').removeClass('hidden');
        var stage  =1;
        var cycle = 0;

        $.ajax({
            url: 'http://' + location.host + '/import_progress_get/' + hash + '/' + stage,
            dataType: 'json',
            success: function (data, textStatus) {
                lpOnComplete(hash, stage, data.response, cycle);
            }
        });

        return true;
    }

    // Split a string by a string.
    function explode( delimiter, string ) {

        var emptyArray = {
            0: ''
        };

        if (arguments.length != 2 || typeof arguments[0] == 'undefined' || typeof arguments[1] == 'undefined') {
            return null;
        }

        if (delimiter === '' || delimiter === false || delimiter === null) {
            return false;
        }

        if (typeof delimiter == 'function' || typeof delimiter == 'object' || typeof string == 'function' || typeof string == 'object') {
            return emptyArray;
        }

        if (delimiter === true) {
            delimiter = '1';
        }

        return string.toString().split (delimiter.toString());
    }

    function trim(str, chars) {
        return ltrim(rtrim(str, chars), chars);
    }

    function ltrim(str, chars) {
        chars = chars || "\\s";
        return str.replace(new RegExp("^[" + chars + "]+", "g"), "");
    }

    function rtrim(str, chars) {
        chars = chars || "\\s";
        return str.replace(new RegExp("[" + chars + "]+$", "g"), "");
    }
};