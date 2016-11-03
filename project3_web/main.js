var mydump = dump;
var full = true;
//full = false;
if (!full) {
  mydump = mydump.slice(0, 10);
}
var frames = mydump.map(function(obj) {
  var blocks = [];
  var curpos = 0;

  var bs = [];
  bs = obj.frees;

  obj.bins.forEach(function (bin) {
    var chunk_size = bin.chunk_size;
    bin.allocated_pages.forEach(function (page) {
      bs.push({
        position: page.position,
        chunk_size: chunk_size,
        bitmap: page.bitmap,
      });
    });
  });

  bs.sort(function(a, b) {
    return a.position - b.position;
  });
  bs.forEach(function(blockObj) {
    blocks.push({
      tag: 'taken',
      width: blockObj.position - curpos,
    });
    if (blockObj.bitmap) {
      // header
      blocks.push({
        tag: 'page-header',
        width: 4 * 8,
      });
      curpos += 4 * 8;

      for (var i = 0; i < 64; i++) {
        if (blockObj.bitmap[i] === '1') {
          blocks.push({ tag: 'page-free', width: blockObj.chunk_size });
        } else {
          blocks.push({ tag: 'page-taken', width: blockObj.chunk_size });
        }
      }
      curpos += 64 * blockObj.chunk_size;
    } else {
      blocks.push({
        tag: 'free',
        width: blockObj.width,
      });
      curpos = blockObj.position + blockObj.width;
    }
  });

  blocks.push({
    tag: 'taken',
    width: obj.total_width - curpos,
  });

  var ret = {};
  ret.blocks = blocks;
  ret.orig = obj.orig;
  ret.total_width = obj.total_width;
  return ret;
});

var jumpframes = U.range(frames.length).filter(function(i) {
  return i == 0 || frames[i].total_width > frames[i - 1].total_width;
});

window.setFrame = function () {}

var Main = {
  controller: function() {
    var resolution = 800;
    var fixedResolution = m.prop(false);
    var frame = m.prop(0);
    function computeResolution(blocks) {
      return 2048 * 4;
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

    var jumpframe = function(idx) {
      if (idx === undefined) { // getter
        console.log('getter');
        var r = 0;
        jumpframes.forEach(function(i, j) {
          if (i <= frame()) {
            r = j;
          }
        });
        return r;
      } else { // setter
        frame(jumpframes[idx]);
      }
    };

    window.nextFrame = nextFrame;
    window.prevFrame = prevFrame;
    this.nextFrame = nextFrame;
    this.prevFrame = prevFrame;
    this.blocks = getBlocks;
    this.frame = frame;
    this.jumpframe = jumpframe;
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
    function drawByte(tag) {
      return m('.byte', {
        className: tag
      });
    }

    function drawBlock(block) {
      var bytes = Math.ceil(block.width / resolution);
      return U.range(bytes).map(function() {
        return drawByte(block.tag);
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

      m('select[size=20]',{
        selectedIndex: ctrl.jumpframe(),
        onchange: function() {
          ctrl.jumpframe(this.selectedIndex);
        },
        //onchange: m.withAttr('selectedIndex', ctrl.frame),
      }, jumpframes.map(function(i, j) {
        return m('option', {
          onclick: function() { ctrl.jumpframe(j); }
        } , frames[i].orig + '(' + frames[i].total_width + ')');
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
      m('div', [
        m('span', 'Total width: ' + frames[ctrl.frame()].total_width),
      ]),
      m('bytes', allBytes),
    ]);
  },
}
