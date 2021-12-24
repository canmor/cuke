
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
    inline constexpr bool is_context_v = is_context<std::remove_const_t<std::remove_reference_t<T>>>::value;
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

    template<typename T>
    struct is_table : std::false_type {
    };
    template<typename T>
    inline constexpr bool is_table_v = is_table<std::remove_const_t<std::remove_reference_t<T>>>::value;
    template<>
    struct is_table<step::table_type> : std::true_type {
    };

    template<typename T, typename U = std::remove_const_t<std::remove_reference_t<T>>>
    static U to_value(const std::string &input) {
        if constexpr(std::is_same_v<int, U>) {
            // fixme: handle exception
            return std::stoi(input);
        } else if constexpr(std::is_same_v<double, U>) {
            // fixme: handle exception
            return std::stod(input);
        } else {
            return U(input);
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

        int invoke(scenario_ref &scenario_ref, const std::vector<std::string> &parameters,
                   const table_type &table) override {
            if constexpr(is_context_v<FirstType>) {
                auto &scenario = scenario_ref.ensure<FirstType>();
                invoke_handler_with_scenario(scenario, parameters, table, std::index_sequence_for<ArgTypes...>{});
            } else {
                invoke_handler(parameters, table, std::index_sequence_for<ArgTypes...>{});
            }
            return 0;
        }

    private:
        template<typename T, std::size_t... I>
        auto invoke_handler_with_scenario_and_table(T &scenario, const std::vector<std::string> &parameters,
                                                    const step::table_type &table,
                                                    std::index_sequence<I...>) {
            using args_type = std::tuple<ArgTypes...>;
            return handler(scenario,
                           to_value<std::tuple_element_t<I, args_type>>(parameters[I])...,
                           const_cast<step::table_type &>(table));
        }

        template<typename T, std::size_t... I>
        auto invoke_handler_with_scenario(T &scenario, const std::vector<std::string> &parameters,
                                          const step::table_type &table,
                                          std::index_sequence<I...>) {
            if constexpr(sizeof...(ArgTypes) > 0) {
                using first_arg_type = std::tuple_element_t<0, std::tuple<ArgTypes...>>;
                if constexpr(is_table_v<first_arg_type>) {
                    return handler(scenario, const_cast<step::table_type &>(table));
                } else {
                    using last_arg_type = decltype((std::declval<ArgTypes>(), ...));
                    if constexpr(is_table_v<last_arg_type>) {
                        return invoke_handler_with_scenario_and_table(scenario, parameters, table,
                                                                      std::make_index_sequence<
                                                                              sizeof...(ArgTypes) - 1>{});
                    } else {
                        return handler(scenario, to_value<ArgTypes>(parameters[I])...);
                    }
                }
            } else {
                return handler(scenario, to_value<ArgTypes>(parameters[I])...);
            }
        }

        template<std::size_t... I>
        auto invoke_handler_with_table(const std::vector<std::string> &parameters,
                                       const step::table_type &table,
                                       std::index_sequence<I...>) {
            using args_type = std::tuple<ArgTypes...>;
            return handler(to_value<FirstType>(parameters[0]),
                           to_value<std::tuple_element_t<I, args_type>>(parameters[I + 1])...,
                           const_cast<step::table_type &>(table));
        }

        template<std::size_t... I>
        auto invoke_handler(const std::vector<std::string> &parameters,
                            const step::table_type &table,
                            std::index_sequence<I...>) {
            if constexpr(is_table_v<FirstType>) {
                return handler(const_cast<step::table_type &>(table));
            } else {
                using last_arg_type = decltype((std::declval<ArgTypes>(), ...));
                if constexpr(is_table_v<last_arg_type>) {
                    return invoke_handler_with_table(parameters, table,
                                                     std::make_index_sequence<sizeof...(ArgTypes) - 1>{});
                } else {
                    return handler(to_value<FirstType>(parameters[0]), to_value<ArgTypes>(parameters[I + 1])...);
                }
            }
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
