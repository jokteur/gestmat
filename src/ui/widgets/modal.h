#pragma once

#include <string>
#include <functional>
#include <utility>
#include <vector>
#include <iostream>

#include <tempo.h>

#include "ui/drawable.h"

using modal_fct = std::function<void(bool&, bool&, bool&)>;

/**
 * Struct to help draw a Modal
 */
struct Modal {
    std::string title;
    modal_fct draw_fct;
    int flags = 0;
    bool show = false;
    bool enter = false;
    bool escape = false;
    bool no_close = false;
    UIState_ptr ui_state = nullptr;

    // Modals can be stacked inside other modals
    std::shared_ptr<Modal> modal = nullptr;

    void popup(bool* open) {
        if (ImGui::BeginPopupModal(title.c_str(), open, flags)) {
            draw_fct(show, enter, escape);
            if (modal != nullptr) {
                modal->FrameUpdate();
            }
            ImGui::EndPopup();
        }
    }

    void FrameUpdate() {
        if (show) {
            ImGui::OpenPopup(title.c_str());
        }
        if (no_close) {
            popup(nullptr);
        }
        else {
            popup(&show);
        }
    }
};

/**
 * This class helps to manage modal (also modals inside modals)
 * This is a Singleton, to be called from anywhere
 * The class that draws the modal is called ModalsDrawable
 */
class Modals {
private:
    std::shared_ptr<Modal> m_modal = std::make_shared<Modal>();
    std::vector<std::shared_ptr<Modal>> m_stacked_modals;

    void free_memory() {
        // Erase all previously stacked modals
        m_stacked_modals.clear();
    }

    Modals() = default;
public:
    /**
     * Copy constructors stay empty, because of the Singleton
     */
    Modals(Modals const&) = delete;
    void operator=(Modals const&) = delete;

    /**
     * @return instance of the Singleton of the Job Scheduler
     */
    static Modals& getInstance() {
        static Modals instance;
        return instance;
    }

    /**
     * Returns true if any modal is already active
     * @return
     */
    bool isActive() const {
        return m_modal->show;
    }

    /**
     * Sets the current modal to be displayed
     * @param title title of the modal
     * @param draw_fct function to be drawn in the modal
     * @param flags ImGui flags for the modal
     */
    void setModal(std::string title, modal_fct draw_fct, int flags = 0, bool no_close = false) {
        free_memory();

        m_modal->title = std::move(title);
        m_modal->draw_fct = std::move(draw_fct);
        m_modal->show = true;
        m_modal->modal = nullptr;
        m_modal->enter = false;
        m_modal->escape = false;
        m_modal->flags = flags;
        m_modal->no_close = no_close;
    }

    /**
     * Stacks a modal inside the currently displayed modal
     * @param title title of the modal
     * @param draw_fct function to be drawn in the modal
     * @param flags ImGui flags for the modal
     */
    void stackModal(const std::string& title, const modal_fct& draw_fct, int flags = 0, bool no_close = false) {
        // Means that no modal is currently showing
        if (!m_modal->show) {
            m_modal->show = true;
            m_modal->flags = flags;
            m_modal->title = title;
            m_modal->draw_fct = draw_fct;
            m_modal->modal = nullptr;
            m_modal->no_close = no_close;
        }
        else {
            std::shared_ptr<Modal> tmp_modal = m_modal->modal;
            while (tmp_modal != nullptr)
                tmp_modal = tmp_modal->modal;
            auto modal = std::make_shared<Modal>();
            modal->title = title;
            modal->draw_fct = draw_fct;
            modal->flags = flags;
            modal->show = true;
            modal->no_close = no_close;

            m_modal->modal = modal;
            m_stacked_modals.push_back(modal);
        }
    }

    void FrameUpdate() {
        m_modal->FrameUpdate();
    }

    ~Modals() {
        free_memory();
    }
};