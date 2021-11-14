
#ifndef CUKE_STEP_ENGINE_H_
#define CUKE_STEP_ENGINE_H_

#include "step.h"
#include "unicode.h"
#include <regex>

namespace cuke {
    class step_engine {
    public:
        std::tuple<step *, size_t, std::wcmatch> match(std::wstring_view desc) {
            std::wcmatch matches;
            auto found = std::find_if(steps.begin(), steps.end(), [&matches, desc](const auto &step) {
                const std::wstring s = to_wstring(step->matcher);
                std::wregex step_regex{s};
                return std::regex_match(desc.begin(), desc.end(), matches, step_regex);
            });
            if (found != steps.end()) {
                auto id = std::distance(steps.begin(), found);
                return {&**found, id, matches};
            }
            return {};
        }

        std::tuple<step *, size_t, std::wcmatch> match(std::string_view desc) {
            const std::wstring input = to_wstring(desc);
            return match(std::wstring_view{input});
        }

        bool run(size_t id, const std::vector<std::string> &args) {
            auto step = get(id);
            if (step) {
                step->invoke(scenario, args);
                return true;
            }
            return false;
        }

        template<typename... types>
        int define(std::string_view desc, std::function<void(types...)> callback, location location) {
            steps.push_back(make_step(desc, callback, location));
            return 0;
        }

        template<typename Callable>
        int define(std::string_view desc, Callable callback, location loc = location{}) {
            return define(desc, std::function{callback}, loc);
        }

        void begin_scenario() {
            scenario = scenario_ref{};
        }

        void end_scenario() {
            scenario.reset();
        }

    private:
        step *get(size_t id) {
            return id < steps.size() ? &*steps[id] : nullptr;
        }

        std::vector<std::unique_ptr<step>> steps;
        scenario_ref scenario;
    };
}

#endif //CUKE_STEP_ENGINE_H_
