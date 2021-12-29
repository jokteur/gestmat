#include <cstdint>
#include <string>

namespace core {

    namespace Item {
        class Person {
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