#include "item_manager.h"

#include <iostream> 
namespace core {
    namespace Item {
        ObjectID Base::ID = 0;
        std::set<ObjectID> Base::_ids;

        template<typename V, typename T>
        bool find_in_map(const std::map<V, T>& map, V id) {
            return map.find(id) != map.end();
        }

        template<typename V, typename T>
        bool exchange_btw_maps(std::map<V, T>& giving, std::map<V, T>& receiving, V id) {
            if (find_in_map<V, T>(giving, id)) {
                receiving[id] = giving[id];
                giving.erase(id);
                return true;
            }
            return false;
        }
        template<typename V, typename T>
        void copy_map(const std::map<V, std::shared_ptr<T>>& source, std::map<V, std::shared_ptr<T>>& destination) {
            for (auto& pair : source) {
                destination[pair.first] = std::make_shared<T>(*pair.second);
            }
        }

        Manager::Manager(const Manager& other) {
            m_item_loan_map = other.m_item_loan_map;
            m_person_loan_map = other.m_person_loan_map;

            copy_map<ItemID, Item>(other.m_registered_items, m_registered_items);
            copy_map<ItemID, Item>(other.m_retired_items, m_retired_items);

            copy_map<CategoryID, Category>(other.m_registered_categories, m_registered_categories);
            copy_map<CategoryID, Category>(other.m_retired_categories, m_retired_categories);

            copy_map<PropertyID, Property>(other.m_registered_properties, m_registered_properties);
            copy_map<PropertyID, Property>(other.m_retired_properties, m_retired_properties);

            copy_map<PersonID, Person>(other.m_registered_persons, m_registered_persons);
            copy_map<PersonID, Person>(other.m_retired_persons, m_retired_persons);

            copy_map<LoanID, Loan>(other.m_registered_loans, m_registered_loans);
            copy_map<LoanID, Loan>(other.m_retired_loans, m_retired_loans);
        }

        void Manager::change() {
            for (auto& pair : m_subscriber_ids) {
                pair.second = true;
            }
            std::cout << Base::getID() << " " << Base::getIDs().size() << std::endl;
        }

        bool Manager::isChanged(long long int id) {
            if (!m_subscriber_ids.contains(id)) {
                m_subscriber_ids[id] = true;
            }
            bool ret;
            ret = m_subscriber_ids[id];
            m_subscriber_ids[id] = false;
            return ret;
        }

        void Manager::giveBackId(long long int id) {
            if (m_subscriber_ids.contains(id)) {
                m_subscriber_ids.erase(id);
            }
        }

        bool Manager::isLoaned(ItemID item_id) {
            return m_item_loan_map.contains(item_id);
        }

        std::optional<bool> Manager::isRetired(ObjectID id) {
            if (find_in_map<ItemID, Item_ptr>(m_registered_items, (ItemID)id)) {
                return std::optional<bool>(false);
            }
            else if (find_in_map<ItemID, Item_ptr>(m_retired_items, (ItemID)id)) {
                return std::optional<bool>(true);
            }
            if (find_in_map<CategoryID, Category_ptr>(m_registered_categories, (CategoryID)id)) {
                return std::optional<bool>(false);
            }
            else if (find_in_map<CategoryID, Category_ptr>(m_retired_categories, (CategoryID)id)) {
                return std::optional<bool>(true);
            }
            if (find_in_map<PropertyID, Property_ptr>(m_registered_properties, (PropertyID)id)) {
                return std::optional<bool>(false);
            }
            else if (find_in_map<PropertyID, Property_ptr>(m_retired_properties, (PropertyID)id)) {
                return std::optional<bool>(true);
            }
            if (find_in_map<LoanID, Loan_ptr>(m_registered_loans, (LoanID)id)) {
                return std::optional<bool>(false);
            }
            else if (find_in_map<LoanID, Loan_ptr>(m_retired_loans, (LoanID)id)) {
                return std::optional<bool>(true);
            }
            return std::optional<bool>();
        }


