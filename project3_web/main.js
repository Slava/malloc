var frames = dump.map(function(obj) {
  var ret = [];
  var curpos = 0;
  obj.frees.sort(function(a, b) {
    return a.position - b.position;
  });
  obj.frees.forEach(function(blockObj) {
    ret.push({
      free: false,
      width: blockObj.position - curpos,
    });
    ret.push({
      free: true,
      width: blockObj.width,
    });
    curpos = blockObj.position + blockObj.width;
  });

  ret.push({
    free: false,
    width: obj.total_width - curpos,
  });

  return ret;
});

window.setFrame = function () {}

document.getElementById('mySelect').innerHTML = dump.map(function (obj) {
  return '<option onclick="window.setFrame(mySelect.selectedIndex)">' + obj.orig + '</option>';
}).join('\n');

var Main = {
  controller: function() {
    console.trace('creating controller');
    var frame = 0;
    window.setFrame = function (i) {
      frame = i;
      m.redraw(true);
    }
    var resolution = 800;
    var fixedResolution = m.prop(false);
    function computeResolution(blocks) {
      var totalWidth = 0;
      blocks.forEach(function(block) {
        totalWidth += block.width;
      });
      var opt = Math.floor(totalWidth / 200) + 1;
      var rgran = 4;
      var rnd = Math.pow(rgran, Math.floor(Math.log(opt) / Math.log(rgran)));
      return rnd;
    }

    this.blocks = function() {
      console.log('fixedResolution', fixedResolution());
      if (!fixedResolution()) {
        resolution = computeResolution(frames[frame]);
      }
      return frames[frame];
    };

    var nextFrame = function() {
      console.trace('nextFrame');
      if (frame + 1 < frames.length) {
        frame++;
      }
    };

    var prevFrame = function() {
      console.trace('prevFrame');
      if (frame - 1 >= 0) {
        frame--;
      }
    };

    window.nextFrame = nextFrame;
    window.prevFrame = prevFrame;
    this.nextFrame = nextFrame;
    this.prevFrame = prevFrame;
    this.frame = function() {
      return frame;
    };
    this.resolution = function() {
      return resolution;
    };
    this.fixedResolution = fixedResolution;
  },

  view: function(ctrl) {
    function drawByte(free) {
      return m('.byte', {
        className: free ? 'green' : 'red',
      });
    }

    function drawBlock(block) {
      var bytes = Math.floor(block.width / resolution);
      return U.range(bytes).map(function() {
        return drawByte(block.free);
      });
    }

    var blocks = ctrl.blocks();
    var resolution = ctrl.resolution();
    var allBytes = [];
    blocks.forEach(function(block) {
      allBytes = allBytes.concat(drawBlock(block));
    });

    return m('.root', [
      m('div', [
        m('button', { onclick: ctrl.prevFrame, }, '<<<<'),
        m('button', { onclick: ctrl.nextFrame, }, '>>>>'),
      ]),
      m('div', [
        m('span', 'Resolution: ' + resolution),
        m('label', '   fixed'),
        m('input[type=checkbox]', {
          checked: ctrl.fixedResolution(),
          onclick: m.withAttr('checked', ctrl.fixedResolution),
        }),
      ]),
      m('bytes', allBytes),
    ]);
  },
}
