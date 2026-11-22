var api = { layoutGraph: layoutGraph, replayRun: replayRun, logLines: logLines, countState: countState };
if (typeof globalThis !== 'undefined') globalThis.__VIEWER_LOGIC__ = api;

/* ---------------- UI ---------------- */
if (typeof document === 'undefined') return;
