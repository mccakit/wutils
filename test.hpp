#pragma once

#include <iostream>
#include <string_view>

#define ASSERT_TRUE(condition)                                                 \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::cerr << "Assertion failed at " << __FILE__ << ":" << __LINE__       \
                << ": " << #condition << std::endl;                            \
      std::exit(EXIT_FAILURE);                                                 \
    }                                                                          \
  } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))

#define ASSERT_NE(a, b) ASSERT_TRUE((a) != (b))

#define ASSERT_THROW(expression, ExceptionType)                                \
  do {                                                                         \
    try {                                                                      \
      (void)(expression);                                                      \
      std::cerr << "Assertion failed at " << __FILE__ << ":" << __LINE__       \
                << ": Expected exception " << #ExceptionType                   \
                << " was not thrown." << std::endl;                            \
      std::exit(EXIT_FAILURE);                                                 \
    } catch (const ExceptionType &) {                                          \
    } catch (...) {                                                            \
      std::cerr << "Assertion failed at " << __FILE__ << ":" << __LINE__       \
                << ": Unexpected exception was thrown." << std::endl;          \
      std::exit(EXIT_FAILURE);                                                 \
    }                                                                          \
  } while (0)

void run_test(void (*test_func)(), std::string_view test_name) {
  std::cout << "Running test: " << test_name << "..." << std::endl;
  test_func();
  std::cout << "Test passed: " << test_name << std::endl;
}
