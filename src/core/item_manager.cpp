#include "item_manager.h"

#include <iostream> 
namespace core {
    namespace Item {
        ObjectID Base::ID = 0;

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
            return (CategoryID)category.id;
        }

        PropertyID Manager::createProperty(Property property) {
            m_registered_properties[(PropertyID)property.id] = std::make_shared<Property>(property);
            return (PropertyID)property.id;
        }

        // Retirement
        bool Manager::retireItem(ItemID item_id) {
            if (!exchange_btw_maps<ItemID, Item_ptr>(m_registered_items, m_retired_items, item_id))
                return false;

            std::vector<LoanID> to_retire;
            for (auto& loans : m_item_loan_map) {
                for (auto& loan_id : loans.second) {
                    retireLoan(loan_id, getCurrentDate());
                }
            }
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
            return true;
        }
        bool Manager::retireProperty(PropertyID prop_id) {
            return exchange_btw_maps<PropertyID, Property_ptr>(m_registered_properties, m_retired_properties, prop_id);
        }

        // Unretirement
        bool Manager::unretireItem(ItemID item_id) {
            return exchange_btw_maps<ItemID, Item_ptr>(m_retired_items, m_registered_items, item_id);
        }
        bool Manager::unretireCategory(CategoryID cat_id) {
            return exchange_btw_maps<CategoryID, Category_ptr>(m_retired_categories, m_registered_categories, cat_id);
        }
        bool Manager::unretireProperty(PropertyID prop_id) {
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
                return true;
            }
            return false;
        }

        std::optional<LoanID> Manager::new_loan(ItemID item_id, Note note, Date date, PersonID person_id) {
            Loan loan = {
                .item = item_id,
                .note = note,
                .date = date,
                .person = person_id,
            };
            m_registered_loans[loan.id] = std::make_shared<Loan>(loan);
            if (!find_in_map<ItemID, std::set<LoanID>>(m_item_loan_map, item_id)) {
                m_item_loan_map[item_id] = std::set<LoanID>();
            }
            m_item_loan_map[item_id].insert(loan.id);
            m_person_loan_map[person_id].insert(loan.id);
            return std::optional<LoanID>(loan.id);
        }

        std::optional<LoanID> Manager::newLoan(ItemID item_id, Note note, Date date, Person person) {
            std::optional<Item_ptr> item = getItem(item_id);
            if (item.has_value()) {
                m_registered_persons[person.id] = std::make_shared<Person>(person);
                return new_loan(item_id, note, date, person.id);
            }
            else {
                return std::optional<LoanID>();
            }
        }
        std::optional<LoanID> Manager::newLoan(ItemID item_id, Note note, Date date, PersonID person_id) {
            std::optional<Item_ptr> item = getItem(item_id);
            if (item.has_value()) {
                return new_loan(item_id, note, date, person_id);
            }
            else {
                return std::optional<LoanID>();
            }
        }

        bool Manager::retireLoan(LoanID loan_id, core::Date date_back) {
            bool ret = exchange_btw_maps<LoanID, Loan_ptr>(m_registered_loans, m_retired_loans, loan_id);
            if (!ret)
                return false;

            Loan_ptr loan = getLoan(loan_id).value();
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

            return true;
        }
    }
}