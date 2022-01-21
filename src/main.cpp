#include <iostream>
#include <tempo.h>
#include <chrono>

#include "ui/main_window.h"
#include "python/py_api.h"
#include "core/item_manager.h"
#include "core/workspace.h"
#include "core/compress.h"


using namespace core;
int main() {
    PyAPI::Handler::getInstance();

    Tempo::Config config{
        .app_name = "Gestmat",
        .app_title = "Gestion matériel",
    };

    Item::Workspace& workspace = Item::Workspace::getInstance();

    auto files = workspace.getCompatibleFiles();
    for (auto file : files) {
        std::cout << file.filename << " " << file.timestamp << " " << file.action_name << std::endl;
        workspace.loadIntoCurrent(file.path);
    }

    MainApp* app = new MainApp();
    Tempo::Run(app, config);

    return 0;
}