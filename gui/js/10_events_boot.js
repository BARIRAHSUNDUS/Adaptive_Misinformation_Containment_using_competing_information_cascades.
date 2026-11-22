bindBtn('btnPlay', function(){ if (!D) return; if (!playing && cursor >= lenTicks - 0.01){ cursor = 0; draw(); } setPlaying(!playing); });
bindBtn('btnReset', function(){ setPlaying(false); cursor = 0; draw(); });
bindBtn('btnFwd', function(){ setPlaying(false); var k = Math.floor(cursor), ph = cursor - k; goTick(ph < 0.99 ? k : k + 1); });
bindBtn('btnBack', function(){ setPlaying(false); goTick(Math.floor(cursor) - 1); });
$('tickSlider').addEventListener('input', function(){ setPlaying(false); goTick(parseInt($('tickSlider').value, 10)); });
$('speed').addEventListener('change', function(){ speed = parseFloat($('speed').value); });
document.addEventListener('keydown', function(ev){
  var tg = ev.target && ev.target.tagName;
  if (tg === 'SELECT' || tg === 'INPUT' || tg === 'TEXTAREA' || tg === 'BUTTON') return;
  if (ev.key === ' '){ ev.preventDefault(); $('btnPlay').click(); }
  else if (ev.key === 'ArrowRight') $('btnFwd').click();
  else if (ev.key === 'ArrowLeft') $('btnBack').click();
});
window.addEventListener('resize', function(){ draw(); });
$('sumTable').addEventListener('click', function(ev){ var b = ev.target.closest ? ev.target.closest('button') : null; if (!b || !D) return;
  panels[1].run = parseInt(b.getAttribute('data-run'), 10); panels[1].sel.value = String(panels[1].run); panels[1].lastLogTick = -1; draw(); });

if (DEFAULTS){                                   // started through misinfo --gui
  $('setup').hidden = false;
  $('fUsers').value = DEFAULTS.users; $('fTicks').value = DEFAULTS.ticks; $('fBudget').value = DEFAULTS.budget; $('fWarmup').value = DEFAULTS.warmup;
  $('fGraph').value = DEFAULTS.graph; $('fSeed').value = DEFAULTS.seed;
  bindBtn('btnRun', runSim);
  bindBtn('btnNew', function(){ $('fSeed').value = Math.floor(Math.random() * 1000000000); runSim(); });
  ['fUsers', 'fTicks', 'fBudget', 'fWarmup', 'fSeed'].forEach(function(id){ $(id).addEventListener('keydown', function(ev){ if (ev.key === 'Enter') runSim(); }); });
  runSim();
} else {
  $('nodata').hidden = false;
}
requestAnimationFrame(frame);
