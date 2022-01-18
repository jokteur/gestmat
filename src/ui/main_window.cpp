#include "main_window.h"

#include "style.h"
#include "core/item_manager.h"

void MainApp::InitializationBeforeLoop() {
    m_font_regular = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Regular.ttf", 18).value();
    m_font_italic = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Italic.ttf", 18).value();
    m_font_bold = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Bold.ttf", 18).value();
    m_font_title = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Regular.ttf", 30).value();

    setLightStyle();
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

    if (ImGui::Button("Click me")) {
        core::Item::Property property = { .name = "lolilol" };

        std::cout << property.id << std::endl;
    }
    ImGui::End();

}
void MainApp::BeforeFrameUpdate() {}