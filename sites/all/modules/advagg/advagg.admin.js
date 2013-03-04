
/**
 * Toggle the advagg cookie
 */
function advagg_toggle_cookie() {
  var cookie_name = 'AdvAggDisabled';

  // See if the cookie exists.
  var cookie_pos = document.cookie.indexOf(cookie_name + '=' + Drupal.settings.advagg.key);

  // If the cookie does exist then remove it.
  if (cookie_pos !== -1) {
    document.cookie = cookie_name + '=;'
      + 'expires=-1;'
      + ' path=' + Drupal.settings.basePath + ';'
      + ' domain=.' + document.location.hostname + ';';
    alert(Drupal.t('AdvAgg Bypass Cookie Removed'));
  }
  // If the cookie does not exsit then set it.
  else {
    document.cookie = cookie_name + '=' + Drupal.settings.advagg.key + ';'
      + ' path=' + Drupal.settings.basePath + ';'
      + ' domain=.' + document.location.hostname + ';';
    alert(Drupal.t('AdvAgg Bypass Cookie Set'));
  }

  // Must return false, if returning true then form gets submitted.
  return false;
}

