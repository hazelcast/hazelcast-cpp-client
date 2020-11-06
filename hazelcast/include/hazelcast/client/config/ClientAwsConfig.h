/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            /**
             * The ClientAwsConfig contains the configuration for client
             * to connect to nodes in aws environment.
             *
             * The ClientAwsConfig contains the configuration for AWS join mechanism.
             * <p/>
             * what happens behind the scenes is that data about the running AWS instances in a specific region are downloaded using the
             * accesskey/secretkey and are potential Hazelcast members.
             * <p/>
             * <h1>Filtering</h1>
             * There are 2 mechanisms for filtering out AWS instances and these mechanisms can be combined (AND).
             * <ol>
             * <li>If a securityGroup is configured, only instances within that security group are selected.
             * </li>
             * <li>
             * If a tag key/value is set, only instances with that tag key/value will be selected.
             * </li>
             * </ol>
             * <p/>
             */
            class HAZELCAST_API ClientAwsConfig {
            public:
                ClientAwsConfig();
                /**
                 * Gets the access key to access AWS. Returns empty string if no access key is configured.
                 *
                 * @return the access key to access AWS
                 * @see #setAccessKey(std::string)
                 */
                const std::string &get_access_key() const;

                /**
                 * Sets the access key to access AWS.
                 *
                 * @param accessKey the access key to access AWS
                 * @return the updated ClientAwsConfig.
                 * @throws IllegalArgumentException if accessKey is empty.
                 * @see #getAccessKey()
                 * @see #setSecretKey(std::string)
                 */
                ClientAwsConfig &set_access_key(const std::string &access_key);
                /**
                 * Gets the secret key to access AWS. Returns empty string if no access key is configured.
                 *
                 * @return the secret key.
                 * @see #setSecretKey(std::string)
                 */
                const std::string &get_secret_key() const;

                /**
                 * Sets the secret key to access AWS.
                 *
                 * @param secretKey the secret key to access AWS
                 * @return the updated ClientAwsConfig.
                 * @throws IllegalArgumentException if secretKey is empty.
                 * @see #getSecretKey()
                 * @see #setAccessKey(std::string)
                 */
                ClientAwsConfig &set_secret_key(const std::string &secret_key);

                /**
                 * Gets the region where the EC2 instances running the Hazelcast members will be running.
                 *
                 * @return the region where the EC2 instances running the Hazelcast members will be running
                 * @see #setRegion(std::string)
                 */
                const std::string &get_region() const;
                /**
                 * Sets the region where the EC2 instances running the Hazelcast members will be running.
                 *
                 * @param region the region where the EC2 instances running the Hazelcast members will be running
                 * @return the updated ClientAwsConfig
                 * @throws IllegalArgumentException if region is empty.
                 */
                ClientAwsConfig &set_region(const std::string &region);

                /**
                 * Gets the host header; the address where the EC2 API can be found.
                 *
                 * @return the host header; the address where the EC2 API can be found
                 */
                const std::string &get_host_header() const;
                /**
                 * Sets the host header; the address where the EC2 API can be found.
                 *
                 * @param hostHeader the new host header; the address where the EC2 API can be found
                 * @return the updated ClientAwsConfig
                 * @throws IllegalArgumentException if hostHeader is an empty string.
                 */
                ClientAwsConfig &set_host_header(const std::string &host_header);

                /**
                 * Enables or disables the aws join mechanism.
                 *
                 * @param enabled true if enabled, false otherwise.
                 * @return the updated ClientAwsConfig.
                 */
                ClientAwsConfig &set_enabled(bool enabled);
                /**
                 * Checks if the aws join mechanism is enabled.
                 *
                 * @return true if enabled, false otherwise.
                 */
                bool is_enabled() const;

                /**
                 * Sets the security group name. See the filtering section above for more information.
                 *
                 * @param securityGroupName the security group name.
                 * @return the updated ClientAwsConfig.
                 * @see #getSecurityGroupName()
                 */
                ClientAwsConfig &set_security_group_name(const std::string &security_group_name);

                /**
                 * Gets the security group name. If nothing has been configured, empty string is returned.
                 *
                 * @return the security group name; empty string if nothing has been configured
                 */
                const std::string &get_security_group_name() const;

                /**
                 * Sets the tag key. See the filtering section above for more information.
                 *
                 * @param tagKey the tag key. See the filtering section above for more information.
                 * @return the updated ClientAwsConfig.
                 * @see #setTagKey(std::string)
                 */
                ClientAwsConfig &set_tag_key(const std::string &tag_key);

                /**
                 * Gets the tag key. If nothing is specified, empty string is returned.
                 *
                 * @return the tag key. empty string if nothing is returned.
                 */
                const std::string &get_tag_key() const;

                /**
                 * Sets the tag value. See the filtering section above for more information.
                 *
                 * @param tagValue the tag value. See the filtering section above for more information.
                 * @return the updated ClientAwsConfig.
                 * @see #setTagKey(std::string)
                 * @see #getTagValue()
                 */
                ClientAwsConfig &set_tag_value(const std::string &tag_value);

                /**
                 * Gets the tag value. If nothing is specified, empty string is returned.
                 *
                 * @return the tag value. empty string if nothing is returned.
                 */
                const std::string &get_tag_value() const;

                /**
                 * Gets the iamRole name
                 *
                 * @return the iamRole. empty string if nothing is returned.
                 * @see #setIamRole(std::string) (int32_t)
                 */
                const std::string &get_iam_role() const;

                /**
                 * Sets the tag value. See the filtering section above for more information.
                 *
                 * @param iamRole the IAM Role name.
                 * @return the updated ClientAwsConfig.
                 * @see #getIamRole()
                 */
                ClientAwsConfig &set_iam_role(const std::string &iam_role);

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
                ClientAwsConfig &set_inside_aws(bool inside_aws);
            private:
                bool enabled_;
                std::string accessKey_;
                std::string secretKey_;
                std::string region_;
                std::string securityGroupName_;
                std::string tagKey_;
                std::string tagValue_;
                std::string hostHeader_;
                std::string iamRole_;

                bool insideAws_;
            };

            std::ostream HAZELCAST_API &operator<<(std::ostream &out, const ClientAwsConfig &config);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


