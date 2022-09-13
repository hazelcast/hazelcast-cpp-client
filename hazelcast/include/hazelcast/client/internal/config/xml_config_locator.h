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
class HAZELCAST_API declarative_config_util
{
private:
    declarative_config_util() = default;

public:
    static std::string SYSPROP_MEMBER_CONFIG;
    static std::string SYSPROP_CLIENT_CONFIG;
    static std::string SYSPROP_CLIENT_FAILOVER_CONFIG;
    static std::vector<std::string> XML_ACCEPTED_SUFFIXES;
    static std::string XML_ACCEPTED_SUFFIXES_STRING;
    static std::vector<std::string> YAML_ACCEPTED_SUFFIXES;
    static std::string YAML_ACCEPTED_SUFFIXES_STRING;
    static std::vector<std::string> ALL_ACCEPTED_SUFFIXES;
    static std::string ALL_ACCEPTED_SUFFIXES_STRING;
    static void validate_suffix_in_system_property(
      const std::string& property_key);
    static void throw_unaccepted_suffix_in_system_property(
      const std::string& property_key,
      const std::string& config_resource,
      const std::vector<std::string>& accepted_suffixes);
    static bool is_accepted_suffix_configured(
      const std::string& config_file,
      std::vector<std::string> accepted_suffixes);
};
class HAZELCAST_API abstract_config_locator
{
private:
    std::ifstream* in;
    std::FILE* configuration_file;
    bool load_from_system_property(
      const std::string& property_key,
      bool fail_on_unaccepted_suffix,
      const std::vector<std::string>& accepted_suffixes);
    void load_system_property_file_resource(
      const std::string& config_system_property);

protected:
    bool load_from_working_directory(const std::string& config_file_path);
    bool load_from_working_directory(
      const std::string& config_file_prefix,
      const std::vector<std::string>& accepted_suffixes);
    bool load_from_system_property(const std::string& property_key,
                                   const std::vector<std::string>& accepted_suffixes);
    bool load_from_system_property_or_fail_on_unaccepted_suffix(
      const std::string& property_key,
      const std::vector<std::string>& accepted_suffixes);

public:
    std::ifstream* get_in();
    bool is_config_present();
    // virtual bool locate_from_system_property();
    // virtual bool locate_from_system_property_or_fail_on_unaccepted_suffix();
    // virtual bool locate_in_work_directory();
    // virtual bool locate_default();
    bool locate_everywhere();
};
class HAZELCAST_API xml_client_config_locator : public abstract_config_locator
{
protected:
    bool locate_from_system_property_or_fail_on_unaccepted_suffix();

public:
    bool locate_from_system_property();
    bool locate_in_work_directory();
};

} // namespace config
}
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif