var $ = function(id){ return document.getElementById(id); };
var D = null, n = 0, edges = [], runs = [], lenTicks = 1, deg = null, pos = null, replays = [], logs = [], pickedAt = [];
var panels = [], cursor = 0, playing = false, lastTs = 0, speed = 0.8;   // cursor = tick + phase (0..1)

function colors(){
  var cs = getComputedStyle(document.documentElement), g = function(v){ return cs.getPropertyValue(v).trim() || '#888'; };
  return { neutral: g('--neutral'), mis: g('--mis'), fact: g('--fact'), mod: g('--mod'), edge: g('--edge'), src: g('--src'), text: g('--muted'), border: g('--border') };
}

