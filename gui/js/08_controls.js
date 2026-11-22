function goTick(k){ if (!D) return; cursor = Math.max(0, Math.min(lenTicks - 1, k)) + 0.999; draw(); }
function setPlaying(v){ playing = v; $('btnPlay').innerHTML = v ? '&#10074;&#10074; Pause' : '&#9654; Play'; lastTs = 0; }
function frame(ts){
  if (playing && D){
    if (lastTs) cursor += Math.min(0.25, (ts - lastTs) / 1000) * speed;
    lastTs = ts;
    if (cursor >= lenTicks - 0.0005){ cursor = lenTicks - 0.0005; setPlaying(false); }
    draw();
  }
  requestAnimationFrame(frame);
}

function buildSummary(){
  var baseReach = replays[0].ever[lenTicks - 1], maxReach = Math.max.apply(null, runs.map(function(_, i){ return replays[i].ever[lenTicks - 1]; })) || 1;
  var bestIdx = 1, i;
  for (i = 2; i < runs.length; i++) if (replays[i].ever[lenTicks - 1] < replays[bestIdx].ever[lenTicks - 1]) bestIdx = i;
  var html = '<tr><th>Strategy</th><th>Users reached by the rumour</th><th>Saved vs no intervention</th><th>Fact-checked users</th><th>Interventions used</th><th></th></tr>';
  runs.forEach(function(r, i){
    var reach = replays[i].ever[lenTicks - 1], used = r.ticks.reduce(function(s, t){ return s + t.picks.length; }, 0);
    html += '<tr' + (i === bestIdx ? ' class="best"' : '') + '><td>' + r.name + (i === bestIdx ? ' (lowest in this scenario)' : '') + '</td><td><span class="bar" style="width:' + Math.round(reach / maxReach * 160) + 'px"></span>' + reach +
      '</td><td>' + (i === 0 ? '-' : (baseReach - reach)) + '</td><td>' + replays[i].fact[lenTicks - 1] + '</td><td>' + used + '</td><td><button data-run="' + i + '">show on right</button></td></tr>';
  });
  $('sumTable').innerHTML = html;
}

