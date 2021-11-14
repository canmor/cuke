
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
        std::string_view matcher;
        location source_location;

        step(const std::string_view &matcher, const location &loc) : matcher(matcher), source_location(loc) {
            ::testing::GTEST_FLAG(throw_on_failure) = true;
        }

        virtual ~step() {
            ::testing::GTEST_FLAG(throw_on_failure) = false;
        };

    public:
        virtual int invoke(scenario_ref &scenario_ref, const std::vector<std::string> &parameters) = 0;
    };

    template<class T>
    struct is_context : std::true_type {
    };
    template<>
    struct is_context<int> : std::false_type {
    };
    template<>
    struct is_context<double> : std::false_type {
    };
    template<>
    struct is_context<std::string> : std::false_type {
    };

    template<typename Type>
    static Type to_value(const std::string &input) {
        if constexpr(std::is_same_v<int, Type>) {
            return std::stoi(input);
        } else if constexpr(std::is_same_v<double, Type>) {
            return std::stod(input);
        } else {
            return Type{input};
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

        int invoke(scenario_ref &scenario_ref, const std::vector<std::string> &parameters) override {
            using RawFirstType = std::remove_const_t<std::remove_reference_t<FirstType>>;
            if constexpr(is_context<RawFirstType>::value) {
                auto &scenario = scenario_ref.ensure<RawFirstType>();
                auto first = parameters.begin();
                assert(parameters.size() == sizeof...(ArgTypes));
                handler(scenario, to_value<std::remove_reference_t<ArgTypes>>(*first++)...);
            } else {
                assert(parameters.size() == sizeof...(ArgTypes) + 1);
                auto first = parameters.begin();
                auto second = parameters.begin() + 1;
                handler(to_value<FirstType>(*first), to_value<std::remove_reference_t<ArgTypes>>(*second++)...);
            }
            return 0;
        }
    };

    template<typename... types>
    auto make_step(std::string_view desc, std::function<void(types...)> callback, location location) {
        if constexpr(sizeof...(types) == 0) {
            return std::make_unique<step_without_args>(desc, location, callback);
        } else {
            return std::make_unique<step_with_args<types...>>(desc, location, callback);
        }
    }
}

#endif //CUKE_STEP_H_
