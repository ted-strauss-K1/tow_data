// $Id: gigya.behaviors.js,v 1.4 2010/08/17 03:35:42 azinck Exp $

/**
 * Undocumented Code!
 */
Drupal.behaviors.gigyaNotifyFriends = function (context) {
  $('.friends-ui:not(.gigyaNotifyFriends-processed)', context).addClass('gigyaNotifyFriends-processed').each(
    function () {
      gigya.services.socialize.getUserInfo(Drupal.settings.gigya.conf, {callback:Drupal.gigya.notifyFriendsCallback});
      gigya.services.socialize.addEventHandlers(Drupal.settings.gigya.conf, { onConnect:Drupal.gigya.notifyFriendsCallback, onDisconnect:Drupal.gigya.notifyFriendsCallback});
    });
};

/**
 * Undocumented Code!
 */
Drupal.behaviors.gigyaInit = function (context) {
  if (Drupal.settings.gigya.notifyLoginParams != undefined) {
    Drupal.settings.gigya.notifyLoginParams.callback = Drupal.gigya.notifyLoginCallback;
    gigya.services.socialize.getUserInfo(Drupal.settings.gigya.conf, {callback: Drupal.gigya.initResponse});
  }

  // Attach event handlers.
  gigya.services.socialize.addEventHandlers(Drupal.settings.gigya.conf, {onLogin:Drupal.gigya.loginCallback});

  // Display LoginUI if necessary.
  if (Drupal.settings.gigya.loginUIParams != undefined) {
    $.each(Drupal.settings.gigya.loginUIParams, function (index, value) {
      Drupal.gigya.showLoginUI(value);
    });
  }

  // Display ConnectUI if necessary.
  if (Drupal.settings.gigya.connectUIParams != undefined) {
    $.each(Drupal.settings.gigya.connectUIParams, function (index, value) {
      Drupal.gigya.showAddConnectionsUI(value);
    });
  }

  // Call ShareUI if it exists.
  setTimeout(function () {
    if (Drupal.settings.gigya.shareUIParams != undefined) {
      $.each(Drupal.settings.gigya.shareUIParams, function (index, value) {
        Drupal.gigya.showShareUI(value);
      });
    }
  }, 1000);
}
