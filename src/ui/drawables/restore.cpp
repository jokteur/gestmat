#include "restore.h"
#include "ui/widgets/misc.h"
#include "ui/widgets/modal.h"

void Restore::show() {
    m_files = m_workspace.restoreFromDocuments(false).value();
    m_show_window = true;
    m_history = std::make_shared<HistoryWidget>(m_ui_state);
    m_just_closed = true;
}

void Restore::FrameUpdate() {
    if (m_restoring) {
        // ImVec2 min = ImGui::GetWindowContentRegionMin();
        ImVec2 max = ImGui::GetWindowContentRegionMax();

        ImGui::GetForegroundDrawList()->AddRectFilled(
            ImVec2(0, 0),
            ImVec2(4000, 4000), IM_COL32(125, 125, 125, 50));

        std::string txt = "En cours de restauration (cela peut prendre un moment)...\n"
            "Merci de ne pas éteindre l'ordinateur, de tirer la clef USB et ou de fermer le programme.";

        ImVec2 size = ImGui::CalcTextSize(txt.c_str());
        ImGui::SetCursorPos(ImVec2(max.x / 2.f - size.x / 2.f, max.y / 2.f));
        Tempo::PushFont(m_ui_state->font_bold);
        ImGui::TextColored(ImVec4(255.f, 0.f, 0.f, 255.f), txt.c_str());
        Tempo::PopFont();

        ImGui::SetCursorPos(ImVec2(max.x, max.y));
    }

    if (!m_show_window) {
        confirmation = "";
        if (m_history != nullptr && m_just_closed) {
            m_history->abandon();
            m_just_closed = false;
        }
        return;
    }
    m_history->show(false);
    ImGui::Begin("Restauration", &m_show_window);

    if (m_files.empty()) {
        ImGui::Text("Le logiciel n'a pas trouvé de fichiers de sauvegarde compatibles\n"
            "à restaurer dans le dossier des documents de cet ordinateur.");
        if (ImGui::Button("Fermer")) {
            m_show_window = false;
        }
    }
    else {
        ImGui::Text("Vous êtes sur le point de remplacer les fichiers présents sur la clef USB\n"
            "par les fichiers qui se trouvent dans le dossier 'Documents' de cet ordinateur.\n\n"
        );

        ImGui::InputTextWithHint("##taper", "Merci de taper 'Restaurer' pour continuer", &confirmation);
        if (ImGui::Button("Ne rien faire")) {
            m_show_window = false;
        }
        ImGui::SameLine();

        if (confirmation == "Restaurer") {
            error_msg = "";
        }
        else
            error_msg = "Pour confirmer, il faut taper le mot dans le champ ci-dessus.";

        if (button("Restaurer les fichiers", m_ui_state, error_msg, ImVec4(1, 0, 0, 0.7f), false)) {
            m_show_window = false;

            m_restoring = true;
            m_ui_state->read_only = true;
            m_ui_state->restauration = true;
            Tempo::jobFct job = [this](float&, bool&) -> std::shared_ptr<Tempo::JobResult> {
                Tempo::JobResult job_result;
                auto ret = m_workspace.restoreFromDocuments(true);

                job_result.success = ret.has_value();

                m_restoring = false;
                m_ui_state->read_only = false;
                m_ui_state->restauration = false;
                return std::make_shared<Tempo::JobResult>(job_result);
            };

            Tempo::jobResultFct result_fct = [this](std::shared_ptr<Tempo::JobResult> result) {
                std::string message;
                if (result->success) {
                    message = "Les données ont bien été restaurées.\n";
                }
                else {
                    message = "Erreur dans le transfert des données.";
                }

                const modal_fct fct = [message](bool& show, bool&, bool&) {
                    ImGui::Text(message.c_str());
                    if (ImGui::Button("Ok")) {
                        show = false;
                    }
                };
                Modals::getInstance().setModal("Restauration des données", fct);
            };
            Tempo::JobScheduler::getInstance().addJob("restauring", job, result_fct);

        }
        ImGui::Separator();
        ImGui::Text("Voici un aperçu de l'historique qui va être restauré:");
        ImGui::BeginChild("history_preview");
        m_history->FrameUpdate(m_files);
        ImGui::EndChild();
    }
    ImGui::End();
    if (m_history != nullptr) {
        m_history->showPreview();
    }
}