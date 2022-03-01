#include <fstream>

#include "main_window.h"
#include "python/py_api.h"

#include "style.h"

#include "core/item_manager.h"
#include "python/with.h"

#include "ui/widgets/modal.h"
#include "core/util.h"

#include "implot.h"
#include "imgui_internal.h"

namespace py = pybind11;
void MainApp::InitializationBeforeLoop() {
    ui_state->font_regular = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Regular.ttf", 18).value();
    ui_state->font_italic = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Italic.ttf", 18).value();
    ui_state->font_bold = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Bold.ttf", 18).value();
    ui_state->font_title = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Regular.ttf", 30).value();

    setLightStyle();

    auto files = m_workspace.getCompatibleFiles();
    core::Item::File last;
    for (const auto& file : files) {
        last = file;
    }
    // m_workspace.setCompression(false);
    // std::cout << last.path << std::endl;
    if (last.path.empty())
        m_workspace.setCurrentManager(std::make_shared<Item::Manager>());
    else
        m_open_error = m_workspace.loadIntoCurrent(last.path);

    ImPlot::CreateContext();
    // m_workspace.setCompression(false);
    // m_workspace.save("non_compressed", m_workspace.getCurrentManager());
    // m_workspace.setCompression(true);
}

void MainApp::AfterLoop() {
    ImPlot::DestroyContext();
}

void MainApp::preload() {
    if (!m_pandas_loaded) {
        Tempo::jobFct job = [this](float&, bool&) -> std::shared_ptr<Tempo::JobResult> {
            Tempo::JobResult job_result;

            std::string err;
            auto state = PyGILState_Ensure();
            try {
                auto pd = py::module::import("pandas");

                py::dict data, kwargs;
                py::list list;
                list.append(1);
                data["1"] = list;
                kwargs["data"] = data;
                auto df = pd.attr("DataFrame")(**kwargs);

                py::with(pd.attr("ExcelWriter")("test.xlsx"), [&df](py::object writer) {
                    df.attr("to_excel")(writer);
                    });
            }
            catch (const std::exception& e) {
                m_pandas_error = e.what();
                std::cout << e.what() << std::endl;
            }

            PyGILState_Release(state);

            // Wake the usb key up
            try {
                std::ofstream myfile;
                myfile.open("test.txt");
                myfile << "test\n";
                myfile.close();
            }
            catch (const std::exception&) {

            }

            return std::make_shared<Tempo::JobResult>(job_result);
        };
        Tempo::JobScheduler::getInstance().addJob("load_pandas", job);
        m_pandas_loaded = true;
        return;
    }
}

void MainApp::FrameUpdate() {
    preload();

#ifdef IMGUI_HAS_VIEWPORT
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
#else 
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif
    ImGui::Begin("Main window", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoBringToFrontOnFocus);

    m_menubar->FrameUpdate();
    m_menubar->DrawHistory();
    m_navbar->FrameUpdate();

    if (!m_open_error.empty()) {
        ImGui::Text(m_open_error.c_str());
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 20);
    ImGui::BeginChild("Panel window");

    switch (ui_state->active_panel) {
    case UIState::LOAN:
        m_loans->FrameUpdate();
        break;
    case UIState::PROPERTIES:
        m_properties->FrameUpdate();
        break;
    case UIState::MANAGEMENT:
        m_management->FrameUpdate();
        break;
    case UIState::STATE:
        m_state->FrameUpdate();
        break;
    case UIState::STATS:
        m_stats->FrameUpdate();
        break;
    }
    // if (ImGui::Button("Show demo")) {
    //     m_open = true;
    //     ui_state->read_only = !ui_state->read_only;
    // }


    ImGui::EndChild();
    ImGui::PopStyleVar();
    Modals::getInstance().FrameUpdate();
    ImGui::End();
    if (m_open)
        ImGui::ShowDemoWindow(&m_open);
}
void MainApp::BeforeFrameUpdate() {
    m_menubar->BeforeFrameUpdate();
    switch (ui_state->active_panel) {
    case UIState::LOAN:
        m_loans->BeforeFrameUpdate();
        break;
    case UIState::PROPERTIES:
        m_properties->BeforeFrameUpdate();
        break;
    case UIState::MANAGEMENT:
        m_management->BeforeFrameUpdate();
        break;
    case UIState::STATE:
        m_state->BeforeFrameUpdate();
        break;
    case UIState::STATS:
        m_stats->BeforeFrameUpdate();
        break;
    }
}