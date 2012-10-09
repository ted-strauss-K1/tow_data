
(function ($, Drupal, window, undefined) {
  Drupal.automodal.settingsAlter = {
    automodal: function(settings) {
      
	  if (settings.automodalRedirect == undefined) {
        settings.url += '&destination=' + top.window.location.pathname.substring(1);
		alert(settings.url);
      }
	  
      return settings;
    }
  }
})(jQuery, Drupal, window);