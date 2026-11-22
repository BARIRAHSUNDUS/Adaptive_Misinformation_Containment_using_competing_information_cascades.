/* ---------------- pure logic (also used by the tests) ---------------- */
function mulberry32(a){ return function(){ a |= 0; a = a + 0x6D2B79F5 | 0; var t = Math.imul(a ^ a >>> 15, 1 | a);
  t = t + Math.imul(t ^ t >>> 7, 61 | t) ^ t; return ((t ^ t >>> 14) >>> 0) / 4294967296; }; }

// Fruchterman-Reingold layout. Returns coordinates scaled into [0,1] x [0,1].
function layoutGraph(n, edges){
  var x = new Float64Array(n), y = new Float64Array(n);
  var rnd = mulberry32(20240607), i, j, it;
  for (i = 0; i < n; i++){ var a = rnd() * 6.283185307, r = Math.sqrt(rnd()); x[i] = Math.cos(a) * r; y[i] = Math.sin(a) * r; }
  if (n > 1){
    var k = Math.sqrt(4 / n) * 0.9, temp = 0.35;
    var iters = n <= 150 ? 420 : n <= 400 ? 260 : n <= 800 ? 140 : 70;
    var dx = new Float64Array(n), dy = new Float64Array(n);
    for (it = 0; it < iters; it++){
      dx.fill(0); dy.fill(0);
      for (i = 0; i < n; i++) for (j = i + 1; j < n; j++){
        var ex = x[i] - x[j], ey = y[i] - y[j], d2 = ex * ex + ey * ey + 1e-9, d = Math.sqrt(d2);
        var f = k * k / d2;                       // (k^2/d) / d  -> unit vector * k^2/d
        dx[i] += ex * f; dy[i] += ey * f; dx[j] -= ex * f; dy[j] -= ey * f;
      }
      for (var e = 0; e < edges.length; e++){
        var p = edges[e][0], q = edges[e][1];
        var fx = x[p] - x[q], fy = y[p] - y[q], dd = Math.sqrt(fx * fx + fy * fy) + 1e-9;
        var g = dd / k;                           // (d^2/k) / d
        dx[p] -= fx * g; dy[p] -= fy * g; dx[q] += fx * g; dy[q] += fy * g;
      }
      for (i = 0; i < n; i++){                   // weak gravity keeps separate components on screen
        dx[i] -= x[i] * k * 1.6; dy[i] -= y[i] * k * 1.6;
        var disp = Math.sqrt(dx[i] * dx[i] + dy[i] * dy[i]) + 1e-9, lim = Math.min(disp, temp);
        x[i] += dx[i] / disp * lim; y[i] += dy[i] / disp * lim;
      }
      temp = Math.max(temp * 0.985, 0.004);
    }
  }
  var minX = Infinity, maxX = -Infinity, minY = Infinity, maxY = -Infinity;
  for (i = 0; i < n; i++){ minX = Math.min(minX, x[i]); maxX = Math.max(maxX, x[i]); minY = Math.min(minY, y[i]); maxY = Math.max(maxY, y[i]); }
  var span = Math.max(maxX - minX, maxY - minY, 1e-6), cx = (minX + maxX) / 2, cy = (minY + maxY) / 2;
  for (i = 0; i < n; i++){ x[i] = 0.5 + (x[i] - cx) / span; y[i] = 0.5 + (y[i] - cy) / span; }
  return { x: x, y: y };
}

