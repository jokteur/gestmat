#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <ctime>
#include <memory>
#include <chrono>
#include <climits>

#include "util.h"
#include <nlohmann/json.hpp>

namespace core {
    namespace Item {
        using namespace nlohmann;
        typedef long long int ObjectID;
        typedef ObjectID NoteID;
        typedef ObjectID ItemID;
        typedef ObjectID CategoryID;
        typedef ObjectID PropertyID;
        typedef ObjectID LoanID;
        typedef ObjectID PersonID;
        /**
         * @brief Base class is for uuid inventory (item, category, property,...)
         *
         */
        class Base {
        protected:
            static ObjectID ID;
            static std::set<ObjectID> _ids;
        public:
            ObjectID id = 0;
            Base() {
                while (_ids.contains(ID)) {
                    ID++;
                    if (ID >= LLONG_MAX - 10)
                        ID = 0;
                }
                id = ID;
                _ids.insert(id);
            }

            static std::set<ObjectID>& getIDs() {
                return _ids;
            }

            static void addID(ObjectID id) {
                _ids.insert(id);
            }

            static ObjectID getID() {
                return ID;
            }

            static void setID(int id) {
                ID = id;
            }
        };

        /**
         * @brief A note is a written remark that is timestamped and can be
         * assigned to any item
         *
         */
        struct Note : public Base {
            std::string content;
            long long int timestamp;
        };


        struct Property : public Base {
            std::string name;
            bool mandatory;
            bool no_edit;
            std::vector<std::string> select;
        };

        // Forward declaration
        struct Item;

        struct Category : public Base {
            std::string name;
            std::vector<PropertyID> properties;
            std::set<ItemID> registered_items;
            std::set<PropertyID> properties_hide;
        };
        struct Item : public Base {
            CategoryID category = -1;
            std::vector<Note> notes;
            std::map<PropertyID, std::string> property_values;
        };

        struct Loan;
        /**
         * @brief Loaning structure
         * For a loan, we need one person and a loan
         *
         */
        struct Person : public Base {
            std::string name;
            std::string surname;
            std::string place;
            std::vector<Note> notes;
            core::Date birthday;
        };
        struct Loan : public Base {
            ItemID item;
            Note note;
            core::Date date;
            core::Date date_back;
            PersonID person;
            core::Date remainder_date;
        };

        typedef std::shared_ptr<Item> Item_ptr;
        typedef std::shared_ptr<Category> Category_ptr;
        typedef std::shared_ptr<Property> Property_ptr;
        typedef std::shared_ptr<Person> Person_ptr;
        typedef std::shared_ptr<Loan> Loan_ptr;

        class Manager {
        private:
            bool m_changed = false;

            std::map<long long int, bool> m_subscriber_ids;
            long long int m_subscriber_counter = 0;

            std::map<ItemID, std::set<LoanID>> m_item_loan_map;
            std::map<ItemID, std::set<LoanID>> m_past_item_loan_map;
            std::map<PersonID, std::set<LoanID>> m_person_loan_map;
            std::map<PersonID, std::set<LoanID>> m_past_person_loan_map;

            std::map<ItemID, Item_ptr> m_registered_items;
            std::map<CategoryID, Category_ptr> m_registered_categories;
            std::map<PropertyID, Property_ptr> m_registered_properties;
            std::map<PersonID, Person_ptr> m_registered_persons;
            std::map<LoanID, Loan_ptr> m_registered_loans;

            std::map<ItemID, Item_ptr> m_retired_items;
            std::map<CategoryID, Category_ptr> m_retired_categories;
            std::map<PropertyID, Property_ptr> m_retired_properties;
            std::map<PersonID, Person_ptr> m_retired_persons;
            std::map<LoanID, Loan_ptr> m_retired_loans;

            std::optional<LoanID> new_loan(ItemID item_id, Note note, Date date, PersonID person_id);

            Duration m_duration_before_alert = TWO_MONTHS;

            void change();

            void buildIDSet();

            friend class Workspace;
            friend class Debug::Debugger;
        public:
            Manager() = default;

            Manager(const Manager& other);

            /**
             * @brief Returns true if retired, false if not
             * optional is empty when the object has not been found
             *
             * @param id
             * @return std::optional<bool>
             */
            std::optional<bool> isRetired(ObjectID id);

            /**
             * @brief Returns if something changed
             * This function will return false after being
             * called once until the next change
             * @param id
             * @return true
             * @return false
             */
            bool isChanged(long long int id);

            void setChange() { change(); }

            long long int getId() { m_subscriber_counter++; return m_subscriber_counter; }
            void giveBackId(long long int id);

            Duration getDuration() { return m_duration_before_alert; }
            void setDuration(Duration duration) { m_duration_before_alert = duration; }

            void findDuplicates();

            /**
             * @brief This function is solely to clean up the mess introduced
             * by previous bugs that can still lurk in the history of the loans.
             * It is a good idea to call this function after a new load
             * (in workspace)
             */
            void cleanUp();

            // Returns all categories, retired or not
            std::set<CategoryID> getAllCategories();
            // Returns all properties, retired or not
            std::set<PropertyID> getAllProperties();
            std::set<LoanID> getAllLoans();
            std::vector<ItemID> getAllItems();
            std::set<LoanID> getActiveLoans();

            std::optional<Item_ptr> getItem(ItemID item_id);
            std::optional<Category_ptr> getCategory(CategoryID cat_id);
            std::optional<Property_ptr> getProperty(PropertyID prop_id);
            std::optional<Person_ptr> getPerson(PersonID person_id);
            std::optional<Loan_ptr> getLoan(LoanID loan_id);

            bool isLoaned(ItemID item_id);

            std::optional<std::set<LoanID>> findLoans(ItemID);
            std::optional<std::set<LoanID>> findRetiredLoans(ItemID);

            std::optional<PropertyID> findPropertyByName(const std::string& name);
            std::optional<CategoryID> findCategoryByName(const std::string& name);

            ItemID createItem(Item item);
            // Does not check if name already exists by name
            CategoryID createCategory(Category category);
            // Does not check if property already exists by name
            PropertyID createProperty(Property property);

            bool retireItem(ItemID item_id);
            bool retireCategory(CategoryID cat_id);
            bool retireProperty(PropertyID prop_id);

            bool unretireItem(ItemID item_id);
            bool unretireCategory(CategoryID cat_id);
            bool unretireProperty(PropertyID prop_id);

            bool deleteRetiredItem(ItemID item_id);
            bool deleteRetiredCategory(CategoryID cat_id);
            bool deleteRetiredProperty(PropertyID prop_id);

            std::optional<LoanID> newLoan(ItemID item_id, Note note, Date date, Person person);
            std::optional<LoanID> newLoan(ItemID item_id, Note note, Date date, PersonID person_id);
            bool retireLoan(LoanID loan_id, core::Date date_back);
            bool unretireLoan(LoanID loan_id);

            void buildRetiredLoanCorrespondance();
        };
    }  // namespace Item
}  // namespace core