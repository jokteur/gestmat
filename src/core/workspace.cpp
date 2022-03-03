#include "workspace.h"
#include "compress.h"

#include "python/py_api.h"
#include "python/with.h"

#include <windows.h>
#include <direct.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <tempo.h>

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
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Category, name, properties, registered_items, properties_hide, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Loan, note, item, date, date_back, person, remainder_date, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Person, name, surname, place, notes, birthday, id);
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Item, category, notes, property_values, id);

        std::string Workspace::_get_desktop_path() {
            TCHAR appData[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPath(NULL,
                CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_CREATE,
                NULL,
                SHGFP_TYPE_CURRENT,
                appData)))
                return std::string(appData);
            return "";
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
            return true;
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

            if (j.contains("alert_duration")) {
                manager->m_duration_before_alert = j["alert_duration"];
            }

            int loan_maps = 0;
            if (j.contains("past_item_loan_map")) {
                loan_maps++;
                manager->m_past_item_loan_map = j["past_item_loan_map"];
            }
            if (j.contains("past_person_loan_map")) {
                loan_maps++;
                manager->m_past_person_loan_map = j["past_person_loan_map"];
            }

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
                if (!str.contains("date_back")) {
                    str["date_back"] = Date{ 0,0,0 };
                }
                Loan loans = str;
                manager->m_registered_loans[loans.id] = std::make_shared<Loan>(loans);
            }
            for (auto str : j["retired_loans"]) {
                if (!str.contains("date_back")) {
                    str["date_back"] = Date{ 0,0,0 };
                }
                Loan loans = str;
                manager->m_retired_loans[loans.id] = std::make_shared<Loan>(loans);
            }

            Base::setID(j["global_ID"]);
            // if (loan_maps != 2) {
            // manager->buildRetiredLoanCorrespondance();
            // std::cout << manager->m_past_item_loan_map.size() << std::endl;
            // }

            return "";
        }

        std::string Workspace::loadIntoCurrent(std::string path) {
            if (!std::filesystem::exists(path))
                return "File does not exists";

            std::string ret;
            try {
                Manager_ptr manager = std::make_shared<Manager>();
                ret = _load(path, manager);
                if (ret.empty()) {
                    m_current_manager = manager;
                    manager->cleanUp();
                }
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
            out_file["past_item_loan_map"] = manager->m_past_item_loan_map;
            out_file["past_person_loan_map"] = manager->m_past_person_loan_map;
            out_file["alert_duration"] = manager->m_duration_before_alert;

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
            ret &= _save(m_docs_dir + "\\" + DIR_NAME + "\\" + filename, out_str, m_compression);
            std::cout << m_docs_dir << std::endl;

            manager->setChange();

            saveToExcel();

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
            std::map<std::string, File> files_order;
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                std::string file_path = entry.path().string();
                std::string file_name = entry.path().filename().string();
                std::string::size_type const p(file_name.find_last_of('.'));
                if (p >= file_name.size())
                    continue;
                std::string extension = file_name.substr(p);

                if (extension == EXTENSION) {
                    File file{ file_path, file_name };
                    getFileInfo(file);
                    //We shouldn't have twice the same filename
                    files_order[file_name] = file;
                }
            }
            std::vector<File> files;
            for (auto pair : files_order) {
                files.push_back(pair.second);
            }
            return files;
        }

        std::string Workspace::_export_to_excel(std::vector<std::string> paths, std::string) {
            if (m_current_manager == nullptr)
                return "no_manager";

            // One big ugly function to save data to
            // an excel file

            std::string filename = "\\" + getCurrentDate().format("%Y-%m-%d")
                + std::string("-materiel_ergo.xlsx");

            auto state = PyGILState_Ensure();
            try {
                auto pd = py::module::import("pandas");

                py::dict kwargs;

                // Current loans
                py::list l_date_ordered, l_date, l_name, l_surname, l_birthday;
                py::list l_place, l_type, l_infos, l_notes;
                std::string first_col_name;

                for (auto pair : m_current_manager->m_item_loan_map) {
                    auto item = m_current_manager->getItem(pair.first).value();
                    auto cat = m_current_manager->getCategory(item->category).value();
                    for (auto loan_id : pair.second) {
                        auto loan = m_current_manager->getLoan(loan_id).value();
                        auto person = m_current_manager->getPerson(loan->person).value();
                        l_date_ordered.append(loan->date.format("%Y/%m/%d"));
                        l_date.append(loan->date.format("%d/%m/%Y"));
                        l_surname.append(person->surname);
                        l_name.append(person->name);
                        l_type.append(cat->name);
                        l_birthday.append(person->birthday.format("%d/%m/%Y"));
                        l_place.append(person->place);

                        std::string str;
                        int i = 0;
                        for (auto prop_pair : item->property_values) {
                            if (i > 0)
                                str += " / ";
                            auto prop = m_current_manager->getProperty(prop_pair.first).value();
                            str += prop->name + ": " + item->property_values[prop->id];
                            i++;
                        }
                        std::string notes;
                        if (!person->notes.empty()) {
                            i = 0;
                            for (auto note : person->notes) {
                                if (i > 0)
                                    notes += "\n";
                                notes += note.content;
                                i++;
                            }
                        }
                        l_notes.append(notes);
                        l_infos.append(str);
                    }
                }
                py::dict data;
                data["sort"] = l_date_ordered;
                data["Date d'emprunt"] = l_date;
                data["Nom"] = l_surname;
                data["Prénom"] = l_name;
                data["Date de naissance"] = l_birthday;
                data["Unité / Chambre"] = l_place;
                data["Type"] = l_type;
                data["Notes"] = l_notes;
                data["Infos objets"] = l_infos;

                kwargs["data"] = data;
                auto df = pd.attr("DataFrame")(**kwargs);
                df.attr("sort_values")("sort", 0, true, true);
                auto dic = py::dict();
                dic["inplace"] = true;
                df.attr("drop")("sort", 1, **dic);

                // Items
                auto collections = py::module::import("collections");


                std::map<std::string, py::object> dfs;
                for (auto cat_id : m_current_manager->getAllCategories()) {
                    auto cat = m_current_manager->getCategory(cat_id).value();
                    std::map<std::string, py::list> columns;
                    for (auto item_id : cat->registered_items) {
                        auto item = m_current_manager->getItem(item_id).value();
                        for (auto prop_id : cat->properties) {
                            auto prop = m_current_manager->getProperty(prop_id).value();
                            std::string content;
                            if (item->property_values.contains(prop_id)) {
                                content = item->property_values[prop_id];
                            }
                            columns[prop->name].append(content);
                        }
                    }
                    kwargs.clear();

                    data.clear();
                    std::string first_col;
                    int i = 0;
                    for (auto prop_id : cat->properties) {
                        auto prop = m_current_manager->getProperty(prop_id).value();
                        if (i == 0)
                            first_col = prop->name;
                        data[prop->name.c_str()] = columns[prop->name];
                        i++;
                    }
                    kwargs["data"] = data;
                    auto df_ = pd.attr("DataFrame")(**kwargs);
                    if (!first_col.empty())
                        df_.attr("sort_values")(first_col, 0, true, true);
                    dfs[cat->name] = df_;
                    // py::print(dfs[cat->name]);
                    // dfs[cat->name].attr("sort_values")(dfs[cat->name].attr("columns")[0]);
                }

                for (auto path : paths) {
                    path += filename;
                    py::with(pd.attr("ExcelWriter")(path), [&df, &dfs](py::object writer) {
                        py::dict kwargs;
                        kwargs["sheet_name"] = "Emprunts en cours";
                        kwargs["index"] = false;
                        df.attr("to_excel")(writer, **kwargs);
                        for (auto pair : dfs) {
                            kwargs["sheet_name"] = pair.first;
                            pair.second.attr("to_excel")(writer, **kwargs);
                        }
                        });
                }
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                PyGILState_Release(state);
                return e.what();
            }

            PyGILState_Release(state);
            return "";
        }

        bool Workspace::exportToDesktop() {
            try {
                std::string desktop = _get_desktop_path();
                std::string path = desktop + "\\gestion_materiel";
                std::filesystem::create_directory(path);
                std::string ret = _export_to_excel({ path });
                if (ret.empty())
                    return true;
                else {
                    std::cout << ret << std::endl;
                    return false;
                }
            }
            catch (std::exception& e) {
                std::cout << e.what() << std::endl;
                return false;
            }
        }

        std::optional<std::vector<File>> Workspace::restoreFromDocuments(bool commit) {
            std::string path = m_docs_dir + "\\" + DIR_NAME;
            auto files = getCompatibleFiles(path);

            if (commit) {
                bool success = true;
                try {
                    std::filesystem::copy(DIR_NAME + "\\",
                        DIR_NAME + getCurrentDate().format("-backup-%Y-%m-%d") + "\\");
                }
                catch (std::exception& e) {
                    std::cout << e.what() << std::endl;
                    success = false;
                }
                try {
                    std::filesystem::remove_all(DIR_NAME);
                }
                catch (std::exception& e) {
                    std::cout << e.what() << std::endl;
                    success = false;
                }
                try {
                    std::filesystem::copy(m_docs_dir + "\\" + DIR_NAME + "\\",
                        DIR_NAME + "\\");

                    auto new_files = getCompatibleFiles();
                    core::Item::File last;
                    for (const auto& file : new_files) {
                        last = file;
                    }
                    if (last.path.empty())
                        setCurrentManager(std::make_shared<Manager>());
                    else
                        loadIntoCurrent(last.path);

                    Tempo::EventQueue::getInstance().post(std::make_shared<Tempo::Event>("change_manager"));
                }
                catch (std::exception& e) {
                    std::cout << e.what() << std::endl;
                    success = false;
                }

                if (!success)
                    return std::optional<std::vector<File>>();
            }
            return std::optional<std::vector<File>>(files);
        }

        bool Workspace::saveToExcel() {
            std::vector<std::string> paths = {
                _get_dir_path() + "\\excel",
                m_docs_dir + "\\" + DIR_NAME + "\\excel"
            };
            std::filesystem::create_directory(paths[0]);
            std::filesystem::create_directory(paths[1]);
            std::string ret = _export_to_excel(paths);
            return true;
        }

        void Workspace::setCurrentManager(Manager_ptr manager) {
            m_current_manager = manager;
        }

        void Workspace::setManager(const Manager& manager) {
            m_current_manager = std::make_shared<Manager>(manager);
        }
    }
}