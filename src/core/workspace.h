#pragma once

#include <vector>
#include <string>
#include "item_manager.h"

namespace core {
    namespace Item {
        using Manager_ptr = std::shared_ptr<Manager>;

        typedef std::vector<std::pair<std::string, std::string>> LoadError;

        struct File {
            std::string path;
            std::string filename;
            std::string action_name;
            long long int timestamp;
        };

        class Workspace {
        private:
            static Workspace& m_workspace_inst;

            Manager_ptr m_current_manager;
            std::string m_work_dir;
            std::string m_docs_dir;
            int m_max_length = 1000;

            bool m_compression = true;

            Workspace() {
                setWorkDir("");
            }

            std::string _get_dir_path();

            bool _save(std::string path, const std::string& content, bool binary);
            std::string _load(const std::string& path, Manager_ptr manager);

            bool _set_my_docs_dir();

            std::string _export_to_excel(std::vector<std::string> paths, std::string name = "");
        public:
            // Singleton stuff
            Workspace(Workspace& other) = delete;
            void operator=(const Workspace&) = delete;
            static Workspace& getInstance() {
                static Workspace instance;
                return instance;
            }

            void setCompression(bool compress);

            std::string loadIntoCurrent(std::string path);
            bool save(std::string action_name, Manager_ptr manager = nullptr);

            std::vector<File> getCompatibleFiles(std::string path = " ");

            std::string getDocsDir();

            bool saveToExcel();

            bool setWorkDir(const std::string& path);
            void setCurrentManager(Manager_ptr manager);
            void setManager(const Manager& manager);

            Manager_ptr getCurrentManager() { return m_current_manager; }
        };
    }
}