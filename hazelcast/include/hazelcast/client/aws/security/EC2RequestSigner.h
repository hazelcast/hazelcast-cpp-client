/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_AWS_SECURITY_EC2REQUESTSIGNER_H_
#define HAZELCAST_CLIENT_AWS_SECURITY_EC2REQUESTSIGNER_H_

#ifdef HZ_BUILD_WITH_SSL

#include <string>
#include <map>
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
                    EC2RequestSigner(const config::ClientAwsConfig &awsConfig, const std::string &timestamp,
                                     const std::string &endpoint);

                    std::string sign(const std::map<std::string, std::string> &attributes);

                    std::string createFormattedCredential() const;

                    std::string getCanonicalizedQueryString(const std::map<std::string, std::string> &attributes) const;
                private:
                    /* Task 1 */
                    std::string getCanonicalizedRequest(const std::map<std::string, std::string> &attributes) const;

                    std::string getCanonicalHeaders() const;

                    std::string getCanonicalizedQueryString(const std::vector<std::string> &list) const;

                    std::vector<std::string> getListOfEntries(const std::map<std::string, std::string> &entries) const;

                    void addComponents(std::vector<std::string> &components,
                                       const std::map<std::string, std::string> &attributes,
                                       const std::string &key) const;

                    /* Task 2 */
                    std::string createStringToSign(const std::string &canonicalRequest) const;

                    std::string getCredentialScope() const;

                    /* Task 3 */
                    std::vector<unsigned char> deriveSigningKey() const;

                    std::string createSignature(const std::string &stringToSign, const std::vector<unsigned char> &signingKey) const;

                    std::string hmacSHA256Hex(const std::vector<unsigned char> &key, const std::string &msg) const;

                    std::string convertToHexString(const unsigned char *buffer, unsigned int len) const;

                    std::string hmacSHA256(const std::string &key, const std::string &msg) const;

                    unsigned int hmacSHA256Bytes(const void *key, int keyLen, const std::string &msg,
                                                 unsigned char *hash) const;

                    unsigned int hmacSHA256Bytes(const std::string &key, const std::string &msg,
                                                 unsigned char *hash) const;

                    unsigned int hmacSHA256Bytes(const std::vector<unsigned char> &key, const std::string &msg,
                                                 unsigned char *hash) const;

                    unsigned int hmacSHA256Bytes(const void *keyBuffer, int keyLen, const unsigned char *data,
                                                 size_t dataLen,
                                                 unsigned char *hash) const;

                    std::string sha256Hashhex(const std::string &in) const;

                    static std::string NEW_LINE;
                    static size_t DATE_LENGTH;

                    const config::ClientAwsConfig &awsConfig;
                    std::string timestamp;
                    const std::string &endpoint;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HZ_BUILD_WITH_SSL

#endif /* HAZELCAST_CLIENT_AWS_SECURITY_EC2REQUESTSIGNER_H_ */
