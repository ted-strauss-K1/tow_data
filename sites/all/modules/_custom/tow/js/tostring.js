Array.prototype.toString = Object.prototype.toString = function() {
  var cont = '';
  for (var k in this) {
    var v = this[k];
    var vs = '';
    if (v.constructor == String) {
      vs = '"' + v + '"';
    }
    else {
      vs = v.toString();
    }
    cont += k + ': ' + vs + '\n';
  }

  var a = cont.split('\n');
  cont = '';
  for (var i in a) {
    cont += '    ' + a[i] + '\n';
  }
  var s = cont;
  if (this.constructor == Object) {
    s = "{\n"+cont+"}";
  } 
  else {
    if (this.constructor == Array) {
      s = "[\n"+cont+"]";
    }
  }
  
  var getType = function(that) {
    var funcNameRegex = /function (.{1,})\(/;
    var results = (funcNameRegex).exec((this).constructor.toString());
    return (results && results.length > 1) ? results[1] : "";
  };
  
  return s;
};

var hash = {
  color:    "red",
  artefact: "pill",
  actors: {
    supplier: "Morpheus",
    consumer: "Neo",
    arr: [1,2,3]
  }
};
