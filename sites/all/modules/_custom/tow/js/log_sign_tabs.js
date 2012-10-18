Drupal.behaviors.log_sign = function(context) {

  //Hide Log in form/Sign in form
  if (window.location.pathname == '/user/login') {
    $('form:last').addClass('hidden');
	$('.log-tab').addClass('user-login');
  }
  else {
    $('form:first').addClass('hidden');
	$('.sign-tab').addClass('user-login');
  }
  
  //Tabs
  $('.sign-tab').live('click', function() {
    $('#user-login').addClass('hidden');
	$('#user-register').removeClass('hidden');
	$(this).addClass('user-login');
	$('.log-tab').removeClass('user-login');
  });
  
  $('.log-tab').live('click', function() {
    $('#user-login').removeClass('hidden');
	$('#user-register').addClass('hidden');
	$(this).addClass('user-login');
	$('.sign-tab').removeClass('user-login');
  });
	
  
}



