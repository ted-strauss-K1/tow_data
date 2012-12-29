$.hrd = {};
$.hrd.noty = function(options) {
	if(options['type'] == 'success') {
            var opt = {
		text: options['text'],
		type: options['type'],
		dismissQueue: true,
		timeout: 5000,
		force: true,
		layout: 'topCenter'
	};
        }
        else {
            var opt = {
		text: options['text'],
		type: options['type'],
		dismissQueue: true,
		layout: 'topCenter',
		closeWith: ['button'],
		force: true
            };
        }
	opt = $.extend(opt, options);
	noty(opt);
};