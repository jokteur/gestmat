#include "main_window.h"

#include "style.h"
#include "core/item_manager.h"

#include "ui/widgets/modal.h"

#include "imgui_internal.h"

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
    // m_workspace.setCompression(true);
    // m_workspace.save("transfert_donnees_ancien", m_workspace.getCurrentManager());
}

void MainApp::FrameUpdate() {
#ifdef IMGUI_HAS_VIEWPORT
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
#else 
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif
    ImGui::Begin("Main window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

    m_menubar->FrameUpdate();
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
    case UIState::ALERTES:
        m_alerts->FrameUpdate();
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
    case UIState::ALERTES:
        m_alerts->BeforeFrameUpdate();
        break;
    }
}