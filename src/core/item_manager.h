#include <cstdint>
#include <string>

#include "util.h"

namespace core {
    namespace Item {
        class Base {
        protected:
            int m_id = 0;
        public:
            static int ID;
            Base() {
                m_id = ID;
                ID++;
            }
        };
        class Property : Base {

        };
        class PropertyValue : Base {

        };
        class Category : Base {

        };
        class Item : Base {

        };

        int Base::ID = 0;

        struct Person : Base {
            std::string m_name;
            std::string m_surname;
            std::string m_place;
            std::string m_note;
            Date m_birthday;

            /**
             * @brief Construct a new Person object
             *
             * @param name
             * @param surname
             * @param place
             * @param note
             * @param birthday
             */
            Person(std::string name = "", std::string surname = "", std::string place = "", std::string note = "", Date birthday = Date());
        };
        class Loan : Base {

        };
        class Manager {
        private:
            Manager() = default;

        public:
            /**
                     * Copy constructors stay empty, because of the Singleton
                     */
            Manager(Manager const&) = delete;

            void operator=(Manager const&) = delete;

            /**
                     * @return instance of the Singleton of the Project Manager
                     */
            static Manager& getInstance() {
                static Manager instance;
                return instance;
            }
        };
    }  // namespace Item
}  // namespace core