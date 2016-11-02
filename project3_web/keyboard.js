/*
  usage:
  ctrlProp = m.prop(0);
  keyboard = Keyboard({ CtrlKey: ctrlProp })
  options = {
    onkeyup: keyboard.up,
    onkeydown: keyboard.down,
  }
 */

Keyboard = function(keys) {
  var that = {};
  function make(val) { // make a event handler bound to val
    return function(ev) {
      if (ev.key in keys) {
        keys[ev.key](val); // call the callback
      }
    };
  }

  that.up = make(0);
  that.down = make(1);

  that.addKey = function(key, prop) {
    keys[key] = prop;
  };
  return that;
};
