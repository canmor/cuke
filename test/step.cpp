#include <gmock/gmock.h>
#include "cuke/step_hub.h"

using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::Eq;
using ::testing::StrEq;
using ::testing::DoubleEq;
using ::testing::MockFunction;
using ::testing::_;
using cuke::step_engine;

class Step : public ::testing::Test {
protected:
    step_engine hub;
};

TEST_F(Step, NoArg) {
    hub.define("hello", []() {});

    auto[step, _, __] = hub.match(L"hello");

    EXPECT_THAT(step, NotNull());
}

TEST_F(Step, StringArg) {
    hub.define(R"(say: (\w+))", [](const std::string &word) {});

    auto[step, _, __] = hub.match(L"say: hello");

    EXPECT_THAT(step, NotNull());
}

TEST_F(Step, RunNoArg) {
    MockFunction<void()> mock;
    EXPECT_CALL(mock, Call()).Times(1);
    hub.define("hello", mock.AsStdFunction());

    hub.run(0, {"hello"});
}

TEST_F(Step, IntArg) {
    MockFunction<void(int)> mock;
    EXPECT_CALL(mock, Call(1)).Times(1);
    hub.define(R"(input (\d+))", mock.AsStdFunction());

    hub.run(0, {"1"});
}

TEST_F(Step, DoubleArg) {
    MockFunction<void(double)> mock;
    EXPECT_CALL(mock, Call(DoubleEq(3.14))).Times(1);

    hub.define(R"(input double ([\d\.]+))", mock.AsStdFunction());

    auto matched = hub.run(0, {"3.14"});
}

TEST_F(Step, DoubleAndIntArg) {
    MockFunction<void(double, int)> mock;
    EXPECT_CALL(mock, Call(DoubleEq(3.14), Eq(300))).Times(1);

    hub.define(R"(input double ([\d\.]+), int (\d+))", mock.AsStdFunction());

    auto matched = hub.run(0, {"3.14", "300"});
}

TEST_F(Step, StringAndIntArg) {
    MockFunction<void(const std::string &, double)> mock;
    EXPECT_CALL(mock, Call(StrEq("hello world"), Eq(300))).Times(1);

    hub.define(R"(input string (\w+), double (\d+\.?\d*))", mock.AsStdFunction());

    auto matched = hub.run(0, {"hello world", "300"});
}

TEST_F(Step, DataTable) {
    MockFunction<void(const std::vector<std::vector<std::string>> &)> mock;
    std::vector<std::vector<std::string>> args{
            {"key1",   "key2"},
            {"value1", "value2"},
            {"valueA", "valueB"}
    };
    EXPECT_CALL(mock, Call(args)).Times(1);

    hub.define(R"(step with data table)", mock.AsStdFunction());

    auto matched = hub.run(0, {}, args);
}

TEST_F(Step, IntAndDataTable) {
    MockFunction<void(int, const std::vector<std::vector<std::string>> &)> mock;
    std::vector<std::vector<std::string>> args{
            {"key1",   "key2"},
            {"value1", "value2"},
            {"valueA", "valueB"}
    };
    EXPECT_CALL(mock, Call(1024, args)).Times(1);

    hub.define(R"(step input: (\d+) and data table)", mock.AsStdFunction());

    auto matched = hub.run(0, {"1024"}, args);
}

TEST_F(Step, StringUTF8) {
    hub.define(R"(我输入: (.+))", [](const std::string &word) {});

    auto[step, _, args] = hub.match(L"我输入: 你好");

    EXPECT_THAT(step, NotNull());
    EXPECT_THAT(args.position(1), Eq(5));
}

class StepWithContext : public Step {
};

struct Context {
};

TEST_F(StepWithContext, NoArg) {
    hub.define("hello", [](Context &ctx) {});

    auto[step, _, __] = hub.match(L"hello");

    EXPECT_THAT(step, NotNull());
}

TEST_F(StepWithContext, StringAndIntArg) {
    MockFunction<void(Context &ctx, const std::string &, double)> mock;
    EXPECT_CALL(mock, Call(_, StrEq("hello world"), Eq(300))).Times(1);

    hub.define(R"(input string (\w+), double (\d+\.?\d*))", mock.AsStdFunction());

    auto matched = hub.run(0, {"hello world", "300"});
}

TEST_F(StepWithContext, DataTable) {
    MockFunction<void(Context &ctx, const std::vector<std::vector<std::string>> &)> mock;
    std::vector<std::vector<std::string>> args{
            {"key1",   "key2"},
            {"value1", "value2"},
            {"valueA", "valueB"}
    };
    EXPECT_CALL(mock, Call(_, args)).Times(1);

    hub.define(R"(step with data table)", mock.AsStdFunction());

    auto matched = hub.run(0, {}, args);
}

TEST_F(StepWithContext, IntAndDataTable) {
    MockFunction<void(Context &ctx, int, const std::vector<std::vector<std::string>> &)> mock;
    std::vector<std::vector<std::string>> args{
            {"key1",   "key2"},
            {"value1", "value2"},
            {"valueA", "valueB"}
    };
    EXPECT_CALL(mock, Call(_, 1024, args)).Times(1);

    hub.define(R"(step input: (\d+) and data table)", mock.AsStdFunction());

    auto matched = hub.run(0, {"1024"}, args);
}
