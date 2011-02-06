#include "Abstraction.h"
#include "Exception.h"
#include "catch.hpp"

// These are kind of stupid, but what the hell
// If we don't crash from exceptions, this is considered a success
TEST_CASE("exception/structor", "Test construction/destruction")
{
  // Const char* construction
  Exception ex1("test");

  // Construction from temporary variable in expression
  Exception ex2(std::string("test"));

  // Construction from constr string
  const std::string text1("test");
  Exception ex3(text1);

  // Construction from non-const string
  std::string text2("test");
  Exception ex4(text2);
  REQUIRE(text2 == "test");

  // Construction from string expression
  Exception ex5(text1 + "test");
}

// A little more complicated, but honestly, not much to test here
TEST_CASE("exception/what", "Test getting the string from an exception")
{
  // Const char* construction
  Exception ex1("test");
  REQUIRE(ex1.what() == "test");

  // Construction from temporary variable in expression
  Exception ex2(std::string("test"));
  REQUIRE(ex2.what() == "test");

  // Construction from constr string
  const std::string text1("test");
  Exception ex3(text1);
  REQUIRE(ex3.what() == "test");

  // Construction from non-const string
  std::string text2("test");
  Exception ex4(text2);
  REQUIRE(text2 == "test");
  REQUIRE(ex4.what() == "test");

  // Construction from string expression
  Exception ex5(text1 + "test");
  REQUIRE(ex5.what() == "testtest");
}
