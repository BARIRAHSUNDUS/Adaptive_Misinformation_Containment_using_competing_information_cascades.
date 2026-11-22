// (Re)loads a simulation: new graph, new events. Called after every "Run".
function loadData(data){
  D = data; var m = D.meta; n = m.users; edges = D.edges; runs = D.runs; lenTicks = runs[0].ticks.length;
  $('meta').textContent = m.users + ' users \u00b7 ' + edges.length + ' connections \u00b7 ' + m.graph + ' graph \u00b7 source User ' + m.source +
    ' \u00b7 misinformation spread p=' + m.misP + ' \u00b7 fact-check spread p=' + m.factP + ' \u00b7 moderator budget ' + m.budget +
    ' (acts once per tick from tick ' + (m.warmup + 1) + ', only while the rumour is still spreading) \u00b7 seed ' + m.seed;
  $('bench').textContent = D.bench || '';
  deg = new Int32Array(n);
  edges.forEach(function(e){ deg[e[0]]++; deg[e[1]]++; });
  pos = layoutGraph(n, edges);
  replays = runs.map(function(r){ return replayRun(r, n); });
  logs = runs.map(function(r){ return logLines(r, r.name); });
  pickedAt = runs.map(function(r){ var a = new Int32Array(n).fill(-1); r.ticks.forEach(function(t, k){ t.picks.forEach(function(p){ a[p.u] = k; }); }); return a; });
  $('panels').textContent = '';
  panels = [ { run: 0 }, { run: runs.length - 1 } ];       // left: no intervention, right: Marginal Gain
  buildPanel(0); buildPanel(1);
  $('tickSlider').max = lenTicks - 1;
  buildSummary();
  $('app').hidden = false;
  setPlaying(false); cursor = 0; draw();
}

/* ---- live mode: the form asks the C++ engine (through the local server) for a new simulation ---- */
function runSim(){
  var f = function(id){ return $(id).value; };
  var url = '/api/run?users=' + encodeURIComponent(f('fUsers')) + '&ticks=' + encodeURIComponent(f('fTicks')) + '&budget=' + encodeURIComponent(f('fBudget')) + '&warmup=' + encodeURIComponent(f('fWarmup')) +
            '&graph=' + encodeURIComponent(f('fGraph')) + '&seed=' + encodeURIComponent(f('fSeed')) + '&bench=' + ($('fBench').checked ? 1 : 0);
  var st = $('runStatus'); st.className = ''; st.textContent = 'Running the C++ engine...';
  $('btnRun').disabled = true; $('btnNew').disabled = true;
  var done = function(){ $('btnRun').disabled = false; $('btnNew').disabled = false; };
  fetch(url).then(function(r){ return r.json().then(function(j){ return { ok: r.ok, j: j }; }); })
    .then(function(res){ if (!res.ok) throw new Error(res.j && res.j.error ? res.j.error : 'the server refused the request'); loadData(res.j); st.textContent = ''; done(); })
    .catch(function(e){ st.className = 'err'; st.textContent = 'Error: ' + e.message; done(); });
}
function bindBtn(id, fn){ $(id).addEventListener('click', function(ev){ fn(ev); if (ev.currentTarget && ev.currentTarget.blur) ev.currentTarget.blur(); }); }

