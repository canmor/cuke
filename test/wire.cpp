#include <gmock/gmock.h>

#include "cuke/session.h"

using ::testing::Eq;
using ::testing::StrEq;
using ::testing::MockFunction;
using ::nlohmann::json;
using cuke::step_engine;
using cuke::location;

class Wire : public ::testing::Test {
protected:
    step_engine engine;
    std::stringstream in;
    std::stringstream out;
    cuke::session session{in, out, engine};
};

TEST_F(Wire, UnknownCommand) {
    in << R"(["unknown command"])" << std::endl;
    session.run();

    std::string response;
    std::getline(out, response);
    EXPECT_THAT(response, Eq("[\"fail\"]"));
}

TEST_F(Wire, DecodeStepMatches) {
    in << R"(["step_matches", {"name_to_match":"name to match"}])" << std::endl;
    session.run();

    std::string response;
    EXPECT_THAT(std::getline(out, response), testing::IsTrue());
    EXPECT_THAT(response, Eq("[\"success\",[]]"));
}

TEST_F(Wire, DecodeStepMatchesIfMatchedOne) {
    location loc{"wire.cpp", 35};
    engine.define(R"(start calculator)", []() {}, loc);

    auto matcher = "start calculator";
    in << json::array({"step_matches", json{{"name_to_match", matcher}}}).dump() << std::endl;
    session.run();

    std::string response;
    std::getline(out, response);
    EXPECT_THAT(response, Eq(
            json::array({"success", json::array({json{
                    {"args",   json::array()},
                    {"id",     "0"},
                    {"regexp", matcher},
                    {"source", loc.to_string()}
            }})}).dump())
    );
}

