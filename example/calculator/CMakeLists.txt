add_executable(calculator)
target_sources(calculator PRIVATE features/step_definitions/main.cpp features/step_definitions/calculator.cpp)
target_link_libraries(calculator PRIVATE cuke GTest::gmock)