        template<typename V, typename T>
        void add_to_set(const std::map<V, T>& map) {
            for (auto& pair : map) {
                Base::addID(pair.first);
            }
        }
        void Manager::buildIDSet() {
            Base::getIDs().clear();
            add_to_set<ItemID, Item_ptr>(m_registered_items);
            add_to_set<ItemID, Item_ptr>(m_retired_items);
            add_to_set<CategoryID, Category_ptr>(m_registered_categories);
            add_to_set<CategoryID, Category_ptr>(m_retired_categories);
            add_to_set<PropertyID, Property_ptr>(m_registered_properties);
            add_to_set<PropertyID, Property_ptr>(m_retired_properties);
            add_to_set<PersonID, Person_ptr>(m_registered_persons);
            add_to_set<PersonID, Person_ptr>(m_retired_persons);
            add_to_set<LoanID, Loan_ptr>(m_registered_loans);
            add_to_set<LoanID, Loan_ptr>(m_retired_loans);

            Base::setID(100);
        }

        std::vector<ObjectID> intersect(std::set<ObjectID> v1, std::set<ObjectID> v2) {
            std::vector<ObjectID> v_intersection;

            std::set_intersection(v1.begin(), v1.end(),
                v2.begin(), v2.end(),
                std::back_inserter(v_intersection));
            return v_intersection;
        }

        template<typename V, typename T>
        void fill_set(const std::map<V, T>& map, std::set<ObjectID>& set) {
            for (auto& pair : map) {
                set.insert(pair.first);
            }
        }


        void Manager::findDuplicates() {
            std::set<ObjectID> items;
            std::set<ObjectID> categories;
            std::set<ObjectID> properties;
            std::set<ObjectID> persons;
            std::set<ObjectID> loans;

            fill_set<ItemID, Item_ptr>(m_registered_items, items);
            fill_set<ItemID, Item_ptr>(m_retired_items, items);
            fill_set<CategoryID, Category_ptr>(m_registered_categories, categories);
            fill_set<CategoryID, Category_ptr>(m_retired_categories, categories);
            fill_set<PropertyID, Property_ptr>(m_registered_properties, properties);
            fill_set<PropertyID, Property_ptr>(m_retired_properties, properties);
            fill_set<PersonID, Person_ptr>(m_registered_persons, persons);
            fill_set<PersonID, Person_ptr>(m_retired_persons, persons);
            fill_set<LoanID, Loan_ptr>(m_registered_loans, loans);
            fill_set<LoanID, Loan_ptr>(m_retired_loans, loans);

            // auto i_c = intersect(items, categories);
            // std::cout << "i_c " << i_c.size() << std::endl;
            // auto i_pr = intersect(items, properties);
            // std::cout << "i_pr " << i_pr.size() << std::endl;
            // auto i_pe = intersect(items, persons);
            // std::cout << "i_pe " << i_pe.size() << std::endl;
            // auto i_l = intersect(items, loans);
            // std::cout << "i_l " << i_l.size() << std::endl;
            // auto c_pr = intersect(categories, properties);
            // std::cout << "c_pr " << c_pr.size() << std::endl;
            // auto c_pe = intersect(categories, persons);
            // std::cout << "c_pe " << c_pe.size() << std::endl;
            // auto c_l = intersect(categories, loans);
            // std::cout << "c_l " << c_l.size() << std::endl;
            // auto pr_pe = intersect(properties, persons);
            // std::cout << "pr_pe " << pr_pe.size() << std::endl;
            // auto pe_l = intersect(persons, loans);
            // std::cout << "pe_l " << pe_l.size() << std::endl;
            // std::cout << std::endl;
        }

        void Manager::cleanUp() {
            buildIDSet();
            findDuplicates();
            std::set<ItemID> to_delete;
            for (auto pair : m_item_loan_map) {
                if (pair.second.empty()) {
                    to_delete.insert(pair.first);
                }
                // for (auto loan_id : pair.second) {

                // }
            }
            for (auto item_id : to_delete) {
                m_item_loan_map.erase(item_id);
            }
        }

