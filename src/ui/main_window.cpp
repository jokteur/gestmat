#include "main_window.h"

#include "style.h"
#include "core/item_manager.h"

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
    m_workspace.loadIntoCurrent(last.path);
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

    switch (ui_state->active_panel) {
    case UIState::LOAN:
        m_loans->FrameUpdate();
        break;
    case UIState::MANAGEMENT:
        m_management->FrameUpdate();
        break;
    case UIState::STATE:
        m_state->FrameUpdate();
        break;
    }
    ImGui::End();
}
void MainApp::BeforeFrameUpdate() {
}