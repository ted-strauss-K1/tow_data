YAHOO.namespace("drupal.calendar");

YAHOO.drupal.calendar.init = function(id) {
  if (id != null &&	 id != 'null') {
    var cal = new YAHOO.widget.Calendar(id + '-cal', id + '-calendar',  { title: 'Choose a date:', close: true } );
	cal.hide();
	
	var dropDown = YAHOO.util.Dom.get(id + '-drop-down');
	var dateOnly = YAHOO.util.Dom.get(id + '-date-only');
    if (dropDown != null && dropDown != 'null') {
		dropDown = dropDown.value;
	}
    if (dateOnly != null && dateOnly != 'null') {
		dateOnly = dateOnly.value;
	}
	
	// The happy object that will be used....
	var myCal = {'id' : id, 'cal' : cal, 'dropDown' : dropDown, 'dateOnly' : dateOnly};

    cal.selectEvent.subscribe(function (type, args, obj) { YAHOO.drupal.calendar.handleSelect(type, args, obj, myCal); }, cal, true);

    YAHOO.drupal.calendar.updateDateValue(myCal);
	if (myCal.dropDown === '1') {
	  $("#" + myCal.id).next(".description").attr('style', 'display: none;');
	  YAHOO.util.Dom.setStyle(myCal.id, 'display', 'none');
	  YAHOO.util.Dom.setStyle(myCal.id + '-year', 'display', 'inline');
	  YAHOO.util.Dom.setStyle(myCal.id + '-month', 'display', 'inline');
	  YAHOO.util.Dom.setStyle(myCal.id + '-day', 'display', 'inline');
	  if (myCal.dateOnly !== '1') {
	  	YAHOO.drupal.calendar.buildAutoComplete(myCal);
	    YAHOO.util.Dom.setStyle(myCal.id + '-time', 'display', 'inline');
      }
	}

    YAHOO.util.Event.addListener(myCal.id + '-icon', 'click', function() { YAHOO.drupal.calendar.show(myCal); } );
  }
};

YAHOO.drupal.calendar.show = function(myCal) {
  var xy = YAHOO.util.Dom.getXY(myCal.id + '-icon');
  var myDate = Date.parseDate(YAHOO.util.Dom.get(myCal.id).value, '%y-%m-%d %H:%M:%S');
  var date = myDate.format('m/d/Y');

  if (date) {
    myCal.cal.cfg.setProperty('selected', date);
    myCal.cal.cfg.setProperty('pagedate', new Date(date), true);
  }

  YAHOO.util.Dom.setStyle(myCal.id + '-calendar', 'display', 'block');
  YAHOO.util.Dom.setStyle(myCal.id + '-calendar', 'position', 'absolute');

  xy[1] = xy[1] + 20;
  YAHOO.util.Dom.setXY(myCal.id + '-calendar', xy);
    
  myCal.cal.render();
  myCal.cal.show();
};

YAHOO.drupal.calendar.handleSelect = function(type, args, obj, myCal) {
  var dates = args[0];
  var date = dates[0];
  var year = date[0], month = date[1], day = date[2];

  YAHOO.util.Dom.get(myCal.id + '-year').selectedIndex = Math.abs(year - 1969 - YAHOO.util.Dom.get(myCal.id + '-year').length + 1);
  YAHOO.util.Dom.get(myCal.id + '-month').selectedIndex = month - 1;
  YAHOO.util.Dom.get(myCal.id + '-day').selectedIndex = day - 1;

  YAHOO.drupal.calendar.updateDateValue(myCal);
};

YAHOO.drupal.calendar.updateDateValue = function(myCal) {
  YAHOO.util.Dom.get(myCal.id).value = 
    YAHOO.util.Dom.get(myCal.id + '-year')[YAHOO.util.Dom.get(myCal.id + '-year').selectedIndex].value + '-' +
    YAHOO.util.Dom.get(myCal.id + '-month')[YAHOO.util.Dom.get(myCal.id + '-month').selectedIndex].value + '-' +
    YAHOO.util.Dom.get(myCal.id + '-day')[YAHOO.util.Dom.get(myCal.id + '-day').selectedIndex].value;
  if (myCal.dateOnly !== '1') {
    YAHOO.util.Dom.get(myCal.id).value += ' ' +
      YAHOO.util.Dom.get(myCal.id + '-time').value + ' ' + YAHOO.util.Dom.get(myCal.id + '-timezone').value;
  }
};