        std::set<CategoryID> Manager::getAllCategories() {
            std::set<CategoryID> set;
            for (auto cat_pair : m_registered_categories) {
                set.insert(cat_pair.first);
            }
            for (auto cat_pair : m_retired_categories) {
                set.insert(cat_pair.first);
            }
            return set;
        }
        std::vector<ItemID> Manager::getAllItems() {
            std::vector<ItemID> set;
            for (auto cat_pair : m_registered_items) {
                set.push_back(cat_pair.first);
            }
            for (auto cat_pair : m_retired_items) {
                set.push_back(cat_pair.first);
            }
            return set;
        }
        std::set<PropertyID> Manager::getAllProperties() {
            std::set<PropertyID> set;
            for (auto cat_pair : m_registered_properties) {
                set.insert(cat_pair.first);
            }
            for (auto cat_pair : m_retired_properties) {
                set.insert(cat_pair.first);
            }
            return set;
        }
        std::set<LoanID> Manager::getAllLoans() {
            std::set<LoanID> set;
            for (auto cat_pair : m_registered_loans) {
                set.insert(cat_pair.first);
            }
            for (auto cat_pair : m_retired_loans) {
                set.insert(cat_pair.first);
            }
            return set;
        }
        std::set<LoanID> Manager::getActiveLoans() {
            std::set<LoanID> set;
            for (auto cat_pair : m_registered_loans) {
                set.insert(cat_pair.first);
            }
            return set;
        }

        std::optional<Item_ptr> Manager::getItem(ItemID item_id) {
            if (find_in_map<ItemID, Item_ptr>(m_registered_items, item_id)) {
                return std::optional<Item_ptr>(m_registered_items[item_id]);
            }
            else if (find_in_map<ItemID, Item_ptr>(m_retired_items, item_id)) {
                return std::optional<Item_ptr>(m_retired_items[item_id]);
            }
            return std::optional<Item_ptr>();
        }
        std::optional<Category_ptr> Manager::getCategory(CategoryID cat_id) {
            if (find_in_map<CategoryID, Category_ptr>(m_registered_categories, cat_id)) {
                return std::optional<Category_ptr>(m_registered_categories[cat_id]);
            }
            else if (find_in_map<CategoryID, Category_ptr>(m_retired_categories, cat_id)) {
                return std::optional<Category_ptr>(m_retired_categories[cat_id]);
            }
            return std::optional<Category_ptr>();
        }
        std::optional<Property_ptr> Manager::getProperty(PropertyID prop_id) {
            if (find_in_map<PropertyID, Property_ptr>(m_registered_properties, prop_id)) {
                return std::optional<Property_ptr>(m_registered_properties[prop_id]);
            }
            else if (find_in_map<PropertyID, Property_ptr>(m_retired_properties, prop_id)) {
                return std::optional<Property_ptr>(m_retired_properties[prop_id]);
            }
            return std::optional<Property_ptr>();
        }
        std::optional<Person_ptr> Manager::getPerson(PersonID person_id) {
            if (find_in_map<PersonID, Person_ptr>(m_registered_persons, person_id)) {
                return std::optional<Person_ptr>(m_registered_persons[person_id]);
            }
            else if (find_in_map<PersonID, Person_ptr>(m_retired_persons, person_id)) {
                return std::optional<Person_ptr>(m_retired_persons[person_id]);
            }
            return std::optional<Person_ptr>();
        }
        std::optional<Loan_ptr> Manager::getLoan(LoanID loan_id) {
            if (find_in_map<LoanID, Loan_ptr>(m_registered_loans, loan_id)) {
                return std::optional<Loan_ptr>(m_registered_loans[loan_id]);
            }
            else if (find_in_map<LoanID, Loan_ptr>(m_retired_loans, loan_id)) {
                return std::optional<Loan_ptr>(m_retired_loans[loan_id]);
            }
            return std::optional<Loan_ptr>();
        }

        std::optional<std::set<LoanID>> Manager::findLoans(ItemID item_id) {
            if (!m_item_loan_map.contains(item_id))
                return std::optional<std::set<LoanID>>();
            return std::optional<std::set<LoanID>>(m_item_loan_map[item_id]);
        }

        std::optional<std::set<LoanID>> Manager::findRetiredLoans(ItemID item_id) {
            if (!m_past_item_loan_map.contains(item_id))
                return std::optional<std::set<LoanID>>();
            return std::optional<std::set<LoanID>>(m_past_item_loan_map[item_id]);
        }

