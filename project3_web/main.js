var mydump = dump;
//var full = false;
var full = true;
if (!full) {
  mydump = mydump.slice(0, 10);
}
var frames = mydump.map(function(obj) {
  var blocks = [];
  var curpos = 0;
  obj.frees.sort(function(a, b) {
    return a.position - b.position;
  });
  obj.frees.forEach(function(blockObj) {
    blocks.push({
      free: false,
      width: blockObj.position - curpos,
    });
    blocks.push({
      free: true,
      width: blockObj.width,
    });
    curpos = blockObj.position + blockObj.width;
  });

  blocks.push({
    free: false,
    width: obj.total_width - curpos,
  });

  var ret = {};
  ret.blocks = blocks;
  ret.orig = obj.orig;
  return ret;
});

window.setFrame = function () {}

var Main = {
  controller: function() {
    var resolution = 800;
    var fixedResolution = m.prop(false);
    var frame = m.prop(0);
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

    var getBlocks = function() {
      if (!fixedResolution()) {
        resolution = computeResolution(frames[frame()].blocks);
      }
      return frames[frame()].blocks;
    };

    var nextFrame = function() {
      if (frame() + 1 < frames.length) {
        frame(frame() + 1);
      }
    };

    var prevFrame = function() {
      if (frame() - 1 >= 0) {
        frame(frame() - 1 );
      }
    };

    window.nextFrame = nextFrame;
    window.prevFrame = prevFrame;
    this.nextFrame = nextFrame;
    this.prevFrame = prevFrame;
    this.blocks = getBlocks;
    this.frame = frame;
    this.resolution = function() {
      return resolution;
    };
    this.fixedResolution = fixedResolution;
    this.keyboard = Keyboard({
      'ArrowLeft': prevFrame,
      'ArrowRight': nextFrame,
    });
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

    return m('.root[tabindex=1', {
      config: function(el) {
        el.focus();
      },
      onblur: function() {
        this.focus();
      },
      onkeyup: ctrl.keyboard.up,
      onkeydown: ctrl.keyboard.down,
    }, [
      m('select[size=20]',{
        selectedIndex: ctrl.frame(),
        onchange: function() {
          ctrl.frame(this.selectedIndex);
        },
        //onchange: m.withAttr('selectedIndex', ctrl.frame),
      }, frames.map(function(obj, i) {
        return m('option', {
          onclick: function() { ctrl.frame(i); }
        } , obj.orig);
      })),
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
