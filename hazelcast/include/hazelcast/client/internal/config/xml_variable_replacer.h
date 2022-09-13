/*
* Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
 */
#pragma once

#include <string>
#include <vector>
#include "hazelcast/util/export.h"
#include <boost/property_tree/ptree.hpp>
#include "hazelcast/client/client_config.h"
#include "hazelcast/client/client_properties.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace config {

class HAZELCAST_API config_replacer
{
public:
    virtual void init(std::unordered_map<std::string, std::string>* properties) = 0;
    virtual std::string get_prefix() = 0;
    virtual std::string get_replacement(const std::string& variable) = 0;
};

class HAZELCAST_API property_replacer : public config_replacer
{
private:

    std::unordered_map<std::string, std::string>* properties = new std::unordered_map<std::string, std::string>();

public:
    property_replacer();
    void init(std::unordered_map<std::string, std::string>* properties) override;
    std::string get_prefix() override;
    std::string get_replacement(const std::string& variable) override;
};

class HAZELCAST_API dom_variable_replacer
{
public:
    virtual void replace_variables(boost::property_tree::ptree* node,
                                   property_replacer replacer,
                                   bool fail_fast) = 0;
};

class HAZELCAST_API abstract_dom_variable_replacer
  : public dom_variable_replacer
{
private:
    bool non_replaceable_node(const boost::property_tree::ptree& node);
    static void handle_missing_variable(const std::string& variable,
                                        const std::string& node_name,
                                        bool fail_fast);

protected:
    static std::string replace_value(const boost::property_tree::ptree& node,
                                     property_replacer replacer,
                                     bool fail_fast,
                                     const std::string& value);

public:
    void replace_variable_in_node_value(boost::property_tree::ptree* node,
                                        property_replacer replacer,
                                        bool fail_fast);
};

class HAZELCAST_API xml_dom_variable_replacer
  : public abstract_dom_variable_replacer
{
public:
    xml_dom_variable_replacer();
    void replace_variables(boost::property_tree::ptree* node,
                           property_replacer replacer,
                           bool fail_fast) override;
};

class HAZELCAST_API config_replacer_helper
{
private:
    config_replacer_helper();
    static void traverse_children_and_replace_variables(
      boost::property_tree::ptree* root,
      const property_replacer& replacer,
      bool fail_fast,
      xml_dom_variable_replacer variable_replacer);

public:
    static void traverse_children_and_replace_variables(
      boost::property_tree::ptree* root,
      const std::vector<property_replacer>& replacers,
      bool fail_fast,
      const xml_dom_variable_replacer& variable_replacer);
};

} // namespace config
}
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