        ItemID Manager::createItem(Item item) {
            if (!find_in_map<CategoryID, Category_ptr>(m_registered_categories, item.category))
                return -1;
            Category_ptr cat = m_registered_categories[item.category];
            for (auto prop_id : cat->properties) {
                if (!find_in_map<PropertyID, std::string>(item.property_values, prop_id)) {
                    item.property_values[prop_id] = "";
                }
            }
            m_registered_items[(ItemID)item.id] = std::make_shared<Item>(item);
            cat->registered_items.insert(item.id);
            change();
            return (ItemID)item.id;
        }

        CategoryID Manager::createCategory(Category category) {
            std::vector<PropertyID> properties;
            for (PropertyID prop_id : category.properties) {
                if (find_in_map<PropertyID, Property_ptr>(m_registered_properties, prop_id)) {
                    properties.push_back(prop_id);
                }
                else if (find_in_map<PropertyID, Property_ptr>(m_retired_properties, prop_id)) {
                    unretireProperty(prop_id);
                    properties.push_back(prop_id);
                }
            }
            category.properties = properties;
            category.registered_items.clear();
            m_registered_categories[(CategoryID)category.id] = std::make_shared<Category>(category);
            change();
            return (CategoryID)category.id;
        }

        PropertyID Manager::createProperty(Property property) {
            m_registered_properties[(PropertyID)property.id] = std::make_shared<Property>(property);
            change();
            return (PropertyID)property.id;
        }

        // Retirement
        bool Manager::retireItem(ItemID item_id) {
            if (!exchange_btw_maps<ItemID, Item_ptr>(m_registered_items, m_retired_items, item_id))
                return false;

            std::vector<LoanID> to_retire;
            for (auto loan_id : m_item_loan_map[item_id]) {
                to_retire.push_back(loan_id);
            }
            for (auto loan_id : to_retire) {
                retireLoan(loan_id, getCurrentDate());
            }
            change();
            return true;
        }
        bool Manager::retireCategory(CategoryID cat_id) {
            bool ret = exchange_btw_maps<CategoryID, Category_ptr>(m_registered_categories, m_retired_categories, cat_id);
            if (!ret)
                return false;
            for (auto item_id : m_retired_categories[cat_id]->registered_items) {
                if (m_item_loan_map.find(item_id) != m_item_loan_map.end()) {
                    for (auto loan_id : m_item_loan_map[item_id]) {
                        retireLoan(loan_id, getCurrentDate());
                    }
                }
            }
            change();
            return true;
        }
        bool Manager::retireProperty(PropertyID prop_id) {
            change();
            return exchange_btw_maps<PropertyID, Property_ptr>(m_registered_properties, m_retired_properties, prop_id);
        }

        // Unretirement
        bool Manager::unretireItem(ItemID item_id) {
            change();
            return exchange_btw_maps<ItemID, Item_ptr>(m_retired_items, m_registered_items, item_id);
        }
        bool Manager::unretireCategory(CategoryID cat_id) {
            change();
            return exchange_btw_maps<CategoryID, Category_ptr>(m_retired_categories, m_registered_categories, cat_id);
        }
        bool Manager::unretireProperty(PropertyID prop_id) {
            change();
            return exchange_btw_maps<PropertyID, Property_ptr>(m_retired_properties, m_registered_properties, prop_id);
        }

        // Deletion
        bool Manager::deleteRetiredItem(ItemID item_id) {
            if (find_in_map<ItemID, Item_ptr>(m_retired_items, item_id)) {
                Item_ptr item = getItem(item_id).value();
                if (find_in_map<CategoryID, Category_ptr>(m_registered_categories, item->category)) {
                    m_registered_categories[item->category]->registered_items.erase(item_id);
                }
                else if (find_in_map<CategoryID, Category_ptr>(m_retired_categories, item->category)) {
                    m_retired_categories[item->category]->registered_items.erase(item_id);
                }
                m_retired_items.erase(item_id);
                m_item_loan_map.erase(item_id);
                change();
                return true;
            }
            return false;
        }
        bool Manager::deleteRetiredCategory(CategoryID cat_id) {
            if (find_in_map<CategoryID, Category_ptr>(m_retired_categories, cat_id)) {
                for (auto item_id : m_retired_categories[cat_id]->registered_items) {
                    m_retired_items.erase(item_id);
                    m_registered_items.erase(item_id);
                }
                m_retired_categories.erase(cat_id);
                change();
                return true;
            }
            return false;
        }
        bool Manager::deleteRetiredProperty(PropertyID prop_id) {
            if (find_in_map<PropertyID, Property_ptr>(m_retired_properties, prop_id)) {
                auto categories = getAllCategories();
                for (auto cat_id : categories) {
                    std::shared_ptr<Category> cat = getCategory(cat_id).value();
                    for (auto it = cat->properties.begin();it != cat->properties.end();it++) {
                        if (*it == prop_id) {
                            cat->properties.erase(it);
                            break;
                        }
                    }
                }
                m_retired_properties.erase(prop_id);
                for (auto item_pair : m_retired_items) {
                    auto item = item_pair.second;
                    if (item->property_values.contains(prop_id)) {
                        item->property_values.erase(prop_id);
                    }
                }
                for (auto item_pair : m_registered_items) {
                    auto item = item_pair.second;
                    if (item->property_values.contains(prop_id)) {
                        item->property_values.erase(prop_id);
                    }
                }
                change();
                return true;
            }
            return false;
        }

