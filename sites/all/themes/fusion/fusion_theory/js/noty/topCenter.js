;
(function($) {

    $.noty.layouts.topCenter = {
        name: 'topCenter',
        options: { // overrides options

        },

        container: {
            object: '<div id="noty_topCenter_layout_container" />',
            selector: 'div#noty_topCenter_layout_container',
            style: function() {
                $(this).css({
                    top: 0,
                    left: '0!important',
                    //right:0,
                    //position: 'fixed',
                    height: 'auto',
                    margin: '0 auto',
                    padding: 0,
                    listStyleType: 'none',
                    zIndex: 10000000
                });

               /*$('div#noty_topCenter_layout_container').css({
                    left: ($(window).width() - $('div#noty_topCenter_layout_container').outerWidth()) / 2 + 'px',
                });*/
                    $('body > div#noty_topCenter_layout_container').wrap('<div class="centrier" />');

            }
        },
        parent: {
            object: '<div />',
            selector: 'div',
            css: {}
        },
        css: {
            display: 'none'
        },
        addClass: ''

    };
})(jQuery);