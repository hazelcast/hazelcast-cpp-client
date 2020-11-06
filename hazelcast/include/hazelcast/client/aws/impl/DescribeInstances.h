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
#include <unordered_map>
#include <memory>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/aws/security/EC2RequestSigner.h"
#include "hazelcast/util/SyncHttpsClient.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    class logger;

    namespace client {
        namespace config {
            class ClientAwsConfig;
        }
        namespace aws {
            namespace impl {
                /**
                 * See http://docs.aws.amazon.com/AWSEC2/latest/APIReference/API_DescribeInstances.html
                 * for AWS API details.
                 */
                class HAZELCAST_API DescribeInstances {
                public:
                    DescribeInstances(config::ClientAwsConfig &aws_config, const std::string &endpoint,
                                      logger &lg);

                    virtual ~DescribeInstances();

                    /**
                     * Invoke the service to describe the instances, unmarshal the response and return the discovered node map.
                     * The map contains mappings from private to public IP and all contained nodes match the filtering rules defined by
                     * the {@link #ClientAwsConfig}.
                     *
                     * @return map from private to public IP or empty map in case of failed response unmarshalling
                     * @throws IException if there is an exception invoking the service
                     */
                    std::unordered_map<std::string, std::string> execute();
                private:
                    static std::string get_formatted_timestamp();

                    std::istream &call_service();

                    void check_keys_from_iam_roles();
                    void try_get_default_iam_role();
                    void get_keys_from_iam_task_role();
                    void get_keys_from_iam_role();
                    void parse_and_store_role_creds(std::istream &in);

                    /**
                     * Add available filters to narrow down the scope of the query
                     */
                    void add_filters();

                    std::unique_ptr<security::EC2RequestSigner> rs_;
                    config::ClientAwsConfig &aws_config_;
                    const std::string &endpoint_;
                    std::unordered_map<std::string, std::string> attributes_;
                    std::unique_ptr<util::SyncHttpsClient> https_client_;
                    logger &logger_;

                    static const std::string QUERY_PREFIX;
                    static const std::string IAM_ROLE_ENDPOINT;
                    static const std::string IAM_ROLE_QUERY;
                    static const std::string IAM_TASK_ROLE_ENDPOINT;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


