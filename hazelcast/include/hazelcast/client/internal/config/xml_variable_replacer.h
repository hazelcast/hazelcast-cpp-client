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
/**
 * Interface to be implemented by pluggable variable replacers for the configuration files. The replacers can be configured in
 * XML configuration files and they are used to replace custom strings during loading the configuration.
 *
 * A Variable to be replaced within the configuration file has following form:
 * "$" PREFIX "{" MASKED_VALUE "}"
 *
 * where the PREFIX is the value returned by get_prefix() method and MASKED_VALUE is a value provided to
 * the get_replacement(std::string) method. The result of get_replacement(std::string) method call replaces the whole
 * Variable String.
 */
class HAZELCAST_API config_replacer
{
public:
    /**
     * Initialization method called before first usage of the config replacer.
     * @param properties – properties configured
     */
    virtual void init(std::unordered_map<std::string, std::string> properties) = 0;
    /**
     * Variable replacer prefix string. The value returned should be a constant unique short alphanumeric string without whitespaces.
     * @return constant prefix of this replacer
     */
    virtual std::string get_prefix() = 0;
    /**
     * Provides String which should be used as a variable replacement for given masked value.
     * @param variable - – the masked value
     * @return either not empty String to be used as a replacement for the variable; or empty string when this replacer is not able to handle the masked value.
     */
    virtual std::string get_replacement(const std::string& masked_value) = 0;
};
/**
 * ConfigReplacer for replacing property names with property values for properties provided in init(Properties) method.
 */
class HAZELCAST_API property_replacer : public config_replacer
{
private:

    std::unordered_map<std::string, std::string> properties;

public:
    property_replacer();
    void init(std::unordered_map<std::string, std::string> properties) override;
    std::string get_prefix() override;
    std::string get_replacement(const std::string& variable) override;
};
/**
 * Virtual class for replacing variable in DOM Nodes.
 */
class HAZELCAST_API dom_variable_replacer
{
public:
    /**
     * Replaces variables in the given Node with the provided config_replacer.
     * @param node – The node in which the variables to be replaced
     * @param replacer  – The replacer to be used for replacing the variables
     * @param fail_fast – Indicating whether or not a InvalidConfigurationException should be thrown if no replacement found for the variables in the node
     * @param node_name - node name
     * Throws: InvalidConfigurationException – if no replacement is found for a variable and failFast is true
     */
    virtual void replace_variables(boost::property_tree::ptree* node,
                                   property_replacer replacer,
                                   bool fail_fast,
                                   std::string node_name) = 0;
};
class HAZELCAST_API abstract_dom_variable_replacer
  : public dom_variable_replacer
{
private:
    static void handle_missing_variable(const std::string& variable,
                                        const std::string& node_name,
                                        bool fail_fast);

protected:
    static std::string replace_value(const boost::property_tree::ptree& node,
                                     property_replacer replacer,
                                     bool fail_fast,
                                     const std::string& value,
                                     const std::string& node_name);

public:
    static void replace_variable_in_node_value(boost::property_tree::ptree* node,
                                               property_replacer replacer,
                                               bool fail_fast,
                                               const std::string& node_name);
};

class HAZELCAST_API xml_dom_variable_replacer
  : public abstract_dom_variable_replacer
{
public:
    xml_dom_variable_replacer();
    void replace_variables(boost::property_tree::ptree* node,
                           property_replacer replacer,
                           bool fail_fast,
                           std::string node_name) override;
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
} // namespace internal
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

