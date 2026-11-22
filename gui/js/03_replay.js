function countState(arr, s){ var c = 0; for (var i = 0; i < arr.length; i++) if (arr[i] === s) c++; return c; }

// Replays the recorded events. Fact-check is applied first (it wins conflicts), then misinformation, then the moderator.
function replayRun(run, n){
  var cur = new Uint8Array(n), ever = 0, out = { spread: [], pick: [], everSpread: [], misNow: [], fact: [], ever: [] };
  for (var k = 0; k < run.ticks.length; k++){
    var ev = run.ticks[k], i;
    for (i = 0; i < ev.fact.length; i++) cur[ev.fact[i][0]] = FACT;
    for (i = 0; i < ev.mis.length; i++){ cur[ev.mis[i][0]] = MIS; ever++; }
    out.spread.push(cur.slice()); out.everSpread.push(ever);
    for (i = 0; i < ev.picks.length; i++) cur[ev.picks[i].u] = FACT;
    out.pick.push(cur.slice());
    out.misNow.push(countState(cur, MIS)); out.fact.push(countState(cur, FACT)); out.ever.push(ever);
  }
  return out;
}

function logLines(run, name){
  var lines = [];
  for (var k = 0; k < run.ticks.length; k++){
    var ev = run.ticks[k], parts = [];
    if (k === 0) parts.push({ t: 'Tick 0: the rumour starts at User ' + (ev.mis.length ? ev.mis[0][0] : '?') });
    else parts.push({ t: 'Tick ' + k + ': +' + ev.mis.length + ' misinformed, +' + ev.fact.length + ' corrected' });
    for (var i = 0; i < ev.picks.length; i++){
      var p = ev.picks[i], sc = '';
      for (var j = 0; j < p.s.length; j++) if (p.s[j][0] === p.u) sc = ', score ' + p.s[j][1].toFixed(2);
      parts.push({ t: '   moderator fact-checks User ' + p.u + ' (' + p.c + ' eligible' + sc + ')', mod: true });
    }
    if (ev.skipped && !(k > 0 && run.ticks[k - 1].skipped)) parts.push({ t: '   moderator idle from here on: a fact-check can no longer change the outcome', mod: false });   // said once
    lines.push({ tick: k, parts: parts });
  }
  return lines;
}

