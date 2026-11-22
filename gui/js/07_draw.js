function drawNet(p, k, phase){
  var o = sizeCanvas(p.net), ctx = o.ctx, w = o.w, h = o.h, C = colors(), run = runs[p.run], ev = run.ticks[k];
  ctx.clearRect(0, 0, w, h);
  ctx.lineWidth = 1; ctx.strokeStyle = C.edge; ctx.beginPath();
  var i, e, a, b;
  for (e = 0; e < edges.length; e++){ a = px(p, w, h, edges[e][0]); b = px(p, w, h, edges[e][1]); ctx.moveTo(a[0], a[1]); ctx.lineTo(b[0], b[1]); }
  ctx.stroke();
  var sv = stateView(p.run, k, phase), st = sv.st;
  // transmissions of this tick: highlighted edges, travelling dots while phase < SPREAD_AT
  var travel = ease(phase / SPREAD_AT), fade = phase < SPREAD_AT ? 1 : Math.max(0, 1 - (phase - SPREAD_AT) / (1 - SPREAD_AT));
  function drawTrans(list, col){
    ctx.strokeStyle = col; ctx.fillStyle = col; ctx.lineWidth = 2;
    for (var t = 0; t < list.length; t++){
      if (list[t][1] < 0) continue;
      var f = px(p, w, h, list[t][1]), to = px(p, w, h, list[t][0]);
      ctx.globalAlpha = 0.85 * fade; ctx.beginPath(); ctx.moveTo(f[0], f[1]); ctx.lineTo(to[0], to[1]); ctx.stroke();
      if (phase < SPREAD_AT){ ctx.globalAlpha = 1; ctx.beginPath(); ctx.arc(f[0] + (to[0] - f[0]) * travel, f[1] + (to[1] - f[1]) * travel, 3.4, 0, 6.2832); ctx.fill(); }
    }
    ctx.globalAlpha = 1;
  }
  drawTrans(ev.mis, C.mis); drawTrans(ev.fact, C.fact);
  // nodes
  var col = [C.neutral, C.mis, C.fact];
  for (i = 0; i < n; i++){
    var q = px(p, w, h, i), r = nodeRadius(i);
    ctx.fillStyle = col[st[i]]; ctx.beginPath(); ctx.arc(q[0], q[1], r, 0, 6.2832); ctx.fill();
    if (pickedAt[p.run][i] >= 0 && (pickedAt[p.run][i] < k || (pickedAt[p.run][i] === k && phase >= PICK_AT))){
      ctx.strokeStyle = C.mod; ctx.lineWidth = 3; ctx.beginPath(); ctx.arc(q[0], q[1], r + 2.5, 0, 6.2832); ctx.stroke();
    }
    if (i === D.meta.source){ ctx.strokeStyle = C.src; ctx.lineWidth = 2; ctx.beginPath(); ctx.arc(q[0], q[1], r + 5, 0, 6.2832); ctx.stroke(); }
  }
  // pulse on the users that changed in this tick
  if (phase >= SPREAD_AT){
    var pr = (phase - SPREAD_AT) / (1 - SPREAD_AT);
    var pulse = function(list, c){ ctx.strokeStyle = c; ctx.lineWidth = 2; for (var t = 0; t < list.length; t++){ var q = px(p, w, h, list[t][0]);
      ctx.globalAlpha = 0.7 * (1 - pr); ctx.beginPath(); ctx.arc(q[0], q[1], nodeRadius(list[t][0]) + 3 + 9 * pr, 0, 6.2832); ctx.stroke(); } ctx.globalAlpha = 1; };
    pulse(ev.mis, C.mis); pulse(ev.fact, C.fact);
  }
  if (phase >= PICK_AT){
    var pp = (phase - PICK_AT) / (1 - PICK_AT);
    ev.picks.forEach(function(pk){ var q = px(p, w, h, pk.u); ctx.strokeStyle = C.mod; ctx.lineWidth = 3; ctx.globalAlpha = 1 - pp;
      ctx.beginPath(); ctx.arc(q[0], q[1], nodeRadius(pk.u) + 6 + 14 * pp, 0, 6.2832); ctx.stroke(); ctx.globalAlpha = 1; });
  }
  // counters
  p.vMis.textContent = countState(st, MIS); p.vFact.textContent = countState(st, FACT); p.vReach.textContent = sv.ever;
  return sv;
}

