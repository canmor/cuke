
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
        virtual int invoke(scenario_ref &scenario_ref, const std::vector<std::string> &parameters,
                           const table_type &table) { return 0; };
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

        int invoke(scenario_ref &scenario_ref, const std::vector<std::string> &parameters,
                   const table_type &table) override {
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
                                          const step::table_type &table,
                                          std::index_sequence<I...>) {
            if constexpr(sizeof...(ArgTypes) > 0) {
                using first_arg_type = std::tuple_element_t<0, std::tuple<ArgTypes...>>;
                using first_arg_raw_type = std::remove_const_t<std::remove_reference_t<first_arg_type>>;
                if constexpr(std::is_same_v<first_arg_raw_type, step::table_type>) {
                    return handler(scenario, const_cast<step::table_type &>(table));
                } else {
                    return handler(scenario, to_value<std::remove_reference_t<ArgTypes>>(parameters[I])...);
                }
            } else {
                return handler(scenario, to_value<std::remove_reference_t<ArgTypes>>(parameters[I])...);
            }
        }

        template<std::size_t... I>
        auto invoke_handler(const std::vector<std::string> &parameters,
                            const step::table_type &table,
                            std::index_sequence<I...>) {
            using RawFirstType = std::remove_const_t<std::remove_reference_t<FirstType>>;
            if constexpr(std::is_same_v<RawFirstType, step::table_type>) {
                return handler(const_cast<step::table_type &>(table));
            } else {
                return handler(to_value<std::remove_reference_t<FirstType>>(parameters[0]),
                               to_value<std::remove_reference_t<ArgTypes>>(parameters[I + 1])...);
            }
        }

        int invoke(scenario_ref &scenario_ref, const std::vector<std::string> &parameters,
                   const table_type &table) override {
            using RawFirstType = std::remove_const_t<std::remove_reference_t<FirstType>>;
            if constexpr(is_context<RawFirstType>::value) {
                auto &scenario = scenario_ref.ensure<RawFirstType>();
                invoke_handler_with_scenario(scenario, parameters, table, std::index_sequence_for<ArgTypes...>{});
            } else {
                invoke_handler(parameters, table, std::index_sequence_for<ArgTypes...>{});
            }
            return 0;
        }
    };

    template<typename... Types>
    std::unique_ptr<step>
    make_step(std::string_view desc, std::function<void(Types...)> callback, location location) {
        if constexpr(sizeof...(Types) > 0) {
            return std::make_unique<step_with_args<Types...>>(desc, location, callback);
        } else {
            return std::make_unique<step_without_args>(desc, location, callback);
        }
    }
}

#endif //CUKE_STEP_H_
