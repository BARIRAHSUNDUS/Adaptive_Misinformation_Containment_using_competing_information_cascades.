function buildPanel(idx){
  var el = document.createElement('div'); el.className = 'panel';
  var opts = runs.map(function(r, i){ return '<option value="' + i + '"' + (i === panels[idx].run ? ' selected' : '') + '>' + r.name + '</option>'; }).join('');
  el.innerHTML = '<div class="panelHead"><label>Strategy <select class="sel">' + opts + '</select></label><span class="muted why"></span></div>' +
    '<div class="stats"><span class="sReach" title="Users who EVER believed the rumour. This is the score: lower is better.">Reached by rumour (score): <b class="vReach">0</b></span>' +
    '<span class="sMis" title="Users who believe the rumour at this moment. It falls to 0 at the end because everybody is corrected in the end; it does not show the damage.">Believing it right now: <b class="vMis">0</b></span>' +
    '<span class="sFact">Fact-checked: <b class="vFact">0</b></span></div>' +
    '<canvas class="net"></canvas><canvas class="chart"></canvas><div class="log"></div>';
  $('panels').appendChild(el);
  var p = panels[idx];
  p.el = el; p.net = el.querySelector('canvas.net'); p.chart = el.querySelector('canvas.chart'); p.logEl = el.querySelector('.log');
  p.vMis = el.querySelector('.vMis'); p.vReach = el.querySelector('.vReach'); p.vFact = el.querySelector('.vFact');
  p.sel = el.querySelector('.sel'); p.why = el.querySelector('.why');
  p.sel.addEventListener('change', function(){ p.run = parseInt(p.sel.value, 10); p.lastLogTick = -1; draw(); });
  p.net.addEventListener('mousemove', function(ev){ hover(p, ev); });
  p.net.addEventListener('mouseleave', function(){ $('tip').style.display = 'none'; });
  p.lastLogTick = -1;
}

function sizeCanvas(c){
  var dpr = window.devicePixelRatio || 1, w = c.clientWidth || 400, h = c.clientHeight || 300;
  if (c.width !== Math.round(w * dpr) || c.height !== Math.round(h * dpr)){ c.width = Math.round(w * dpr); c.height = Math.round(h * dpr); }
  var ctx = c.getContext('2d'); ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  return { ctx: ctx, w: w, h: h };
}
function ease(t){ return t < 0 ? 0 : t > 1 ? 1 : t * t * (3 - 2 * t); }
function nodeRadius(i){ var f = Math.max(0.5, Math.min(1.5, Math.sqrt(120 / Math.max(n, 1)))); return Math.max(2.6, Math.min(11, (2.6 + Math.sqrt(deg[i]) * 1.15) * f)); }

function stateView(runIdx, k, phase){
  var R = replays[runIdx];
  var base = k > 0 ? R.pick[k - 1] : null;
  var st = phase >= PICK_AT ? R.pick[k] : phase >= SPREAD_AT ? R.spread[k] : base;
  var ever = phase >= SPREAD_AT ? R.everSpread[k] : (k > 0 ? R.ever[k - 1] : 0);
  return { st: st || new Uint8Array(n), ever: ever };
}

function px(p, w, h, i){ var pad = 16; return [pad + pos.x[i] * (w - 2 * pad), pad + pos.y[i] * (h - 2 * pad)]; }

