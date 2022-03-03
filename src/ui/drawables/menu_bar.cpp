#include "menu_bar.h"
#include "ui/widgets/modal.h"
// #include "nfd.h"

void MenuBar::BeforeFrameUpdate() {
    auto m_manager = m_workspace.getCurrentManager();
    if (m_manager != nullptr) {
        auto durations = core::getDurations();
        auto duration = m_manager->getDuration();
        m_selected_duration[duration] = true;
        bool set_to_false = false;
        for (auto pair : durations) {
            if (m_selected_duration[pair.first]) {
                if (duration != pair.first) {
                    set_to_false = true;
                    m_manager->setDuration(pair.first);
                    continue;
                }
            }
        }
        if (set_to_false)
            m_selected_duration[duration] = false;
    }
}

void MenuBar::FrameUpdate() {
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("Fichiers")) {
        if (ImGui::MenuItem("Exporter données", 0, nullptr, !m_ui_state->restauration)) {
            bool success = m_workspace.exportToDesktop();
            std::string message;
            if (success) {
                message = "Les données (emprunt et liste matériel) ont été exportées sur cet ordinateur.\n"
                    "\n"
                    "Vous trouverez les données dans un fichier excel sur le bureau\n"
                    "dans le dossier 'gestion_materiel' préfacé avec la date d'aujourd'hui.";
            }
            else {
                message = "Le logiciel n'a pas réussi à exporter les données.\n\n"
                    "Néanmoins, vous trouverez quand même les dernières données dans \n"
                    "le dossier 'Documents' puis 'sauvegardes_matgest' et finalement 'excel'";
                m_workspace.save("export_data");
            }

            const modal_fct fct = [message](bool& show, bool&, bool&) {
                ImGui::Text(message.c_str());
                if (ImGui::Button("Ok")) {
                    show = false;
                }
            };
            Modals::getInstance().setModal("Export des données", fct);
        }
        if (ImGui::MenuItem("Restaurer des données depuis cet ordinateur", 0, nullptr, !m_ui_state->restauration)) {
            m_restore_window.show();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Durée alertes")) {
        auto m_manager = m_workspace.getCurrentManager();
        if (m_manager != nullptr) {
            auto durations = core::getDurations();
            for (auto pair : durations) {
                ImGui::MenuItem(pair.second.c_str(), NULL, &m_selected_duration[pair.first]);
            }
        }
        ImGui::EndMenu();
    }
    float width = ImGui::GetContentRegionAvail().x;

    float button_width = ImGui::CalcTextSize("Historique").x;

    float margin = width - button_width - ImGui::GetStyle().ItemInnerSpacing.x * 4.5f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + margin - 10.f);
    if (!m_ui_state->restauration)
        if (ImGui::Button("Historique")) {
            m_history.show();
        }
    // Hidden debugger
    ImGui::Text("   ");
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right)
        && ImGui::IsItemHovered(0)) {
        m_debugger.show();
    }

    ImGui::EndMainMenuBar();
    m_history.showPreview();
    m_debugger.FrameUpdate();
    m_restore_window.FrameUpdate();
}