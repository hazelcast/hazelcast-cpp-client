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
#include <hazelcast/client/client_config.h>
#include <hazelcast/client/client_properties.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace config {
class HAZELCAST_API config_replacer
{
public:
    virtual void init(hazelcast::client::client_properties properties);
    virtual std::string get_prefix();
    virtual std::string get_replacement(std::string masked_value);
};
class HAZELCAST_API property_replacer : public config_replacer
{
private:
    hazelcast::client::client_properties properties;
public:
    property_replacer();
    void init(hazelcast::client::client_properties properties) override;
    std::string get_prefix() override;
    std::string get_replacement(std::string variable) override;
};
class HAZELCAST_API dom_variable_replacer
{
public:
    virtual void replace_variables(boost::property_tree::ptree node,config_replacer  replacer, bool fail_fast);
};
class HAZELCAST_API config_replacer_helper
{
private:
    config_replacer_helper();
    static void traverse_children_and_replace_variables(boost::property_tree::ptree root, config_replacer replacer, bool fail_fast,
                                                        dom_variable_replacer variable_replacer);
    static boost::property_tree::ptree pair_to_node(std::string node_name, boost::property_tree::ptree node_content);

public:
    static void traverse_children_and_replace_variables(boost::property_tree::ptree root, std::vector<config_replacer> replacers, bool fail_fast,
                                                        dom_variable_replacer variable_replacer);
};
class HAZELCAST_API abstract_dom_variable_replacer : public dom_variable_replacer
{
private:
    bool non_replaceable_node(boost::property_tree::ptree node);
    static void handle_missing_variable(std::string variable, std::string node_name, bool fail_fast);
protected:
    static std::string replace_value(boost::property_tree::ptree node, config_replacer replacer, bool fail_fast, std::string value, std::string node_name);
public:
    void replace_variable_in_node_value(boost::property_tree::ptree node, config_replacer replacer, bool fail_fast, std::string node_name);
};
class HAZELCAST_API xml_dom_variable_replacer : public abstract_dom_variable_replacer
{
public:
    xml_dom_variable_replacer();
    void replace_variables(boost::property_tree::ptree node, config_replacer replacer, bool fail_fast) override;

};

} // namespace config
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