        std::optional<LoanID> Manager::new_loan(ItemID item_id, Note note, Date date, PersonID person_id) {
            Loan loan = {
                .item = item_id,
                .note = {},
                .date = date,
                .person = person_id,
            };
            m_registered_loans[loan.id] = std::make_shared<Loan>(loan);
            if (!find_in_map<ItemID, std::set<LoanID>>(m_item_loan_map, item_id)) {
                m_item_loan_map[item_id] = std::set<LoanID>();
            }
            m_item_loan_map[item_id].insert(loan.id);
            m_person_loan_map[person_id].insert(loan.id);

            // Right now, notes are going to person, not loan
            if (!note.content.empty()) {
                auto person = getPerson(person_id).value();
                person->notes.push_back(note);
            }
            change();
            return std::optional<LoanID>(loan.id);
        }

        std::optional<LoanID> Manager::newLoan(ItemID item_id, Note note, Date date, Person person) {
            std::optional<Item_ptr> item = getItem(item_id);
            if (item.has_value()) {
                m_registered_persons[person.id] = std::make_shared<Person>(person);
                change();
                return new_loan(item_id, note, date, person.id);
            }
            else {
                return std::optional<LoanID>();
            }
        }
        std::optional<LoanID> Manager::newLoan(ItemID item_id, Note note, Date date, PersonID person_id) {
            std::optional<Item_ptr> item = getItem(item_id);
            if (item.has_value()) {
                change();
                return new_loan(item_id, note, date, person_id);
            }
            else {
                return std::optional<LoanID>();
            }
        }

        bool Manager::retireLoan(LoanID loan_id, core::Date date_back) {
            exchange_btw_maps<LoanID, Loan_ptr>(m_registered_loans, m_retired_loans, loan_id);
            // if (!ret)
            //     return false;

            auto loan_ret = getLoan(loan_id);
            if (!loan_ret.has_value())
                return false;
            Loan_ptr loan = loan_ret.value();

            loan->date_back = date_back;

            if (m_item_loan_map.find(loan->item) != m_item_loan_map.end()) {
                m_item_loan_map[loan->item].erase(loan_id);
                if (m_item_loan_map[loan->item].empty()) {
                    m_item_loan_map.erase(loan->item);
                }
            }
            m_person_loan_map[loan->person].erase(loan_id);
            if (m_person_loan_map[loan->person].empty()) {
                m_person_loan_map.erase(loan->person);
                exchange_btw_maps<PersonID, Person_ptr>(m_registered_persons, m_retired_persons, loan->person);
            }

            m_past_item_loan_map[loan->item].insert(loan_id);
            m_past_person_loan_map[loan->person].insert(loan_id);

            change();
            return true;
        }

        void Manager::buildRetiredLoanCorrespondance() {
            m_past_person_loan_map.clear();
            m_past_item_loan_map.clear();
            for (auto loan_pair : m_retired_loans) {
                auto loan = loan_pair.second;
                auto person = getPerson(loan->person).value();
                auto loan_id = loan_pair.first;
                m_past_item_loan_map[loan->item].insert(loan_id);
                m_past_person_loan_map[loan->person].insert(loan_id);
            }
        }
    }
}