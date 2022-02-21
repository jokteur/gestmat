#pragma once
#include <type_traits>

#include "py_api.h"

namespace pybind11 {

    namespace detail {

        void translate_exception_to_python(std::exception_ptr last_exception);

        template <typename F, typename SFINAE = void>
        struct takes_object_rval_ref_argument : std::false_type {};

        template <typename F>
        struct takes_object_rval_ref_argument<F, decltype(std::declval<F>()(std::declval<object>()))> : std::true_type {};


        template <typename F, typename SFINAE = void>
        struct takes_object_lval_ref_argument : std::false_type {};

        template <typename F>
        struct takes_object_lval_ref_argument<F, decltype(std::declval<F>()(std::declval<object&>()))> : std::true_type {};


        template <typename F, typename SFINAE = void>
        struct takes_no_arguments : std::false_type {};

        template <typename F>
        struct takes_no_arguments<F, decltype(std::declval<F>()())> : std::true_type {};

    }

    // PEP 343 specification: https://www.python.org/dev/peps/pep-0343/#specification-the-with-statement

    enum class exception_policy {
        cascade,
        translate_to_python
    };

    template <typename Block, typename std::enable_if<detail::takes_object_rval_ref_argument<Block>::value>::type* = nullptr>
    void with(const object& mgr, Block&& block, exception_policy policy = exception_policy::translate_to_python) {
        object exit = mgr.attr("__exit__");
        object value = mgr.attr("__enter__")();
        // bool exc = true;

        std::exception_ptr original_exception = nullptr;

        try {
            try {
                std::forward<Block>(block)(std::move(value));
            }
            catch (const error_already_set&) {
                // If already a Python error, catch in the outer try-catch
                original_exception = std::current_exception();
                throw;
            }
            catch (...) {
                // Else, try our best to translate the error into a Python error before calling mrg.__exit__
                original_exception = std::current_exception();
                if (policy == exception_policy::translate_to_python)
                    detail::translate_exception_to_python(std::current_exception());
                else
                    throw;
            }
        }
        catch (const error_already_set&) {
            // A Python error
            // auto exit_result = exit(e.exc_type() ? e.exc_type() : none(),
            //     e.exc_value() ? e.exc_value() : none(),
            //     e.exc_trace() ? e.exc_trace() : none());
            // if (!bool_(std::move(exit_result)))
            std::rethrow_exception(original_exception);
        }
        catch (...) {
            // Not a Python error
            exit(none(), none(), none());
            std::rethrow_exception(original_exception);
        }

        exit(none(), none(), none());
    }

    template <typename Block, typename std::enable_if<!detail::takes_object_rval_ref_argument<Block>::value&&
        detail::takes_object_lval_ref_argument<Block>::value>::type* = nullptr>
        inline void with(const object& mgr, Block&& block) {
        with(mgr, [&block](object&& o) { block(o); });
    }

    template <typename Block, typename std::enable_if<detail::takes_no_arguments<Block>::value>::type* = nullptr>
    inline void with(const object& mgr, Block&& block) {
        with(mgr, [&block](object&&) { block(); });
    }

}