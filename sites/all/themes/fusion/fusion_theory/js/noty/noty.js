$.hrd = {};
$.hrd.noty = function(options) {
	if(options['type'] == 'success') {
            var opt = {
		text: options['text'],
		type: options['type'],
		dismissQueue: true,
		timeout: 5000,
		layout: 'top'
	};
        }
        else {
            var opt = {
		text: options['text'],
		type: options['type'],
		dismissQueue: true,
		layout: 'top',
		closeWith: ['button'],
		force: true
            };
        }
	opt = $.extend(opt, options);
	noty(opt);
};