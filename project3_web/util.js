var U = {
  max: function(ar) {
    var ret = 0;
    ar.forEach(function(x) {
      if (x > ret) ret = x;
    });
    return ret;
  },
  min: function(ar) {
    var ret = ar[0];
    ar.forEach(function(x) {
      if (x < ret) ret = x;
    });
    return ret;
  },

  consume: function(ev) {
    ev.preventDefault();
    ev.stopPropagation();
  },

  /*
   * returns [ar[0], sep, ar[1], sep, ..., sep, ar[ar.length-1]]
   * */
  leave: function(sep, ar) {
    var ret = [];
    ar.forEach(function(el) {
      ret.push(el);
      ret.push(sep);
    });
    ret.pop();
    return ret;
  },

  isobj: function(obj) {
    return typeof(obj) === 'object';
  },

  rot13: function(s) {
    return s.split('').map(function(ch) {
      if (ch == ' ') return ch;
      var c = ch.charCodeAt(0);
      var a = 'A'.charCodeAt(0);
      return String.fromCharCode((c - a + 13) % 26 + a);
    }).join('');
  },

  c: function(x) { return function() { return U.clone(x) } },

  nop: function() {},

  forEach: function(obj, fn) {
    Object.keys(obj).forEach(function(key) {
      fn(key, obj[key]);
    });
  },

  filter: function(obj, fn) {
    var ret = {};
    U.forEach(obj, function(a, b) {
      if (fn(a, b)) {
        ret[a] = b;
      }
    });
    return ret;
  },

  map: function(obj, fn) {
    return Object.keys(obj).map(function(key) {
      return fn(key, obj[key]);
    });
  },

  clone: function(obj) {
    if (obj == undefined) return obj;
    return JSON.parse(JSON.stringify(obj));
  },

  values: function(obj) {
    var ret = [];
    for (var k in obj) { ret.push(obj[k]); }
    return ret;
  },
  range: function(x, y) {
    if (y == undefined) {
      y = x;
      x = 0;
    }
    var ret = [];
    for (var i = x; i < y; i += 1) {
      ret.push(i);
    }
    return ret;
  },

  flatten: function(matr) {
    var ret = [];
    matr.forEach(function(ar) { ret = ret.concat(ar) });
    return ret;
  },

  unique: function(ar) {
    var ret = true;
    ar.forEach(function(x, i) { ret = ret && ar.indexOf(x, i + 1) != -1 });
    return ret;
  },

  subset: function(a, b) {
    var ret = true;
    a.forEach(function(x) { ret = ret && b.indexOf(a) != -1 });
  },

  subtr: function(a, b) {
    var ret = [];
    a.forEach(function(x) { if (b.indexOf(x) == -1) ret.push(x) });
    return ret;
  },

  mconcat: function(a, b) {
    var n = Math.min(a.length, b.length), ret = [];
    for (var i = 0; i < n; i += 1) ret.push(a[i].concat(b[i]));
    return ret;
  },

  all: function(a) {
    var ret = true;
    a.forEach(function(x) { ret = ret && x; });
    return ret;
  },
  any: function(a) {
    var ret = false;
    a.forEach(function(x) { ret = ret || x; });
    return ret;
  }
};

Array.prototype.flatMap = function(lambda) { 
    return Array.prototype.concat.apply([], this.map(lambda)); 
};

Array.prototype.shuffle = function() {
    var j, x, i;
    for (i = this.length; i; i--) {
        j = Math.floor(Math.random() * i);
        x = this[i - 1];
        this[i - 1] = this[j];
        this[j] = x;
    }
}

Array.prototype.take = function(indices) {
  return indices.map(function(idx) {
    return this[idx];
  }.bind(this));
}

Array.prototype.contains = function(el) {
  return this.indexOf(el) != -1;
};

Array.prototype.remove = function(el) {
  if (this.indexOf(el) != -1) {
    this.splice(this.indexOf(el), 1);
  }
};

var range = U.range;
function opp(color) {
  return color == 'white' ? 'black' : 'white';
}


