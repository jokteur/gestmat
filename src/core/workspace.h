#pragma once

#include <vector>
#include <string>
#include "item_manager.h"

namespace core {
    namespace Item {
        using Manager_ptr = std::shared_ptr<Manager>;

        struct HistoryPoint {
            std::string name;
            Manager_ptr manager;
            long long int timestamp;
        };

        class Workspace {
        private:
            static Workspace& m_workspace_inst;

            std::map<long long int, HistoryPoint> m_history;
            Manager_ptr m_current_manager;
            std::string m_work_dir;
            std::string m_docs_dir;
            int m_max_length = 1000;

            Workspace() {
                setWorkDir("");
            }

            std::string _get_dir_path();

            bool _save(std::string path, const std::string& content);
            bool _load(const std::string& path, HistoryPoint& point);
            void _insert_history_point(const Manager_ptr& manager, std::string name, long long int timestamp = 0);

            bool _set_my_docs_dir();

        public:
            // Singleton stuff
            Workspace(Workspace& other) = delete;
            void operator=(const Workspace&) = delete;
            static Workspace& getInstance() {
                static Workspace instance;
                return instance;
            }

            bool loadDir(bool my_docs = false);
            bool loadIntoCurrent(std::string path);
            bool save(std::string action_name, Manager_ptr manager = nullptr);

            bool setWorkDir(const std::string& path);
            void setMaxHistoryLength(int length);
            void setCurrentManager(Manager_ptr manager);

            void insertManager(const Manager& manager);


            Manager_ptr getCurrentManager() { return m_current_manager; }
            std::map<long long int, HistoryPoint> getManagerHistory() { return m_history; }
        };
    }
}