#include <gmock/gmock.h>
#include "cuke/step_hub.h"

using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::Eq;
using ::testing::DoubleEq;
using ::testing::MockFunction;
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

TEST_F(Step, StringUTF8) {
    hub.define(R"(我输入: (.+))", [](const std::string &word) {});

    auto[step, _, args] = hub.match("我输入: 你好");

    EXPECT_THAT(step, NotNull());
    EXPECT_THAT(args.position(1), Eq(5));
}

class StepWithContext : public Step {
};

struct Context {
};

TEST_F(StepWithContext, NoArg) {
    hub.define("hello", [](Context &ctx) {});

    auto[step, _, __] = hub.match("hello");

    EXPECT_THAT(step, NotNull());
}