function drawChart(p, k){
  var o = sizeCanvas(p.chart), ctx = o.ctx, w = o.w, h = o.h, C = colors(), R = replays[p.run], padL = 28, padB = 14, padT = 6;
  ctx.clearRect(0, 0, w, h);
  var ymax = Math.max(n, 1), T = lenTicks - 1 || 1;
  var X = function(t){ return padL + t / T * (w - padL - 6); }, Y = function(v){ return padT + (1 - v / ymax) * (h - padT - padB); };
  ctx.strokeStyle = C.border; ctx.lineWidth = 1; ctx.beginPath(); ctx.moveTo(padL, Y(0)); ctx.lineTo(w - 6, Y(0)); ctx.moveTo(padL, Y(0)); ctx.lineTo(padL, padT); ctx.stroke();
  ctx.fillStyle = C.text; ctx.font = '11px system-ui,sans-serif'; ctx.textBaseline = 'middle'; ctx.textAlign = 'right';
  ctx.fillText(String(ymax), padL - 4, Y(ymax) + 3); ctx.fillText('0', padL - 4, Y(0) - 3);
  ctx.textAlign = 'center'; ctx.textBaseline = 'alphabetic'; ctx.fillText('tick', w / 2, h - 1);
  function line(series, col, upto){
    ctx.strokeStyle = col; ctx.lineWidth = 2; ctx.beginPath();
    for (var t = 0; t <= upto; t++){ var xx = X(t), yy = Y(series[t]); if (t === 0) ctx.moveTo(xx, yy); else ctx.lineTo(xx, yy); }
    ctx.stroke();
  }
  ctx.globalAlpha = 0.25; line(R.ever, C.mis, lenTicks - 1); line(R.fact, C.fact, lenTicks - 1); line(R.misNow, C.mis, lenTicks - 1); ctx.globalAlpha = 1;
  line(R.ever, C.neutral, k); line(R.misNow, C.mis, k); line(R.fact, C.fact, k);
  ctx.strokeStyle = C.text; ctx.lineWidth = 1; ctx.setLineDash([3, 3]); ctx.beginPath(); ctx.moveTo(X(k), padT); ctx.lineTo(X(k), Y(0)); ctx.stroke(); ctx.setLineDash([]);
  ctx.textAlign = 'left'; ctx.fillStyle = C.text; ctx.fillText('grey: ever reached  red: misinformed now  green: fact-checked', padL + 6, padT + 10);
}

function drawLog(p, k){
  if (p.lastLogTick === k) return; p.lastLogTick = k;
  p.logEl.textContent = '';
  for (var t = k; t >= 0; t--) logs[p.run][t].parts.forEach(function(part){
    var d = document.createElement('div'); if (part.mod) d.className = 'mod'; d.textContent = part.t; p.logEl.appendChild(d); });
  p.logEl.scrollTop = 0;
}

function hover(p, ev){
  var rect = p.net.getBoundingClientRect(), mx = ev.clientX - rect.left, my = ev.clientY - rect.top, best = -1, bd = 1e9, k = Math.floor(cursor), ph = cursor - k;
  for (var i = 0; i < n; i++){ var q = px(p, rect.width, rect.height, i), d = Math.hypot(q[0] - mx, q[1] - my); if (d < bd){ bd = d; best = i; } }
  var tip = $('tip');
  if (best >= 0 && bd < nodeRadius(best) + 5){
    var s = stateView(p.run, k, ph).st[best];
    tip.textContent = 'User ' + best + ' \u00b7 ' + deg[best] + ' connections \u00b7 ' + ['neutral', 'misinformed', 'fact-checked'][s] + (best === D.meta.source ? ' \u00b7 source' : '') + (pickedAt[p.run][best] >= 0 ? ' \u00b7 moderator pick (tick ' + pickedAt[p.run][best] + ')' : '');
    tip.style.left = (ev.clientX + 12) + 'px'; tip.style.top = (ev.clientY + 12) + 'px'; tip.style.display = 'block';
  } else tip.style.display = 'none';
}

function draw(){
  if (!D) return;
  var k = Math.min(lenTicks - 1, Math.floor(cursor)), phase = cursor - k;
  var svs = panels.map(function(p){ var sv = drawNet(p, k, phase); drawChart(p, k); drawLog(p, k);
    p.why.textContent = p.run === 0 ? 'nobody fact-checks' : ''; return sv; });
  $('tickSlider').value = k; $('tickLabel').textContent = 'Tick ' + k + ' / ' + (lenTicks - 1);
  var a = panels[0], b = panels[1], ra = svs[0].ever, rb = svs[1].ever, diff = ra - rb;
  $('delta').textContent = 'Tick ' + k + ':  ' + runs[a.run].name + ' reached ' + ra + ' users,  ' + runs[b.run].name + ' reached ' + rb + ' users' +
    (a.run === b.run ? '' : diff > 0 ? '  \u2192  ' + runs[b.run].name + ' is ' + diff + ' users ahead' : diff < 0 ? '  \u2192  ' + runs[a.run].name + ' is ' + (-diff) + ' users ahead' : '  \u2192  equal');
}

