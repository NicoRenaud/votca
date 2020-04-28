/*
 * Copyright 2009-2020 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE calculator_test
#include "../../include/votca/tools/calculator.h"
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>

using namespace ::votca;

BOOST_AUTO_TEST_SUITE(calculator_test)

BOOST_AUTO_TEST_CASE(load_defaults_test) {

  class TestCalc : public tools::Calculator {

   public:
    std::string Identify() override { return "testcalc"; }

    void Initialize(const tools::Property &user_options) override {

      // Create folder for test
      const char dir_path[] = "calculators";
      boost::filesystem::path dir(dir_path);
      boost::filesystem::create_directory(dir);
      dir.append("xml");
      boost::filesystem::create_directory(dir);

      std::ofstream defaults("calculators/xml/testcalc.xml");
      defaults << "<options>\n"
               << "<testcalc>\n"
               << "<option0 choices=\"foo,bar\">foo</option0>\n"
               << "<option1 choices=\"int+\">0</option1>\n"
               << "<option2 choices=\"float\">-3.141592</option2>\n"
               << "<option4 choices=\"float+\">3.141592</option4>\n"
               << "<option5 choices=\"bool\">true</option5>\n"
               << "</testcalc>\n"
               << "</options>";
      defaults.close();

      // Load and check the options
      tools::Property final_opt =
          LoadDefaultsAndUpdateWithUserOptions("calculators", user_options);

      std::string prop0 = final_opt.get("option0").as<std::string>();
      Index prop1 = final_opt.get("option1").as<votca::Index>();
      double prop2 = final_opt.get("option2").as<double>();
      std::string prop3 = final_opt.get("option3.nested").as<std::string>();
      double prop4 = final_opt.get("option4").as<double>();
      bool prop5 = final_opt.get("option5").as<bool>();
      BOOST_CHECK_EQUAL(prop0, "foo");
      BOOST_CHECK_EQUAL(prop1, 42);
      BOOST_CHECK_CLOSE(prop2, -3.141592, 0.00001);
      BOOST_CHECK_EQUAL(prop3, "nested_value");
      BOOST_CHECK_CLOSE(prop4, 3.141592, 0.00001);
      BOOST_CHECK_EQUAL(prop5, true);
    }
  };

  setenv("VOTCASHARE", ".", 1);
  char buff[FILENAME_MAX];
  std::cout << "WARNING: the VOTCASHARE env. variable has been updated to "
            << getcwd(buff, FILENAME_MAX) << "\n";

  // Generate user options
  tools::Property user_options;

  tools::Property &opt = user_options.add("options", "");
  tools::Property &opt_test = opt.add("testcalc", "");
  opt_test.add("option1", "42");
  tools::Property &new_prop = opt_test.add("option3", "");
  new_prop.add("nested", "nested_value");

  TestCalc test_calc;
  test_calc.Initialize(user_options);
}

BOOST_AUTO_TEST_CASE(test_choices) {

  class TestChoices : public tools::Calculator {

    std::string _line;

   public:
    std::string Identify() override { return "testchoices"; }

    void SetOption(const std::string &line) { _line = line; }

    void Initialize(const tools::Property &user_options) override {

      // Create folder for test
      const char dir_path[] = "calculators";
      boost::filesystem::path dir(dir_path);
      boost::filesystem::create_directory(dir);
      dir.append("xml");
      boost::filesystem::create_directory(dir);

      std::ofstream defaults("calculators/xml/testchoices.xml");
      defaults << "<options>\n"
               << "<testchoices>\n"
               << _line << "</testchoices>\n"
               << "</options>";
      defaults.close();

      // Load and check the options
      tools::Property final_opt =
          LoadDefaultsAndUpdateWithUserOptions("calculators", user_options);
      std::cout << final_opt << "\n";
    }
  };

  setenv("VOTCASHARE", ".", 1);
  char buff[FILENAME_MAX];
  std::cout << "WARNING: the VOTCASHARE env. variable has been updated to "
            << getcwd(buff, FILENAME_MAX) << "\n";

  // Generate user options
  tools::Property user_options;
  tools::Property &opt = user_options.add("options", "");
  opt.add("testchoices", "");

  TestChoices test1, test2, test3, test4, test5;
  test1.SetOption("<option1 choices=\"foo, bar, baz, qux\">boom</option1>\n");
  test2.SetOption("<option2 choices =\"float\">some</option2>\n");
  test3.SetOption("<option3 choices=\"int\">3.14</option3>\n");
  test4.SetOption("<option4 choices=\"int+\">-2</option4>\n");
  test5.SetOption("<option5 choices=\"float+\">-3.14</option5>\n");

  BOOST_CHECK_THROW(test1.Initialize(user_options), std::runtime_error);
  BOOST_CHECK_THROW(test2.Initialize(user_options), std::runtime_error);
  BOOST_CHECK_THROW(test3.Initialize(user_options), std::runtime_error);
  BOOST_CHECK_THROW(test4.Initialize(user_options), std::runtime_error);
  BOOST_CHECK_THROW(test5.Initialize(user_options), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
