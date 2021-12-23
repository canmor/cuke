add_executable(ut)
target_sources(ut PRIVATE step.cpp wire.cpp)
target_link_libraries(ut PRIVATE cuke::cuke GTest::gmock_main)