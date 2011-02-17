#include "Abstraction.h"
#include "AbstractionException.h"
#include "Log.h"
#include "catch.hpp"
#include <sstream>
#include <fstream>

const std::string logFile1("libCommonTest1.log");
const std::string logFile2("libCommonTest2.log");

TEST_CASE("log/fileMode", "Test logging to a file")
{
  // Clear log files
  {
    std::ofstream out1(logFile1.c_str(), std::ios_base::out | std::ios_base::trunc);
    std::ofstream out2(logFile2.c_str(), std::ios_base::out | std::ios_base::trunc);
  }

  Log::getInstance().setFileMode(logFile1);

  // Test debug mode
  Log::getInstance().setLevel(Log::Debug);
  LogDebug("Log statement " << 1);
  LogInfo("Test text " << std::string("test"));
  LogError("Quiddity " << 1);

  // Test info mode
  Log::getInstance().setLevel(Log::Info);
  LogDebug("Log statement " << 2);
  LogInfo("Test text " << std::string("text"));
  LogError("Quiddity " << 2);

  // Test error mode
  Log::getInstance().setLevel(Log::Error);
  LogDebug("Log statement " << 3);
  LogInfo("Text text " << std::string("text"));
  LogError("Quiddity " << 3);

  // Switch to a new file, make sure Error level is maintained
  Log::getInstance().setFileMode(logFile2);
  LogDebug("Log statement " << 4);
  LogInfo("Test test " << std::string("test"));
  LogError("Quiddity " << 4);

  // Test disabled level
  Log::getInstance().setLevel(Log::Disabled);
  LogDebug("Log statement " << 5);
  LogInfo("Text test " << std::string("test"));
  LogError("Quiddity " << 5);

  // Verify file data
  std::string line;

  // logFile1:
  // <stamp> Log statement 1
  // <stamp> Test text test
  // <stamp> Quiddity 1
  // <stamp> Test text text
  // <stamp> Quiddity 2
  // <stamp> Quiddity 3
  std::ifstream in1(logFile1.c_str());

  REQUIRE(in1.good());
  getline(in1, line); REQUIRE(line.find("Log statement 1") != std::string::npos);
  getline(in1, line); REQUIRE(line.find("Test text test") != std::string::npos);
  getline(in1, line); REQUIRE(line.find("Quiddity 1") != std::string::npos);
  getline(in1, line); REQUIRE(line.find("Test text text") != std::string::npos);
  getline(in1, line); REQUIRE(line.find("Quiddity 2") != std::string::npos);
  getline(in1, line); REQUIRE(line.find("Quiddity 3") != std::string::npos);
  getline(in1, line); REQUIRE(line == "");
  REQUIRE(in1.eof());

  // logFile2:
  // <stamp> Quiddity 4
  std::ifstream in2(logFile2.c_str());

  REQUIRE(in2.good());
  getline(in2, line); REQUIRE(line.find("Quiddity 4") != std::string::npos);
  getline(in2, line); REQUIRE(line == "");
  REQUIRE(in2.eof());

  // Set logging back to debug/stdout
  Log::getInstance().setLevel(Log::Debug);
  Log::getInstance().setStreamMode(std::cout);
}

TEST_CASE("log/streamMode", "Test logging to a stream")
{
  // Test debug mode
  std::stringstream out;
  Log::getInstance().setStreamMode(out);
  Log::getInstance().setLevel(Log::Debug);

  LogDebug("Log statement " << 1);
  REQUIRE(out.str().find("Log statement 1") != std::string::npos);
  out.str("");

  LogInfo("Test text " << std::string("test"));
  REQUIRE(out.str().find("Test text test") != std::string::npos);
  out.str("");

  LogError("Quiddity " << 1);
  REQUIRE(out.str().find("Quiddity 1") != std::string::npos);
  out.str("");

  // Test info mode
  Log::getInstance().setLevel(Log::Info);

  LogDebug("Log statement " << 2);
  REQUIRE(out.str() == "");
  out.str("");

  LogInfo("Test text " << std::string("text"));
  REQUIRE(out.str().find("Test text text") != std::string::npos);
  out.str("");

  LogError("Quiddity " << 2);
  REQUIRE(out.str().find("Quiddity 2") != std::string::npos);
  out.str("");

  // Test error mode
  Log::getInstance().setLevel(Log::Error);

  LogDebug("Log statement " << 3);
  REQUIRE(out.str() == "");
  out.str("");

  LogInfo("Text text " << std::string("text"));
  REQUIRE(out.str() == "");
  out.str("");

  LogError("Quiddity " << 3);
  REQUIRE(out.str().find("Quiddity 3") != std::string::npos);
  out.str("");

  // Set a new stream, make sure Error mode is maintained
  std::stringstream out2;
  Log::getInstance().setStreamMode(out2);

  LogDebug("Log statement " << 4);
  REQUIRE(out2.str() == "");
  out2.str("");

  LogInfo("Test test " << std::string("test"));
  REQUIRE(out2.str() == "");
  out2.str("");

  LogError("Quiddity " << 4);
  REQUIRE(out2.str().find("Quiddity 4") != std::string::npos);
  out2.str("");

  // Test disabled level
  Log::getInstance().setLevel(Log::Disabled);

  LogDebug("Log statement " << 5);
  REQUIRE(out2.str() == "");
  out2.str("");

  LogInfo("Text test " << std::string("test"));
  REQUIRE(out2.str() == "");
  out2.str("");

  LogError("Quiddity " << 5);
  REQUIRE(out2.str() == "");
  out2.str("");

  // Set logging back to debug/stdout
  Log::getInstance().setLevel(Log::Debug);
  Log::getInstance().setStreamMode(std::cout);
}
