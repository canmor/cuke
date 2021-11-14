#include <cuke/cuke.h>
#include <gmock/gmock.h>

using cuke::given;
using cuke::when;
using cuke::then;

struct Calculator {
    std::vector<double> digits;
    double result{0};
};

void calculator_steps() {
    given(LOCATION(), "I have entered ([\\d\\.]+) into the calculator", [](Calculator &self, double i) {
        self.digits.push_back(i);
    });

    when(LOCATION(), "I press (\\w+)", [](Calculator &self, const std::string& operation) {
        if (operation == "add") {
            self.result = self.digits[0] + self.digits[1];
        } else if (operation == "divide") {
            self.result = self.digits[0] / self.digits[1];
        }
    });

    then(LOCATION(), "the result should be ([\\d\\.]+) on the screen", [](Calculator &self, double result) {
        EXPECT_THAT(self.result, ::testing::DoubleEq(result));
    });

    given(LOCATION(), "我在计算器中输入了 ([\\d\\.]+)", [](Calculator &self, double i) {
        self.digits.push_back(i);
    });

    when(LOCATION(), "我按(.+)", [](Calculator &self, const std::string& operation) {
        if (operation == "add") {
            self.result = self.digits[0] + self.digits[1];
        } else if (operation == "除法") {
            self.result = self.digits[0] / self.digits[1];
        }
    });

    then(LOCATION(), "屏幕上的结果应该是 ([\\d\\.]+)", [](Calculator &self, double result) {
        EXPECT_THAT(self.result, ::testing::DoubleEq(result));
    });
}
