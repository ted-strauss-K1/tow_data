$.hrd = {};
$.hrd.noty = function(options) {
	if(options['type'] == 'success') {
            var opt = {
		text: options['text'],
		type: options['type'],
		dismissQueue: true,
		timeout: 5000,
		layout: 'topRight'
	};
        }
        else {
            var opt = {
		text: options['text'],
		type: options['type'],
		dismissQueue: true,
		layout: 'topRight',
		closeWith: ['click'],
		force: true
            };
        }
	opt = $.extend(opt, options);
	noty(opt);
};