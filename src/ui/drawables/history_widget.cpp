#include "history_widget.h"
#include "ui/widgets/misc.h"
#include "ui/imgui_util.h"

#include "ui/widgets/modal.h"

HistoryWidget::HistoryWidget(UIState_ptr ui_state) : Drawable(ui_state) {

}

std::string getMonth(int month) {
    switch (month) {
    case 1:
        return "Janvier";
    case 2:
        return "Février";
    case 3:
        return "Mars";
    case 4:
        return "Avril";
    case 5:
        return "Mai";
    case 6:
        return "Juin";
    case 7:
        return "Juillet";
    case 8:
        return "Août";
    case 9:
        return "Septembre";
    case 10:
        return "Octobre";
    case 11:
        return "Novembre";
    case 12:
        return "Décembre";
    }
    return "inconnu";
}
int day_of_week(int y, int m, int d)
{
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    y -= m < 3;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

std::string getDay(int y, int m, int d) {
    switch (day_of_week(y, m, d)) {
    case 0:
        return "Dimanche";
    case 1:
        return "Lundi";
    case 2:
        return "Mardi";
    case 3:
        return "Mercredi";
    case 4:
        return "Jeudi";
    case 5:
        return "Vendredi";
    case 6:
        return "Samedi";
    }
    return "inconnu";
}
std::string getTime(DateTime time) {
    std::ostringstream txt;
    txt.fill('0');
    txt << std::setw(2) << time.hour
        << "h" << std::setw(2) << time.minute;
    return txt.str();
}

void HistoryWidget::FrameUpdate() {
    if (!m_show)
        return;
    ImGui::Begin("Historique", &m_show);

    auto files = m_workspace.getCompatibleFiles();
    int prev_day = 0;
    int prev_month = 0;
    int prev_year = 0;
    bool draw_day = false;
    bool first = true;
    for (auto file : files) {
        auto datetime = getDatetime(file.timestamp);
        if (prev_year != datetime.year) {
            prev_year = datetime.year;
            Tempo::PushFont(m_ui_state->font_bold);
            ImGui::Text(std::to_string(datetime.year).c_str());
            Tempo::PopFont();
            ImGui::Separator();
        }
        if (prev_month != datetime.month) {
            prev_month = datetime.month;
            Tempo::PushFont(m_ui_state->font_italic);
            ImGui::Text(getMonth(datetime.month).c_str());
            Tempo::PopFont();
        }
        if (prev_day != datetime.day) {
            if (!first && draw_day) {
                ImGui::TreePop();
            }
            first = false;
            prev_day = datetime.day;
            std::string day = getDay(datetime.year, datetime.month, datetime.day);
            day += " " + std::to_string(datetime.day);
            draw_day = ImGui::TreeNode(day.c_str());
        }
        if (draw_day) {
            if (ImGui::Button(labelize(file.timestamp, "Prévisualiser").c_str())) {
                if (m_manager == nullptr)
                    m_manager = m_workspace.getCurrentManager();

                std::string err = m_workspace.loadIntoCurrent(file.path);

                Tempo::EventQueue::getInstance().post(std::make_shared<Tempo::Event>("change_manager"));

                if (err.empty()) {
                    m_visualize = true;
                    m_ui_state->read_only = true;
                    m_current_file = std::make_shared<Item::File>(file);
                }
                else {
                    const modal_fct error_fct = [this, file, err](bool& show, bool&, bool&) {
                        ImGui::Text("Le programme a essayé de charger '"); ImGui::SameLine();
                        ImGui::Text(file.filename.c_str()); ImGui::SameLine();
                        ImGui::Text("' et l'erreur suivante s'est produite:\n\n");
                        ImGui::Text(err.c_str());
                        if (ImGui::Button("Retour")) {
                            show = false;
                        }
                    };
                    Modals::getInstance().setModal("Erreur lors du chargement", error_fct);
                }
            }
            ImGui::SameLine();
            ImGui::AlignTextToFramePadding();
            std::string txt = "À " + getTime(datetime) + ": " + file.action_name;
            ImGui::Text(txt.c_str());
        }
    }
    if (files.size() > 0 && draw_day)
        ImGui::TreePop();
    if (m_set_scroll) {
        m_set_scroll = false;
        ImGui::SetScrollHereY();
    }


    m_vMin = ImGui::GetWindowPos();
    m_vMax = ImGui::GetWindowSize();
    m_vMax.x += ImGui::GetWindowPos().x;
    m_vMax.y += ImGui::GetWindowPos().y;

    ImGui::End();
}
void HistoryWidget::previewBar() {
    auto& style = ImGui::GetStyle();
    float margin = 2.f * style.FramePadding.y + ImGui::GetTextLineHeight();
    ImGui::SetNextWindowPos(ImVec2(m_window_pos.x, m_window_pos.y + m_window_size.y - 1.5f * margin));
    ImGui::SetNextWindowSize(ImVec2(m_window_size.x, 1.5f * margin));
    ImGui::Begin("Previz_widget", 0,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

    auto datetime = getDatetime(m_current_file->timestamp);
    std::string day = getDay(datetime.year, datetime.month, datetime.day);
    std::string txt = "Prévisualisation du ";
    txt += day + " " + std::to_string(datetime.day) + " " + getMonth(datetime.month);
    txt += " " + std::to_string(datetime.year);
    txt += " à " + getTime(datetime);
    ImGui::AlignTextToFramePadding();
    ImGui::Text(txt.c_str()); ImGui::SameLine();
    if (ImGui::Button("Quitter")) {
        abandon();
    }
    ImGui::SameLine();
    if (ImGui::Button("Revenir à cette date")) {
        const modal_fct error_fct = [this](bool& show, bool&, bool&) {
            ImGui::Text("Vous êtes sur le point de remettre l'historique sur une date antérieur");
            ImGui::Text("\nCette action ne va pas supprimer les actions entre la date sélectionnée");
            ImGui::Text("et aujourd'hui, donc il est toujours possible de revenir en avant.");

            if (ImGui::Button("Abandonner")) {
                abandon();
                show = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Continuer")) {
                m_visualize = false;
                m_show = false;
                show = false;
                m_ui_state->read_only = false;
                m_workspace.save("revenir_sur_historique");
            }
        };
        Modals::getInstance().setModal("Revenir à la date", error_fct);
    }
    ImGui::End();
}
void HistoryWidget::showPreview() {
    if (!m_visualize)
        return;

    if (m_current_file == nullptr) {
        m_visualize = false;
        return;
    }

    auto& style = ImGui::GetStyle();
    m_window_pos = ImGui::GetWindowPos();
    m_window_size = ImGui::GetWindowSize();
    float margin = 2.f * style.FramePadding.y + ImGui::GetTextLineHeight();

    if (!m_show) {
        ImGui::GetForegroundDrawList()->AddRectFilled(
            m_window_pos, ImVec2(m_window_size.x, m_window_size.y - 0.5f * margin), IM_COL32(125, 125, 125, 50));
    }
    else {
        margin;
        ImGui::GetForegroundDrawList()->AddRectFilled(
            m_window_pos,
            ImVec2(m_window_size.x, m_vMin.y - m_window_pos.y + margin), IM_COL32(125, 125, 125, 50));
        ImGui::GetForegroundDrawList()->AddRectFilled(
            ImVec2(m_window_pos.x, m_vMax.y),
            ImVec2(m_window_size.x, m_window_size.y - 0.5f * margin), IM_COL32(125, 125, 125, 50));
        ImGui::GetForegroundDrawList()->AddRectFilled(
            ImVec2(m_window_pos.x, m_vMin.y - m_window_pos.y + margin),
            ImVec2(m_vMin.x, m_vMax.y), IM_COL32(125, 125, 125, 50));
        ImGui::GetForegroundDrawList()->AddRectFilled(
            ImVec2(m_vMax.x, m_vMin.y - m_window_pos.y + margin),
            ImVec2(m_window_size.x, m_vMax.y), IM_COL32(125, 125, 125, 50));
    }
    previewBar();
}

void HistoryWidget::abandon() {
    m_visualize = false;
    m_show = false;
    m_current_file = nullptr;
    m_ui_state->read_only = false;
    Tempo::EventQueue::getInstance().post(std::make_shared<Tempo::Event>("change_manager"));
    m_workspace.setCurrentManager(m_manager);
}

void HistoryWidget::BeforeFrameUpdate() {

}