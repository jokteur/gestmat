#include <iostream>
#include <tempo.h>
#include <chrono>

#include "ui/main_window.h"
#include "python/py_api.h"
#include "core/item_manager.h"
#include "core/workspace.h"
#include "core/compress.h"


using namespace core;
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
int main() {
    PyAPI::Handler::getInstance();

    Tempo::Config config{
        .app_name = "Gestmat",
        .app_title = "Gestion matériel",
    };
    config.imgui_config_flags = 0;

    MainApp* app = new MainApp();
    Tempo::Run(app, config);

    return 0;
}