YAHOO.drupal.calendar.buildAutoComplete = function(myCal) {
  var count = 0;
  YAHOO.drupal.timeArray = [];
  for (var h = 0; h < 24; h++) {
    for (var m = 0; m < 60; m++) {
      YAHOO.drupal.timeArray[count] = (h < 10 ? '0' : '') + h + ':' + (m < 10 ? '0' : '') + m + ':00';
      count++;
    }
  }

  YAHOO.drupal.ACJSArray = new function() {
    this.oACDS = new YAHOO.widget.DS_JSArray(YAHOO.drupal.timeArray);
    this.oAutoComp = new YAHOO.widget.AutoComplete(myCal.id + '-time', myCal.id + '-time-container', this.oACDS);
    this.oAutoComp.prehighlightClassName = 'yui-ac-prehighlight';
    this.oAutoComp.typeAhead = true;
    this.oAutoComp.useShadow = true;
    this.oAutoComp.minQueryLength = 0;
    this.oAutoComp.textboxFocusEvent.subscribe(function(){
      var sInputValue = YAHOO.util.Dom.get(myCal.id + '-time').value;
      if (sInputValue.length === 0) {
        var oSelf = this;
        setTimeout(function() { oSelf.sendQuery(sInputValue); }, 0);
      }
    });
  };

  YAHOO.util.Event.addListener(myCal.id + '-year', 'change', function() { YAHOO.drupal.calendar.updateDateValue(myCal) });
  YAHOO.util.Event.addListener(myCal.id + '-month', 'change', function() { YAHOO.drupal.calendar.updateDateValue(myCal) });
  YAHOO.util.Event.addListener(myCal.id + '-day', 'change', function() { YAHOO.drupal.calendar.updateDateValue(myCal) });
  YAHOO.util.Event.addListener(myCal.id + '-time', 'keyup', function() { YAHOO.drupal.calendar.updateDateValue(myCal) });
};

// DATE
// Initial version borrowed from Drupal jstools module (jstools has a similar calendar module).
Date._MD = new Array(31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31);
Date.SECOND = 1000;
Date.MINUTE = 60 * Date.SECOND;
Date.HOUR = 60 * Date.MINUTE;
Date.DAY = 24 * Date.HOUR;
Date.WEEK = 7 * Date.DAY;
Date.parseDate = function (str, fmt) {
  var today = new Date();
  var y = 0;
  var m = -1;
  var d = 0;
  var a = str.split(/\W+/g);
  var b = fmt.match(/%./g);
  var i = 0, j = 0;
  var hr = 0;
  var min = 0;

  for (i = 0; i < a.length; i++) {
    if (!a[i])
      continue;

    switch (b[i]) {
      case "%d":
      case "%e":
        d = parseInt(a[i], 10);
        break;
      case "%m":
        m = parseInt(a[i], 10)-1;
        break;
      case "%Y":
      case "%y":
        y = parseInt(a[i], 10);
        y < 100 && (y += (y > 29 ? 1900 : 2000));
        break;
      case "%b":
      case "%B":
        for (j = 0; j < 12; j++) {
          if (Calendar._MN[j].substr(0, a[i].length).toLowerCase() == a[i].toLowerCase()) {
            m = j;
            break;
          }
        }
        break;
      case "%H":
      case "%I":
      case "%k":
      case "%l":
        hr = parseInt(a[i], 10);
        break;
      case "%P":
      case "%p":
        if (pm.test(a[i]) && hr < 12)
          hr += 12;
        else
          if (am.test(a[i]) && hr >= 12)
            hr -= 12;
        break;
      case "%M":
        min = parseInt(a[i], 10);
        break;
      case "%O":
        break;
    }
  }

  if (isNaN(y))
    y = today.getFullYear();
  if (isNaN(m))
    m = today.getMonth();
  if (isNaN(d))
    d = today.getDate();
  if (isNaN(hr))
    hr = today.getHours();
  if (isNaN(min))
    min = today.getMinutes();
  if (y != 0 && m != -1 && d != 0)
    return new Date(y, m, d, hr, min, 0);

  y = 0;
  m = -1;
  d = 0;
  for (i = 0; i < a.length; i++) {
    if (a[i].search(/[a-zA-Z]+/) != -1) {
      var t = -1;

      for (j = 0; j < 12; j++) {
        if (Calendar._MN[j].substr(0, a[i].length).toLowerCase() == a[i].toLowerCase()) {
          t = j;
          break;
        }
      }
      if (t != -1) {
        if (m != -1) {
          d = m + 1;
        }
        m = t;
      }
    }
    else if (parseInt(a[i], 10) <= 12 && m == -1) {
      m = a[i] - 1;
    }
    else if (parseInt(a[i], 10) > 31 && y == 0) {
      y = parseInt(a[i], 10);
      y < 100 && (y += y > (29 ? 1900 : 2000));
    }
    else if (d == 0) {
      d = a[i];
    }
  }
  if (y == 0)
    y = today.getFullYear();

  if (m != -1 && d != 0)
    return new Date(y, m, d, hr, min, 0, 0, 5);

  return today;
};