TEST_F(Wire, DecodeStepMatchesIfMatchedWithArg) {
    engine.define(R"(calculator add (\d+) and (\d+))", []() {});

    auto matcher = "calculator add 100 and 200";
    in << json::array({"step_matches", json{{"name_to_match", matcher}}}).dump() << std::endl;
    session.run();

    std::string response;
    std::getline(out, response);
    EXPECT_THAT(response, Eq(R"JSON(
            [
              "success",
              [
                {
                  "args": [
                    { "pos": 15, "val": "100" },
                    { "pos": 23, "val": "200" }
                  ],
                  "id": "0",
                  "regexp": "calculator add (\\d+) and (\\d+)",
                  "source": "unknown:0"
                }
              ]
            ]
        )JSON"_json.dump()));
}

TEST_F(Wire, Scenario) {
    in << R"(["begin_scenario"])" << std::endl;
    session.run();

    std::string response;
    std::getline(out, response);
    EXPECT_THAT(response, Eq("[\"success\"]"));
}

TEST_F(Wire, handlesBeginScenarioMessageWithTagsArgument) {
    in << R"([ "begin_scenario", { "tags": [ "bar", "baz", "foo" ] } ])" << std::endl;
    session.run();

    std::string response;
    std::getline(out, response);
    EXPECT_THAT(response, Eq("[\"success\"]"));
}

TEST_F(Wire, handlesInvokeMessageWithNoArgs) {
    int invoked = 0;
    engine.define(R"(start calculator)", [&invoked]() { ++invoked; });

    in << R"(["invoke",{"id":"0","args":[]}])" << std::endl;
    session.run();

    std::string response;
    std::getline(out, response);
    EXPECT_THAT(response, Eq(R"( [ "success" ] )"_json.dump()));
    EXPECT_THAT(invoked, Eq(1));
}

TEST_F(Wire, handlesInvokeMessageWithArgs) {
    int a = 25;
    int b = 53;
    MockFunction<void(int, int)> mock;
    EXPECT_CALL(mock, Call(a, b)).Times(1);
    engine.define(R"(add (\d+) with (\d+))", mock.AsStdFunction());

    in << R"(["invoke",{"id":"0","args":[ "25", "53" ]}])" << std::endl;
    session.run();

    std::string response;
    std::getline(out, response);
    EXPECT_THAT(response, Eq(R"( [ "success" ] )"_json.dump()));
}

TEST_F(Wire, handleInvokeMessageWithDataTableArgs) {
    std::vector<std::vector<std::string>> table{
            {"name",   "email",              "twitter"},
            {"Aslak",  "aslak@cucumber.io",  "@aslak_hellesoy"},
            {"Julien", "julien@cucumber.io", "@jbpros"},
            {"Matt",   "matt@cucumber.io",   "@mattwynne"},
    };
    MockFunction<void(const std::vector<std::vector<std::string>> &table)> mock;
    EXPECT_CALL(mock, Call(Eq(table))).Times(1);
    engine.define(R"(with data table:)", mock.AsStdFunction());

    in << R"(["invoke",{"id":"0","args":[[["name","email","twitter"],)"
       << R"(["Aslak","aslak@cucumber.io","@aslak_hellesoy"],)"
       << R"(["Julien","julien@cucumber.io","@jbpros"],)"
       << R"(["Matt","matt@cucumber.io","@mattwynne"]]]}])"
       << std::endl;
    session.run();

    std::string response;
    std::getline(out, response);
    EXPECT_THAT(response, Eq(R"( [ "success" ] )"_json.dump()));
}

TEST_F(Wire, handleInvokeMessageWithStringAndDataTableArgs) {
    std::vector<std::vector<std::string>> table{
            {"name",   "email",              "twitter"},
            {"Aslak",  "aslak@cucumber.io",  "@aslak_hellesoy"},
            {"Julien", "julien@cucumber.io", "@jbpros"},
            {"Matt",   "matt@cucumber.io",   "@mattwynne"},
    };
    MockFunction<void(const std::string &, const std::vector<std::vector<std::string>> &)> mock;
    EXPECT_CALL(mock, Call(StrEq("hello"), Eq(table))).Times(1);
    engine.define(R"(with data table:)", mock.AsStdFunction());

    in << R"(["invoke",{"id":"0","args":["hello", [["name","email","twitter"],)"
       << R"(["Aslak","aslak@cucumber.io","@aslak_hellesoy"],)"
       << R"(["Julien","julien@cucumber.io","@jbpros"],)"
       << R"(["Matt","matt@cucumber.io","@mattwynne"]]]}])"
       << std::endl;
    session.run();

    std::string response;
    std::getline(out, response);
    EXPECT_THAT(response, Eq(R"( [ "success" ] )"_json.dump()));
}

TEST_F(Wire, handlesSnippet) {
    in << R"(["snippet_text"])" << std::endl;
    session.run();

    std::string response;
    std::getline(out, response);
    EXPECT_THAT(response, Eq(R"( [ "success", "given(...){}" ] )"_json.dump()));
}

struct Scenario {
    int input{0};
};

TEST_F(Wire, ShouldShareScenarioGivenDifferentStep) {
    engine.define(R"(input (\d+))", [](Scenario &scenario, int input) {
        scenario.input = input;
    });
    engine.define(R"(read input)", [](Scenario &scenario) {
        EXPECT_THAT(scenario.input, Eq(1024));
    });

    in << R"(["begin_scenario"])" << std::endl;
    in << R"(["invoke",{"id":"0","args":[ "1024" ]}])" << std::endl;
    in << R"(["invoke",{"id":"1","args":[]}])" << std::endl;
    session.run();
}

TEST_F(Wire, ShouldNotShareScenarioGivenScenarioEnded) {
    engine.define(R"(input (\d+))", [](Scenario &scenario, int input) {
        scenario.input = input;
    });
    engine.define(R"(read input)", [](Scenario &scenario) {
        EXPECT_THAT(scenario.input, Eq(0));
    });

    in << R"(["begin_scenario"])" << std::endl;
    in << R"(["invoke",{"id":"0","args":[ "1024" ]}])" << std::endl;
    in << R"(["end_scenario"])" << std::endl;
    in << R"(["begin_scenario"])" << std::endl;
    in << R"(["invoke",{"id":"1","args":[]}])" << std::endl;
    session.run();
}
