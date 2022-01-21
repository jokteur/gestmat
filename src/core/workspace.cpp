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
        const std::string DIR_NAME{ "sauvegardes_matgest" };
        const std::string EXTENSION{ ".gmat" };

        void getFileInfo(File& file) {
            int i = 11;
            int numlength = 0;
            for (;i < file.filename.size();i++) {
                if (file.filename[i] >= 48 && file.filename[i] <= 57) {
                    numlength++;
                }
                else {
                    break;
                }
            }
            if (numlength == 0 || file.filename.size() < 11) {
                return;
            }

            // Should be okay until 20.11.2286
            if (numlength > 10) {
                numlength = 10;
            }
            file.timestamp = std::stoll(file.filename.substr(11, numlength));
            std::string::size_type const p(file.filename.find_last_of('.'));
            if (i < p - 1) {
                file.action_name = file.filename.substr(i + 1, p - i - 1);
            }
        }

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Property, name, mandatory, no_edit, select, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Note, content, timestamp, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Category, name, properties, registered_items, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Loan, note, date, person, remainder_date, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Person, name, surname, place, notes, birthday, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Item, category, notes, property_values, id);

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
                return DIR_NAME;
            else
                return m_work_dir + "/" + DIR_NAME;
        }

        void Workspace::setCompression(bool compress) {
            m_compression = compress;
        }

        bool Workspace::_save(std::string path, const std::string& content, bool binary) {
            try {
                if (binary) {
                    std::ofstream out(path, std::ios::binary);
                    out.write(content.c_str(), content.size());
                    out.close();
                }
                else {
                    std::ofstream out(path);
                    out << content;
                    out.close();
                }
                return true;

            }
            catch (const std::exception& e) {
                std::cout << "Could not save to file '" << path << "'\n" << e.what() << std::endl;
                return false;
            }
        }

        std::string Workspace::_load(const std::string& path, Manager_ptr manager) {
            std::string json_dump;
            try {
                if (m_compression) {
                    std::ifstream input(path, std::ios::binary);

                    // copies all data into buffer
                    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
                    json_dump = std::string(buffer.begin(), buffer.end());
                    json_dump = decompress_string(json_dump);
                }
                else {
                    std::ifstream in(path);
                    std::string dump(
                        (std::istreambuf_iterator<char>(in)),
                        (std::istreambuf_iterator<char>())
                    );
                    in.close();
                    json_dump = dump;
                }
            }
            catch (std::exception& e) {
                return std::string("Failed to parse file:") + e.what();
            }
            if (manager == nullptr) {
                return std::string("Nullptr error");
            }
            Base::setID(0);

            json j = json::parse(json_dump);
            manager->m_item_loan_map = j["item_loan_map"];
            manager->m_person_loan_map = j["person_loan_map"];

            // Items
            for (auto str : j["registered_items"]) {
                Item item = str;
                manager->m_registered_items[item.id] = std::make_shared<Item>(item);
            }
            for (auto str : j["retired_items"]) {
                Item item = str;
                manager->m_retired_items[item.id] = std::make_shared<Item>(item);
            }
            // Categories
            for (auto str : j["registered_categories"]) {
                Category cat = str;
                manager->m_registered_categories[cat.id] = std::make_shared<Category>(cat);
            }
            for (auto str : j["retired_categories"]) {
                Category cat = str;
                manager->m_retired_categories[cat.id] = std::make_shared<Category>(cat);
            }
            // Properties
            for (auto str : j["registered_properties"]) {
                Property prop = str;
                manager->m_registered_properties[prop.id] = std::make_shared<Property>(prop);
            }
            for (auto str : j["retired_properties"]) {
                Property prop = str;
                manager->m_retired_properties[prop.id] = std::make_shared<Property>(prop);
            }
            // Persons
            for (auto str : j["registered_persons"]) {
                Person person = str;
                manager->m_registered_persons[person.id] = std::make_shared<Person>(person);
            }
            for (auto str : j["retired_persons"]) {
                Person person = str;
                manager->m_retired_persons[person.id] = std::make_shared<Person>(person);
            }
            // Loans
            for (auto str : j["registered_loans"]) {
                Loan loans = str;
                manager->m_registered_loans[loans.id] = std::make_shared<Loan>(loans);
            }
            for (auto str : j["retired_loans"]) {
                Loan loans = str;
                manager->m_retired_loans[loans.id] = std::make_shared<Loan>(loans);
            }

            Base::setID(j["global_ID"]);

            return "";
        }

        std::string Workspace::loadIntoCurrent(std::string path) {
            if (!std::filesystem::exists(path))
                return "File does not exists";

            std::string ret;
            try {
                Manager_ptr manager = std::make_shared<Manager>();
                ret = _load(path, manager);
                if (ret.empty())
                    m_current_manager = manager;
                else
                    return std::string("Failed to load file: ") + ret;
            }
            catch (std::exception& e) {
                ret = std::string("Failed to load file: ") + e.what();
            }
            return ret;
        }

        bool Workspace::save(std::string name, Manager_ptr manager) {
            if (manager == nullptr)
                manager = m_current_manager;
            if (manager == nullptr)
                return false;

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

            std::string filename = getCurrentDate().format("%Y-%m-%d-")
                + std::to_string(getTimestamp())
                + std::string("-")
                + name + EXTENSION;

            std::string out_str;
            if (m_compression) {
                out_str = compress_string(out_file.dump());
            }
            else {
                out_str = out_file.dump();
            }

            bool ret = false;
            ret = _save(_get_dir_path() + "\\" + filename, out_str, m_compression);
            ret &= _save(m_docs_dir + "\\" + filename, out_str, m_compression);
            return ret;
        }

        bool Workspace::setWorkDir(const std::string& path) {
            _set_my_docs_dir();

            try {
                std::filesystem::create_directory(m_docs_dir + "\\" + DIR_NAME);
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }

            bool ret;
            if (std::filesystem::exists(path)) {
                ret = true;
            }
            if (path.empty())
                ret = std::filesystem::create_directory(DIR_NAME);
            else
                ret = std::filesystem::create_directory(path + "\\" + DIR_NAME);

            m_work_dir = path;
            return ret;
        }

        std::vector<File> Workspace::getCompatibleFiles(std::string path) {
            if (path == " ") {
                path = m_work_dir;
                path += !path.empty() ? "\\" + DIR_NAME : DIR_NAME;
            }
            std::vector<File> files;
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                std::string file_path = entry.path().string();
                std::string file_name = entry.path().filename().string();
                std::string::size_type const p(file_name.find_last_of('.'));
                std::string extension = file_name.substr(p);

                if (extension == EXTENSION) {
                    File file{ file_path, file_name };
                    getFileInfo(file);
                    files.push_back(file);
                }
            }
            return files;
        }


        void Workspace::setCurrentManager(Manager_ptr manager) {
            m_current_manager = manager;
        }

        void Workspace::setManager(const Manager& manager) {
            m_current_manager = std::make_shared<Manager>(manager);
        }
    }
}