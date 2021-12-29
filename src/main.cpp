#include <iostream>

#include "GLFWwindow_handler.h"
#include "app/settings.h"
#include "python/init_python.h"
#include "python/py_api.h"
#include "rendering/gui.h"
#include "core/util.h"

int main(int, char**) {
    PyAPI::Handler::getInstance();
    std::cout << "date formating: " << core::getCurrentDate().format("%d/%m/%Y") << std::endl;
    std::cout << "date formating: " << core::Date(29, 04, 1995).format("%m-%Y") << std::endl;

    GLFWwindowHandler::focus_all = true;

    // Test a simple initialization with an empty window
    Rendering::Application app("App template", 1280, 720);

    app.addImGuiFlags(ImGuiConfigFlags_ViewportsEnable | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard);

    app.init();
    PyAPI::init();

    Rendering::GUI::getInstance().init(app);

    app.loop();
    return 0;
}
