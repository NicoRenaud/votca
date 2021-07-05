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

// Local VOTCA includes
#include "votca/tools/optionshandler.h"
#include <stdexcept>

namespace votca {
namespace tools {

template <typename T>
static bool IsValidCast(const tools::Property &prop) {
  try {
    prop.as<T>();
    return true;
  } catch (const std::runtime_error &e) {
    return false;
  }
}

void OptionsHandler::ResolveLinks(Property &prop) const {

  if (prop.hasAttribute("link")) {
    std::string relative_path =
        "subpackages/" + prop.getAttribute<std::string>("link");
    std::string file_path = defaults_path_ + relative_path;
    tools::Property package;
    package.LoadFromXML(file_path);
    for (const auto &child : package) {
      prop.add(child);
    }
  }

  for (tools::Property &child : prop) {
    ResolveLinks(child);
  }
}

Property OptionsHandler::ProcessUserInput(const Property &user_input,
                                          const std::string &calcname) const {
                                            return user_input;
                                          }

void OptionsHandler::CleanAttributes(
    Property &options, const std::vector<std::string> &attributes) const {}

// load the xml description of the calculator (with defaults and test values)
Property OptionsHandler::LoadDefaults(const std::string &calculatorname) const {
  Property defaults_all;
  std::string defaults_file_path =
      defaults_path_ + "/" + calculatorname + ".xml";
  defaults_all.LoadFromXML(defaults_file_path);
  ResolveLinks(defaults_all);
  return defaults_all.get("options." + calculatorname);
}

void OptionsHandler::InjectDefaultsAsValues(Property &defaults) const {
  for (Property &prop : defaults) {
    if (prop.HasChildren()) {
      InjectDefaultsAsValues(prop);
    } else if (prop.hasAttribute("default")) {
      prop.set(".", prop.getAttribute<std::string>("default"));
    }
  }
}

void OptionsHandler::UpdateWithUserOptions(Property &default_options,
                                           const Property &user_options,
                                           const std::string &calcname) const {

  Property options = user_options.get("options." + calcname);

  // if a value is given override default values
  OverwriteDefaultsWithUserInput(options, default_options);
}

void OptionsHandler::OverwriteDefaultsWithUserInput(const Property &user_input,
                                                    Property &defaults) const {

  for (const Property &prop : user_input) {
    if (prop.HasChildren()) {
      if (defaults.exists(prop.name())) {
        OverwriteDefaultsWithUserInput(prop, defaults.get(prop.name()));
      } else {
        Property &new_prop = defaults.add(prop.name(), "");
        new_prop = prop;
      }
    } else if (defaults.exists(prop.name())) {
      defaults.set(prop.name(), prop.value());
    } else {
      throw std::runtime_error(
          "You added an xml tag, which is not a valid input option:" +
          prop.path() + "/" + prop.name());
    }
  }
}

std::vector<std::string> OptionsHandler::GetPropertyChoices(const Property &p) {
  if (p.hasAttribute("choices")) {
    std::string att = p.getAttribute<std::string>("choices");
    std::size_t start_bracket = att.find('[');
    if (start_bracket != std::string::npos) {
      std::size_t end_bracket = att.find(']');
      att = att.substr(start_bracket + 1, end_bracket - start_bracket - 1);
    }
    return Tokenizer{att, " ,"}.ToVector();
  } else {
    return {};
  }
}

void OptionsHandler::RecursivelyCheckOptions(const Property &p) {
  for (const Property &prop : p) {
    if (prop.HasChildren()) {
      RecursivelyCheckOptions(prop);
    } else {
      std::vector<std::string> choices = GetPropertyChoices(prop);
      if (choices.empty()) {
        return;
      }
      if (!IsValidOption(prop, choices)) {
        std::ostringstream oss;
        oss << "\nThe input value for \"" << prop.name() << "\"";
        if (choices.size() == 1) {
          oss << " should be a \"" << choices.front() << "\"";
        } else {
          oss << " should be one of the following values: ";
          for (const std::string &c : choices) {
            oss << "\"" << c << "\""
                << " ";
          }
        }
        oss << " But \"" << prop.value()
            << "\" cannot be converted into one.\n";
        throw std::runtime_error(oss.str());
      }
    }
  }
}

bool OptionsHandler::IsValidOption(const Property &prop,
                                   const std::vector<std::string> &choices) {
  const std::string &head = choices.front();
  std::ostringstream oss;
  bool is_valid = true;
  if (head == "bool") {
    is_valid = IsValidCast<bool>(prop);
  } else if (head == "float") {
    is_valid = IsValidCast<double>(prop);
  } else if (head == "float+") {
    is_valid = IsValidCast<double>(prop) && (prop.as<double>() >= 0.0);
  } else if (head == "int") {
    is_valid = IsValidCast<Index>(prop);
  } else if (head == "int+") {
    is_valid = IsValidCast<Index>(prop) && (prop.as<Index>() >= 0);
  } else {
    std::string value = prop.as<std::string>();
    std::string att = prop.getAttribute<std::string>("choices");
    std::size_t start_bracket = att.find('[');
    if (start_bracket == std::string::npos) {
      // There is a single choice out of multiple default valid choices
      auto it = std::find(std::cbegin(choices), std::cend(choices), value);
      is_valid = (it != std::cend(choices));
    } else {
      // there are multiple valid choices
      Tokenizer tok{value, " ,"};
      for (const std::string &word : tok) {
        auto it = std::find(std::cbegin(choices), std::cend(choices), word);
        if (it == std::cend(choices)) {
          is_valid = false;
          break;
        }
      }
    }
  }
  return is_valid;
}

}  // namespace tools
}  // namespace votca
