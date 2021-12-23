
#ifndef CUKE_STEP_H_
#define CUKE_STEP_H_

#include "scenario.h"
#include "source_location.h"

#include <gtest/gtest.h>
#include <string_view>
#include <string>
#include <utility>
#include <vector>
#include <cassert>

namespace cuke {
    struct step {
        using table_type = std::vector<std::vector<std::string>>;
        std::string_view matcher;
        location source_location;

        step(const std::string_view &matcher, const location &loc) : matcher(matcher), source_location(loc) {
            ::testing::GTEST_FLAG(throw_on_failure) = true;
        }

        virtual ~step() {
            ::testing::GTEST_FLAG(throw_on_failure) = false;
        };

    public:
        virtual int invoke(scenario_ref &scenario_ref, const std::vector<std::string> &parameters) { return 0; };

        virtual int invoke(scenario_ref &scenario_ref, table_type &table) { return 0; };
    };

    template<typename T>
    struct is_context : std::true_type {
    };
    template<typename T>
    inline constexpr bool is_context_v = is_context<T>::value;
    template<>
    struct is_context<int> : std::false_type {
    };
    template<>
    struct is_context<double> : std::false_type {
    };
    template<>
    struct is_context<std::string> : std::false_type {
    };
    template<>
    struct is_context<std::vector<std::vector<std::string>>> : std::false_type {
    };

    template<typename Type>
    static Type to_value(const std::string &input) {
        if constexpr(std::is_same_v<int, Type>) {
            // fixme: handle exception
            return std::stoi(input);
        } else if constexpr(std::is_same_v<double, Type>) {
            // fixme: handle exception
            return std::stod(input);
        } else {
            return Type(input);
        }
    }

    struct step_without_args : public step {
        using handler_type = std::function<void()>;
        handler_type handler;

        step_without_args(std::string_view matcher, location loc, handler_type handler)
                : step(matcher, loc), handler(std::move(handler)) {}

        int invoke(scenario_ref &scenario_ref, const std::vector<std::string> &parameters) override {
            handler();
            return 0;
        }
    };

    template<typename FirstType, typename ...ArgTypes>
    struct step_with_args : public step {
        using handler_type = std::function<void(FirstType, ArgTypes...)>;
        handler_type handler;

        step_with_args(std::string_view matcher, location loc, handler_type handler)
                : step(matcher, loc), handler(std::move(handler)) {}

        template<typename T, std::size_t... I>
        auto invoke_handler_with_scenario(T &scenario, const std::vector<std::string> &parameters,
                                          std::index_sequence<I...>) {
            return handler(scenario, to_value<std::remove_reference_t<ArgTypes>>(parameters[I])...);
        }

        template<std::size_t... I>
        auto invoke_handler(const std::vector<std::string> &parameters, std::index_sequence<I...>) {
            return handler(to_value<std::remove_reference_t<FirstType>>(parameters[0]),
                           to_value<std::remove_reference_t<ArgTypes>>(parameters[I + 1])...);
        }

        int invoke(scenario_ref &scenario_ref, const std::vector<std::string> &parameters) override {
            using RawFirstType = std::remove_const_t<std::remove_reference_t<FirstType>>;
            if constexpr(is_context<RawFirstType>::value) {
                assert(parameters.size() == sizeof...(ArgTypes) && "parameters count not match");
                auto &scenario = scenario_ref.ensure<RawFirstType>();
                invoke_handler_with_scenario(scenario, parameters, std::index_sequence_for<ArgTypes...>{});
            } else {
                assert(parameters.size() == sizeof...(ArgTypes) + 1 && "parameters count not match");
                invoke_handler(parameters, std::index_sequence_for<ArgTypes...>{});
            }
            return 0;
        }
    };

    template<typename FirstType, typename ...SecondTypes>
    struct step_with_table_args : public step {
        using handler_type = std::function<void(FirstType, SecondTypes...)>;
        handler_type handler;

        step_with_table_args(std::string_view matcher, location loc, handler_type handler)
                : step(matcher, loc), handler(std::move(handler)) {}

        int invoke(scenario_ref &scenario_ref, step::table_type &table) override {
            using first_raw_type = std::remove_const_t<std::remove_reference_t<FirstType>>;
            if constexpr(is_context<first_raw_type>::value) {
                static_assert(sizeof...(SecondTypes) == 1, "a data table parameter is required");
                using second_type = std::tuple_element_t<0, std::tuple<SecondTypes...>>;
                using second_raw_type = std::remove_const_t<std::remove_reference_t<second_type>>;
                static_assert(std::is_same_v<second_raw_type, table_type>, "parameters type not a data table");
                auto &scenario = scenario_ref.ensure<first_raw_type>();
                handler(scenario, table);
            } else {
                static_assert(sizeof...(SecondTypes) == 0, "a data table parameter is required");
                static_assert(std::is_same_v<first_raw_type, table_type>, "parameters type not a data table");
                handler(table);
            }
            return 0;
        }
    };

    template<typename T,
            typename std::enable_if<!is_context_v<std::remove_const_t<std::remove_reference_t<T>>>, int>::type = 0,
            typename... Types>
    std::unique_ptr<step>
    make_step(std::string_view desc, std::function<void(T, Types...)> callback, location location) {
        using first_raw_type = std::remove_const_t<std::remove_reference_t<T>>;
        static_assert(!is_context<first_raw_type>::value, "wrong template dispatch");
        using step_type = std::conditional_t<std::is_same_v<first_raw_type, step::table_type>,
                step_with_table_args<T, Types...>, step_with_args<T, Types...>>;
        return std::make_unique<step_type>(desc, location, callback);
    }

    template<typename T,
            typename std::enable_if<is_context_v<std::remove_const_t<std::remove_reference_t<T>>>, int>::type = 0,
            typename... Types>
    std::unique_ptr<step>
    make_step(std::string_view desc, std::function<void(T, Types...)> callback, location location) {
        using first_raw_type = std::remove_const_t<std::remove_reference_t<T>>;
        static_assert(is_context<first_raw_type>::value, "wrong template dispatch");
        if constexpr(sizeof...(Types) > 0) {
            using first_arg_type = std::tuple_element_t<0, std::tuple<Types...>>;
            using first_arg_raw_type = std::remove_const_t<std::remove_reference_t<first_arg_type>>;
            if constexpr(std::is_same_v<first_arg_raw_type, step::table_type>) {
                return std::make_unique<step_with_table_args<T, Types...>>(desc, location, callback);
            } else {
                return std::make_unique<step_with_args<T, Types...>>(desc, location, callback);
            }
        } else {
            return std::make_unique<step_with_args<T, Types...>>(desc, location, callback);
        }
    }

    template<typename T = void>
    auto make_step(std::string_view desc, std::function<void()> callback, location location) {
        return std::make_unique<step_without_args>(desc, location, callback);
    }
}

#endif //CUKE_STEP_H_
