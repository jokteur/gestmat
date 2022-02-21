#include "with.h"

namespace pybind11 {

    namespace detail {

        void translate_exception_to_python(std::exception_ptr last_exception) {
            auto& registered_exception_translators = get_internals().registered_exception_translators;
            for (auto& translator : registered_exception_translators) {
                try {
                    translator(last_exception);
                }
                catch (...) {
                    last_exception = std::current_exception();
                    continue;
                }
                throw error_already_set();
            }
            // Not a Python error
            std::rethrow_exception(last_exception);
        }
    }
}