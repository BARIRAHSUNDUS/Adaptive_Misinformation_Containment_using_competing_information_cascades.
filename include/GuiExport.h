#ifndef GUIEXPORT_H
#define GUIEXPORT_H
#include <ostream>
#include <string>
#include "Config.h"

// JSON for the viewer: the scenario (graph, source, probabilities) and the full event trace of
// "No Intervention" + the four strategies, all run on that SAME scenario by the normal engine.
// `benchmarkText` is shown in the viewer's benchmark section.
std::string buildViewerJson(const SimulationConfig& cfg, const std::string& benchmarkText);
// The HTML GUI page for the local server: the same viewer, with the setup form filled with `defaults`.
std::string viewerHtmlForServer(const SimulationConfig& defaults);
// Opens a file / URL with the system's default program (the browser for .html). Returns false if it failed.
bool openInBrowser(const std::string& path);
#endif