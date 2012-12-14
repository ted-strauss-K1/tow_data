Drupal.behaviors.log_sign = function(context) {

    // Hide Log in form/Sign in form.
    if (window.location.pathname == '/user/login') {
        $('#user-register, #user-pass').addClass('hidden');
        $('.log-tab').addClass('user-login');
    }
    else {
        $('#user-login, #user-pass').addClass('hidden');
        $('.sign-tab').addClass('user-login');
    }

    // Tabs.
    $('.sign-tab').live('click', function() {
        $('#user-login, #user-pass').addClass('hidden');
        $('#user-register').removeClass('hidden');
        $(this).addClass('user-login');
        $('.log-tab, .forget-tab').removeClass('user-login');
    });

    $('.log-tab').live('click', function() {
        $('#user-login').removeClass('hidden');
        $('#user-register, #user-pass').addClass('hidden');
        $(this).addClass('user-login');
        $('.sign-tab, .forget-tab').removeClass('user-login');
    });
    
    $('.forget-tab').live('click', function() {
        $('#user-pass').removeClass('hidden');
        $('#user-login, #user-register').addClass('hidden');
        $(this).addClass('user-login');
        $('.log-tab, .sign-tab').removeClass('user-login');
    });
}
