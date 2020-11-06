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
#include <vector>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            class ClientAwsConfig;
        }
        namespace aws {
            namespace security {
                class HAZELCAST_API EC2RequestSigner {
                public:
                    EC2RequestSigner(const config::ClientAwsConfig &aws_config, const std::string &timestamp,
                                     const std::string &endpoint);

                    virtual ~EC2RequestSigner();

                    std::string sign(const std::unordered_map<std::string, std::string> &attributes);

                    std::string create_formatted_credential() const;

                    std::string get_canonicalized_query_string(const std::unordered_map<std::string, std::string> &attributes) const;
                private:
                    /* Task 1 */
                    std::string get_canonicalized_request(const std::unordered_map<std::string, std::string> &attributes) const;

                    std::string get_canonical_headers() const;

                    std::string get_canonicalized_query_string(const std::vector<std::string> &list) const;

                    std::vector<std::string> get_list_of_entries(const std::unordered_map<std::string, std::string> &entries) const;

                    static std::string format_attribute(const std::string &key, const std::string &value);

                    /* Task 2 */
                    std::string create_string_to_sign(const std::string &canonical_request) const;

                    std::string get_credential_scope() const;

                    /* Task 3 */
                    std::vector<unsigned char> derive_signing_key() const;

                    std::string create_signature(const std::string &string_to_sign, const std::vector<unsigned char> &signing_key) const;

                    std::string hmac_sh_a256_hex(const std::vector<unsigned char> &key, const std::string &msg) const;

                    std::string convert_to_hex_string(const unsigned char *buffer, unsigned int len) const;

                    unsigned int hmac_sh_a256_bytes(const void *key, int key_len, const std::string &msg,
                                                 unsigned char *hash) const;

                    unsigned int hmac_sh_a256_bytes(const std::string &key, const std::string &msg,
                                                 unsigned char *hash) const;

                    unsigned int hmac_sh_a256_bytes(const std::vector<unsigned char> &key, const std::string &msg,
                                                 unsigned char *hash) const;

                    unsigned int hmac_sh_a256_bytes(const void *key_buffer, int key_len, const unsigned char *data,
                                                 size_t data_len,
                                                 unsigned char *hash) const;

                    std::string sha256_hashhex(const std::string &in) const;

                    static std::string NEW_LINE;
                    static size_t DATE_LENGTH;

                    const config::ClientAwsConfig &aws_config_;
                    std::string timestamp_;
                    const std::string &endpoint_;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


