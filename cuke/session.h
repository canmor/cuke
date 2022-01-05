
#ifndef CUKE_SESSION_H_
#define CUKE_SESSION_H_

#include <asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>
#include "step_engine.h"
#include "unicode.h"

namespace cuke {
    class session {
        using json = nlohmann::json;
    public:
        session(std::istream &in, std::ostream &out, step_engine &engine) : in(in), out(out), engine(engine) {}

        void run() {
            std::string line;
            while (getline(in, line)) {
                auto request = json::parse(line);
                auto command = request[0];
                if (command == "step_matches") {
                    out << handle_step_matches(request[1]) << std::endl << std::flush;
                    continue;
                } else if (command == "begin_scenario") {
                    engine.begin_scenario();
                    out << json::array({"success"}) << std::endl << std::flush;
                    continue;
                } else if (command == "end_scenario") {
                    engine.end_scenario();
                    out << json::array({"success"}) << std::endl << std::flush;
                    continue;
                } else if (command == "invoke") {
                    out << handle_invoke(request[1]) << std::endl << std::flush;
                    continue;
                } else if (command == "snippet_text") {
                    auto type = "success";
                    out << json::array({type, "given(...){}"}) << std::endl << std::flush;
                    continue;
                }
                out << json::array({"fail"}) << std::endl << std::flush;
            }
        }

    private:
        json handle_step_matches(const json &command_detail) {
            std::string name = command_detail["name_to_match"];
            // it's important to keep unicode name alive to prevent UB while reading args later
            const auto unicode_name = to_wstring(name);
            auto[step, id, args] = engine.match(unicode_name);
            auto result = json::array();
            if (step) {
                json match_response;
                auto args_response = json::array();
                for (int i = 1; i < args.size(); ++i) {
                    args_response.push_back(json{
                            {"pos", args.position(i)},
                            {"val", to_utf8(static_cast<std::wstring>(args[i]))},
                    });
                }
                match_response["args"] = args_response;
                match_response["id"] = std::to_string(id);
                match_response["regexp"] = step->matcher;
                match_response["source"] = step->source_location.to_string();
                result.push_back(match_response);
            }
            auto type = "success";
            return json::array({type, result});
        }

        json handle_invoke(const json &command_detail) {
            std::string id = command_detail["id"];
            auto step_id = std::stoul(id);
            try {
                std::vector<std::string> regular_args;
                step::table_type table;
                std::vector<json> args = command_detail["args"];
                if (!args.empty()) {
                    auto last = args.end();
                    if (!args.back().is_string()) {
                        step::table_type tmp = args.back();
                        table = std::move(tmp);
                        --last;
                    }
                    regular_args.assign(args.begin(), last);
                }
                engine.run(step_id, regular_args, table);
            } catch (const std::runtime_error &error) {
                auto type = "fail";
                json result{{"exception", ""},
                            {"message",   error.what()}};
                return json::array({type, result});
            } catch (const std::exception &error) {
                auto type = "fail";
                json result{{"exception", ""},
                            {"message",   std::string("unknown exception: ") + error.what()}};
                return json::array({type, result});
            }
            auto type = "success";
            return json::array({type});
        }

        std::istream &in;
        std::ostream &out;
        step_engine &engine;
    };
}

#endif //CUKE_SESSION_H_
