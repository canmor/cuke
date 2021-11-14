#include <cuke/server.h>
#include "calculator.h"

int main(int argc, const char *argv[]) {
    calculator_steps();
    cuke::server server("127.0.0.1", 3902);
    return 0;
}
