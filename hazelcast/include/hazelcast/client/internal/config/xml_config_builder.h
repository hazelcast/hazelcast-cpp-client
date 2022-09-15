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
#include "hazelcast/client/internal/config/xml_variable_replacer.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace config {

class HAZELCAST_API abstract_config_builder
{
private:
    const static hazelcast::client::client_property* VALIDATION_ENABLED_PROP;

protected:
    bool should_validate_the_schema();
};

class HAZELCAST_API abstract_xml_config_helper : public abstract_config_builder
{
    std::string xmlns =
      "http://www.hazelcast.com/schema/" + get_namespace_type();
    const std::string hazelcast_schema_location =
      "hazelcast-client-config-" + get_release_version() + ".xsd";

protected:
    bool dom_level_3 = true;
    void schema_validation(boost::property_tree::ptree doc);

public:
    static std::string get_release_version();
    static std::string get_namespace_type();
};

class HAZELCAST_API abstract_xml_config_builder
  : public abstract_xml_config_helper
{
    static std::string get_attribute(boost::property_tree::ptree node,
                              const std::string& att_name);
    void fill_properties(
      const boost::property_tree::ptree& node,
      std::unordered_map<std::string, std::string>* properties);

private:
    std::unordered_map<std::string, std::string>* properties = new std::unordered_map<std::string, std::string>();
    std::unordered_set<std::string>* currently_imported_files =
      new std::unordered_set<std::string>();
    void replace_variables(boost::property_tree::ptree* root);
    void replace_imports(boost::property_tree::ptree* root);
    property_replacer create_replacer(const boost::property_tree::ptree& node);

protected:
    void process(boost::property_tree::ptree* root);
    virtual boost::property_tree::ptree parse(std::ifstream* input_stream) = 0;
    void set_properties_internal(
      std::unordered_map<std::string, std::string> properties);

public:
    std::unordered_map<std::string, std::string>* get_properties();
    abstract_xml_config_builder();
};

class HAZELCAST_API xml_client_config_builder
  : public abstract_xml_config_builder
{
private:
    std::ifstream* in;
    void parse_and_build_config(
      hazelcast::client::client_config* client_config);

protected:
    boost::property_tree::ptree parse(std::ifstream* input_stream) override;

public:
    explicit xml_client_config_builder(xml_client_config_locator* locator);
    explicit xml_client_config_builder(const std::string& resource);
    explicit xml_client_config_builder(std::ifstream* in);
    xml_client_config_builder();
    hazelcast::client::client_config build();
    xml_client_config_builder set_properties(
      std::unordered_map<std::string, std::string> properties);
};

} // namespace config
}
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif