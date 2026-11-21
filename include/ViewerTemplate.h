#ifndef VIEWERTEMPLATE_H
#define VIEWERTEMPLATE_H
#include <string>
// The HTML viewer page (gui/viewer_template.html) compiled into the program. It contains the
// placeholder  /*__DEFAULTS__*/null  that viewerHtmlForServer() replaces with the defaults of the setup form.
std::string viewerTemplate();
#endif