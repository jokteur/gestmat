#include "workspace.h"
#include "compress.h"

#include "python/py_api.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace core {
    namespace Item {
        namespace py = pybind11;
        using namespace nlohmann;
        std::string dir_name("sauvegardes_matgest");

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Property, name, mandatory, no_edit, select, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Note, content, timestamp, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Category, name, properties, registered_items, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Loan, note, date, person, remainder_date, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Person, name, surname, place, notes, loans, birthday, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Item, category, notes, property_values, id);

        void Workspace::_insert_history_point(const Manager_ptr& manager, std::string name, long long int timestamp) {
            if (timestamp == 0) {
                const auto p1 = std::chrono::system_clock::now();
                timestamp = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
            }
            HistoryPoint point{
                name,
                manager,
                timestamp
            };
            m_history[timestamp] = point;
        }

        bool Workspace::_set_my_docs_dir() {
            auto state = PyGILState_Ensure();
            try {
                auto wksp = py::module::import("python.scripts.workspace");
                m_docs_dir = wksp.attr("get_docs_dir")().cast<std::string>();
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                PyGILState_Release(state);
                return false;
            }

            PyGILState_Release(state);
            return true;;
        }

        std::string Workspace::_get_dir_path() {
            if (m_work_dir.empty())
                return dir_name;
            else
                return m_work_dir + "/" + dir_name;
        }

        bool Workspace::_save(std::string path, const std::string& content) {
            try {
                std::ofstream out(path);
                out << content;
                out.close();
                return true;
            }
            catch (const std::exception& e) {
                std::cout << "Could not save to file '" << path << "'\n" << e.what() << std::endl;
                return false;
            }
        }

        bool Workspace::_load(const std::string& path, HistoryPoint& point) {
            std::ifstream in(path);
            std::string json_dump(
                (std::istreambuf_iterator<char>(in)),
                (std::istreambuf_iterator<char>())
            );
            in.close();
            Base::setID(0);

            json j = json::parse(json_dump);
            point.name = j["name"];
            point.timestamp = j["timestamp"];
            point.manager->m_item_loan_map = j["item_loan_map"];
            point.manager->m_person_loan_map = j["person_loan_map"];

            // Items
            for (auto str : j["registered_items"]) {
                Item cat = str;
                point.manager->m_registered_items[cat.id] = std::make_shared<Item>(cat);
            }
            for (auto str : j["retired_items"]) {
                Item cat = str;
                point.manager->m_retired_items[cat.id] = std::make_shared<Item>(cat);
            }
            // Categories
            for (auto str : j["registered_categories"]) {
                Category cat = str;
                point.manager->m_registered_categories[cat.id] = std::make_shared<Category>(cat);
            }
            for (auto str : j["retired_categories"]) {
                Category cat = str;
                point.manager->m_retired_categories[cat.id] = std::make_shared<Category>(cat);
            }
            // Properties
            for (auto str : j["registered_properties"]) {
                Property cat = str;
                point.manager->m_registered_properties[cat.id] = std::make_shared<Property>(cat);
            }
            for (auto str : j["retired_properties"]) {
                Property cat = str;
                point.manager->m_retired_properties[cat.id] = std::make_shared<Property>(cat);
            }
            // Persons
            for (auto str : j["registered_persons"]) {
                Person cat = str;
                point.manager->m_registered_persons[cat.id] = std::make_shared<Person>(cat);
            }
            for (auto str : j["retired_persons"]) {
                Person cat = str;
                point.manager->m_retired_persons[cat.id] = std::make_shared<Person>(cat);
            }
            // Loans
            for (auto str : j["registered_loans"]) {
                Loan cat = str;
                point.manager->m_registered_loans[cat.id] = std::make_shared<Loan>(cat);
            }
            for (auto str : j["retired_loans"]) {
                Loan cat = str;
                point.manager->m_retired_loans[cat.id] = std::make_shared<Loan>(cat);
            }

            Base::setID(j["global_ID"]);

            return true;
        }

        bool Workspace::loadDir(bool my_docs) {
            std::string path = my_docs ? m_docs_dir : m_work_dir;
            path += !path.empty() ? "\\" + dir_name : dir_name;


            if (!std::filesystem::exists(path))
                return false;

            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                Manager_ptr manager = std::make_shared<Manager>();
                HistoryPoint point{ "", manager };
                _load(entry.path().string(), point);
                _insert_history_point(point.manager, point.name, point.timestamp);
                m_current_manager = point.manager;
            }

            return true;
        }
        bool Workspace::loadIntoCurrent(std::string path) {
            if (!std::filesystem::exists(path))
                return false;

            HistoryPoint point;
            bool ret = _load(path, point);
            if (ret)
                m_current_manager = point.manager;
            return ret;
        }

        bool Workspace::save(std::string name, Manager_ptr manager) {
            if (manager == nullptr)
                manager = m_current_manager;
            if (manager == nullptr)
                return false;

            _insert_history_point(manager, name);

            json out_file;

            out_file["name"] = name;
            out_file["global_ID"] = Base::getID();
            out_file["timestamp"] = getTimestamp();
            out_file["item_loan_map"] = manager->m_item_loan_map;
            out_file["person_loan_map"] = manager->m_person_loan_map;

            // Items
            for (auto& pair : manager->m_registered_items) {
                out_file["registered_items"].push_back(*pair.second);
            }
            for (auto& pair : manager->m_retired_items) {
                out_file["retired_items"].push_back(*pair.second);
            }
            // Categories
            for (auto& pair : manager->m_registered_categories) {
                out_file["registered_categories"].push_back(*pair.second);
            }
            for (auto& pair : manager->m_retired_categories) {
                out_file["retired_categories"].push_back(*pair.second);
            }
            // Properties
            for (auto& pair : manager->m_registered_properties) {
                out_file["registered_properties"].push_back(*pair.second);
            }
            for (auto& pair : manager->m_retired_properties) {
                out_file["retired_properties"].push_back(*pair.second);
            }
            // Persons
            for (auto& pair : manager->m_registered_persons) {
                out_file["registered_persons"].push_back(*pair.second);
            }
            for (auto& pair : manager->m_retired_persons) {
                out_file["retired_persons"].push_back(*pair.second);
            }
            // Loans
            for (auto& pair : manager->m_registered_loans) {
                out_file["registered_loans"].push_back(*pair.second);
            }
            for (auto& pair : manager->m_retired_loans) {
                out_file["retired_loans"].push_back(*pair.second);
            }

            std::string filename = getCurrentDate().format("%Y-%m-%d-") + std::to_string(getTimestamp()) + ".json";

            bool ret = false;
            ret = _save(_get_dir_path() + "\\" + filename, out_file.dump());
            ret &= _save(m_docs_dir + "\\" + filename, out_file.dump());
            return ret;
        }

        bool Workspace::setWorkDir(const std::string& path) {
            _set_my_docs_dir();

            try {
                std::filesystem::create_directory(m_docs_dir + "\\" + dir_name);
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }

            if (std::filesystem::exists(path)) {
                return true;
            }
            bool ret;
            if (path.empty())
                ret = std::filesystem::create_directory(dir_name);
            else
                ret = std::filesystem::create_directory(path + "\\" + dir_name);

            m_work_dir = path;
            return ret;
        }

        void Workspace::setMaxHistoryLength(int length) {
            if (length <= 0)
                return;
            m_max_length = length;
            // TODO: remove items older than m_max_length
        }

        void Workspace::setCurrentManager(Manager_ptr manager) {
            m_current_manager = manager;
        }

        void Workspace::insertManager(const Manager& manager) {
            m_current_manager = std::make_shared<Manager>(manager);
            _insert_history_point(m_current_manager, "Insertion");
        }
    }
}