// The following date function was borrowed from: http://www.jacwright.com/projects/javascript/date_format.
 Date.prototype.format = function(format) {
  var returnStr = '';
  var replace = Date.replaceChars;
  for (var i = 0; i < format.length; i++) {
    var curChar = format.charAt(i);

    if (replace[curChar])
      returnStr += replace[curChar].call(this);
    else
      returnStr += curChar;
  }
  return returnStr;
};

Date.replaceChars = {
  shortMonths: ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'],
  longMonths: ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'],
  shortDays: ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'],
  longDays: ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'],

  // Day
  d: function() { return (this.getDate() < 10 ? '0' : '') + this.getDate(); },
  D: function() { return Date.replaceChars.shortDays[this.getDay()]; },
  j: function() { return this.getDate(); },
  l: function() { return Date.replaceChars.longDays[this.getDay()]; },
  N: function() { return this.getDay() + 1; },
  S: function() { return (this.getDate() % 10 == 1 && this.getDate() != 11 ? 'st' : (this.getDate() % 10 == 2 && this.getDate() != 12 ? 'nd' : (this.getDate() % 10 == 13 && this.getDate() != 1 ? 'rd' : 'th'))); },
  w: function() { return this.getDay(); },
  z: function() { return "Not Yet Supported"; },

  // Week
  W: function() { return "Not Yet Supported"; },

  // Month
  F: function() { return Date.replaceChars.longMonths[this.getMonth()]; },
  m: function() { return (this.getMonth() < 11 ? '0' : '') + (this.getMonth() + 1); },
  M: function() { return Date.replaceChars.shortMonths[this.getMonth()]; },
  n: function() { return this.getMonth() + 1; },
  t: function() { return "Not Yet Supported"; },

  // Year
  L: function() { return "Not Yet Supported"; },
  o: function() { return "Not Supported"; },
  Y: function() { return this.getFullYear(); },
  y: function() { return ('' + this.getFullYear()).substr(2); },

  // Time
  a: function() { return this.getHours() < 12 ? 'am' : 'pm'; },
  A: function() { return this.getHours() < 12 ? 'AM' : 'PM'; },
  B: function() { return "Not Yet Supported"; },
  g: function() { return this.getHours() == 0 ? 12 : (this.getHours() > 12 ? this.getHours() - 12 : this.getHours()); },
  G: function() { return this.getHours(); },
  h: function() { return (this.getHours() < 10 || (12 < this.getHours() < 22) ? '0' : '') + (this.getHours() < 10 ? this.getHours() + 1 : this.getHours() - 12); },
  H: function() { return (this.getHours() < 10 ? '0' : '') + this.getHours(); },
  i: function() { return (this.getMinutes() < 10 ? '0' : '') + this.getMinutes(); },
  s: function() { return (this.getSeconds() < 10 ? '0' : '') + this.getSeconds(); },

  // Timezone
  e: function() { return "Not Yet Supported"; },
  I: function() { return "Not Supported"; },
  O: function() { return (this.getTimezoneOffset() < 0 ? '-' : '+') + (this.getTimezoneOffset() / 60 < 10 ? '0' : '') + (this.getTimezoneOffset() / 60) + '00'; },
  T: function() { return "Not Yet Supported"; },
  Z: function() { return this.getTimezoneOffset() * 60; },

  // Full Date/Time
  c: function() { return "Not Yet Supported"; },
  r: function() { return this.toString(); },
  U: function() { return this.getTime() / 1000; }
};
