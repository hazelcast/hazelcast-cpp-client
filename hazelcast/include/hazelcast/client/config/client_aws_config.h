/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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
#include <ostream>
#include <stdint.h>

#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace config {
/**
 * The client_aws_config contains the configuration for client
 * to connect to nodes in aws environment.
 *
 * The client_aws_config contains the configuration for AWS join mechanism.
 * <p/>
 * what happens behind the scenes is that data about the running AWS instances
 * in a specific region are downloaded using the accesskey/secretkey and are
 * potential Hazelcast members. <p/> <h1>Filtering</h1> There are 2 mechanisms
 * for filtering out AWS instances and these mechanisms can be combined (AND).
 * <ol>
 * <li>If a securityGroup is configured, only instances within that security
 * group are selected.
 * </li>
 * <li>
 * If a tag key/value is set, only instances with that tag key/value will be
 * selected.
 * </li>
 * </ol>
 * <p/>
 */
class HAZELCAST_API client_aws_config
{
public:
    client_aws_config();
    /**
     * Gets the access key to access AWS. Returns empty string if no access key
     * is configured.
     *
     * @return the access key to access AWS
     * @see #set_access_key(std::string)
     */
    const std::string& get_access_key() const;

    /**
     * Sets the access key to access AWS.
     *
     * @param accessKey the access key to access AWS
     * @return the updated client_aws_config.
     * @throws illegal_argument if accessKey is empty.
     * @see #getAccessKey()
     * @see #set_secret_key(std::string)
     */
    client_aws_config& set_access_key(const std::string& access_key);
    /**
     * Gets the secret key to access AWS. Returns empty string if no access key
     * is configured.
     *
     * @return the secret key.
     * @see #set_secret_key(std::string)
     */
    const std::string& get_secret_key() const;

    /**
     * Sets the secret key to access AWS.
     *
     * @param secretKey the secret key to access AWS
     * @return the updated client_aws_config.
     * @throws illegal_argument if secretKey is empty.
     * @see #getSecretKey()
     * @see #set_access_key(std::string)
     */
    client_aws_config& set_secret_key(const std::string& secret_key);

    /**
     * Gets the region where the EC2 instances running the Hazelcast members
     * will be running.
     *
     * @return the region where the EC2 instances running the Hazelcast members
     * will be running
     * @see #setRegion(std::string)
     */
    const std::string& get_region() const;
    /**
     * Sets the region where the EC2 instances running the Hazelcast members
     * will be running.
     *
     * @param region the region where the EC2 instances running the Hazelcast
     * members will be running
     * @return the updated client_aws_config
     * @throws illegal_argument if region is empty.
     */
    client_aws_config& set_region(const std::string& region);

    /**
     * Gets the host header; the address where the EC2 API can be found.
     *
     * @return the host header; the address where the EC2 API can be found
     */
    const std::string& get_host_header() const;
    /**
     * Sets the host header; the address where the EC2 API can be found.
     *
     * @param hostHeader the new host header; the address where the EC2 API can
     * be found
     * @return the updated client_aws_config
     * @throws illegal_argument if hostHeader is an empty string.
     */
    client_aws_config& set_host_header(const std::string& host_header);

    /**
     * Enables or disables the aws join mechanism.
     *
     * @param enabled true if enabled, false otherwise.
     * @return the updated client_aws_config.
     */
    client_aws_config& set_enabled(bool enabled);
    /**
     * Checks if the aws join mechanism is enabled.
     *
     * @return true if enabled, false otherwise.
     */
    bool is_enabled() const;

    /**
     * Sets the security group name. See the filtering section above for more
     * information.
     *
     * @param securityGroupName the security group name.
     * @return the updated client_aws_config.
     * @see #getSecurityGroupName()
     */
    client_aws_config& set_security_group_name(
      const std::string& security_group_name);

    /**
     * Gets the security group name. If nothing has been configured, empty
     * string is returned.
     *
     * @return the security group name; empty string if nothing has been
     * configured
     */
    const std::string& get_security_group_name() const;

    /**
     * Sets the tag key. See the filtering section above for more information.
     *
     * @param tagKey the tag key. See the filtering section above for more
     * information.
     * @return the updated client_aws_config.
     * @see #set_tag_key(std::string)
     */
    client_aws_config& set_tag_key(const std::string& tag_key);

    /**
     * Gets the tag key. If nothing is specified, empty string is returned.
     *
     * @return the tag key. empty string if nothing is returned.
     */
    const std::string& get_tag_key() const;

    /**
     * Sets the tag value. See the filtering section above for more information.
     *
     * @param tagValue the tag value. See the filtering section above for more
     * information.
     * @return the updated client_aws_config.
     * @see #set_tag_key(std::string)
     * @see #getTagValue()
     */
    client_aws_config& set_tag_value(const std::string& tag_value);

    /**
     * Gets the tag value. If nothing is specified, empty string is returned.
     *
     * @return the tag value. empty string if nothing is returned.
     */
    const std::string& get_tag_value() const;

    /**
     * Gets the iamRole name
     *
     * @return the iamRole. empty string if nothing is returned.
     * @see #set_iam_role(std::string) (int32_t)
     */
    const std::string& get_iam_role() const;

    /**
     * Sets the tag value. See the filtering section above for more information.
     *
     * @param iamRole the IAM Role name.
     * @return the updated client_aws_config.
     * @see #getIamRole()
     */
    client_aws_config& set_iam_role(const std::string& iam_role);

    /**
     * If client is inside aws, it will use private ip addresses directly,
     * otherwise it will convert private ip addresses to public addresses
     * internally by calling AWS API.
     *
     * @return bool true if client is inside aws environment.
     */
    bool is_inside_aws() const;

    /**
     * Set to true if client is inside aws environment
     * Default value is false.
     *
     * @param insideAws isInsideAws
     */
    client_aws_config& set_inside_aws(bool inside_aws);

private:
    bool enabled_;
    std::string access_key_;
    std::string secret_key_;
    std::string region_;
    std::string security_group_name_;
    std::string tag_key_;
    std::string tag_value_;
    std::string host_header_;
    std::string iam_role_;

    bool inside_aws_;
};

std::ostream HAZELCAST_API&
operator<<(std::ostream& out, const client_aws_config& config);
} // namespace config
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
