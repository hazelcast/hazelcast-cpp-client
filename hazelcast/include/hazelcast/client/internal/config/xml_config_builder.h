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
protected:
    bool should_validate_the_schema();
};
/**
 * Contains Hazelcast XML Configuration helper methods and variables.
 */
class HAZELCAST_API abstract_xml_config_helper : public abstract_config_builder
{
    std::string xmlns =
      "http://www.hazelcast.com/schema/" + get_namespace_type();
    const std::string hazelcast_schema_location =
      "hazelcast-client-config-" + get_release_version() + ".xsd";

protected:
    bool dom_level_3 = true;
    void schema_validation(const boost::property_tree::ptree& doc);

public:
    static std::string get_release_version();
    static std::string get_namespace_type();
};
/**
 * Contains logic for replacing system variables in the XML file and importing XML files from different locations.
 */
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
    /**
     * Reads XML from InputStream and parses.
     * @param input_stream InputStream to read from
     * @return ptree after parsing XML
     * @throw Exception – if the XML configuration cannot be parsed or is invalid
     */
    virtual boost::property_tree::ptree parse(std::ifstream input_stream) = 0;
    /**
     * Sets the used properties. Can be null if no properties should be used.
     * Properties are used to resolve ${variable} occurrences in the XML file.
     * @param properties – the new properties
     */
    void set_properties_internal(
      std::unordered_map<std::string, std::string> properties);

public:
    /**
     * Gets the current used properties. Can be null if no properties are set.
     * @return the current used properties
     */
    std::unordered_map<std::string, std::string>* get_properties();
    abstract_xml_config_builder();
};
/**
 * Loads the client_config using XML.
 */
class HAZELCAST_API xml_client_config_builder
  : public abstract_xml_config_builder
{
private:
    std::shared_ptr<std::ifstream> in;
    void parse_and_build_config(
      hazelcast::client::client_config* client_config);

protected:
    boost::property_tree::ptree parse(std::ifstream input_stream) override;

public:
    /**
     * Constructs a xml_client_config_builder that loads the configuration with the provided xml_client_config_locator.
     * it is expected that it already located the configuration XML to load from.
     * No further attempt to locate the configuration XML is made if the configuration XML is not located already.
     * @param locator – the configured locator to use
     */
    explicit xml_client_config_builder(xml_client_config_locator* locator);
    explicit xml_client_config_builder(const std::string& resource);
    explicit xml_client_config_builder(std::shared_ptr<std::ifstream> in);
    /**
     * Loads the client config using the following resolution mechanism:
     * 1. First it checks if a system property 'hazelcast.client.config' is set. If it exist ,it assumes
     * it is a file reference. The configuration file or resource will be loaded only if the postfix of its name ends with `.xml`.
     * 2. It checks if a hazelcast-client.xml is available in the working dir
     */
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