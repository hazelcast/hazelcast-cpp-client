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
#ifdef HZ_BUILD_WITH_SSL

#include <sstream>
#include <iomanip>

#include <boost/algorithm/string/replace.hpp>

#include <openssl/ssl.h>

#include "hazelcast/client/aws/utility/AwsURLEncoder.h"
#include "hazelcast/client/aws/impl/Constants.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/aws/security/EC2RequestSigner.h"

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace security {
                std::string EC2RequestSigner::NEW_LINE = "\n";
                size_t EC2RequestSigner::DATE_LENGTH = 8;

                EC2RequestSigner::EC2RequestSigner(const config::ClientAwsConfig &awsConfig,
                                                   const std::string &timestamp,
                                                   const std::string &endpoint) : awsConfig(awsConfig),
                                                                                  timestamp(timestamp),
                                                                                  endpoint(endpoint) {
                }

                std::string EC2RequestSigner::sign(const std::map<std::string, std::string> &attributes) {
                    std::string canonicalRequest = getCanonicalizedRequest(attributes);
                    std::string stringToSign = createStringToSign(canonicalRequest);
                    std::vector<unsigned char> signingKey = deriveSigningKey();

                    return createSignature(stringToSign, signingKey);
                }

                std::string EC2RequestSigner::createFormattedCredential() const {
                    std::stringstream out;
                    out << awsConfig.getAccessKey() << '/' << timestamp.substr(0, DATE_LENGTH) << '/'
                    << awsConfig.getRegion() << '/' << "ec2/aws4_request";
                    return out.str();
                }

                std::string EC2RequestSigner::getCanonicalizedQueryString(
                        const std::map<std::string, std::string> &attributes) const {
                    std::vector<std::string> components = getListOfEntries(attributes);
                    std::sort(components.begin(), components.end());
                    return getCanonicalizedQueryString(components);
                }

                /* Task 1 */
                std::string EC2RequestSigner::getCanonicalizedRequest(
                        const std::map<std::string, std::string> &attributes) const {
                    std::ostringstream out;
                    out << impl::Constants::GET << NEW_LINE
                    << '/' << NEW_LINE
                    << getCanonicalizedQueryString(attributes) << NEW_LINE
                    << getCanonicalHeaders() << NEW_LINE
                    << "host" << NEW_LINE
                    << sha256Hashhex("");
                    return out.str();
                }

                std::string EC2RequestSigner::getCanonicalHeaders() const {
                    std::ostringstream out;
                    out << "host:" << endpoint << NEW_LINE;
                    return out.str();
                }

                std::string EC2RequestSigner::getCanonicalizedQueryString(const std::vector<std::string> &list) const {
                    std::ostringstream result;
                    std::vector<std::string>::const_iterator it = list.begin();
                    result << (*it);
                    ++it;
                    for (; it != list.end(); ++it) {
                        result << "&" << *it;
                    }
                    return result.str();
                }

                std::vector<std::string> EC2RequestSigner::getListOfEntries(
                        const std::map<std::string, std::string> &entries) const {
                    std::vector<std::string> components;
                    for (std::map<std::string, std::string>::const_iterator it = entries.begin();
                         it != entries.end(); ++it) {
                        addComponents(components, entries, it->first);
                    }
                    return components;
                }

                void EC2RequestSigner::addComponents(std::vector<std::string> &components,
                                                     const std::map<std::string, std::string> &attributes,
                                                     const std::string &key) const {
                    std::ostringstream out;
                    out << utility::AwsURLEncoder::urlEncode(key) << '=' <<
                    utility::AwsURLEncoder::urlEncode(attributes.at(key));
                    components.push_back(out.str());
                }

                /* Task 2 */
                std::string EC2RequestSigner::createStringToSign(const std::string &canonicalRequest) const {
                    std::ostringstream out;
                    out << impl::Constants::SIGNATURE_METHOD_V4 << NEW_LINE
                    << timestamp << NEW_LINE
                    << getCredentialScope() << NEW_LINE
                    << sha256Hashhex(canonicalRequest);
                    return out.str();
                }

                std::string EC2RequestSigner::getCredentialScope() const {
                    // datestamp/region/service/API_TERMINATOR
                    // dateStamp
                    std::ostringstream out;
                    out << timestamp.substr(0, DATE_LENGTH) << "/" << awsConfig.getRegion() << "/ec2/aws4_request";
                    return out.str();
                }

                /* Task 3 */
                std::vector<unsigned char> EC2RequestSigner::deriveSigningKey() const {
                    const std::string &signKey = awsConfig.getSecretKey();
                    std::string dateStamp = timestamp.substr(0, DATE_LENGTH);
                    // this is derived from
                    // http://docs.aws.amazon.com/general/latest/gr/signature-v4-examples.html#signature-v4-examples-python

                    unsigned char kDate[32];
                    std::string key = std::string("AWS4") + signKey;
                    int kDateLen = hmacSHA256Bytes(key, dateStamp, kDate);

                    unsigned char kRegion[32];
                    int kRegionLen = hmacSHA256Bytes(kDate, kDateLen, awsConfig.getRegion(), kRegion);

                    unsigned char kService[32];
                    int kServiceLen = hmacSHA256Bytes(kRegion, kRegionLen, "ec2", kService);

                    std::vector<unsigned char> mSigning(32);
                    hmacSHA256Bytes(kService, kServiceLen, "aws4_request", &mSigning[0]);

                    return mSigning;
                }

                std::string EC2RequestSigner::createSignature(const std::string &stringToSign,
                                                              const std::vector<unsigned char> &signingKey) const {
                    return hmacSHA256Hex(signingKey, stringToSign);
                }

                std::string EC2RequestSigner::hmacSHA256Hex(const std::vector<unsigned char> &key,
                                                            const std::string &msg) const {
                    unsigned char hash[32];

                    unsigned int len = hmacSHA256Bytes(key, msg, hash);

                    return convertToHexString(hash, len);
                }

                std::string EC2RequestSigner::convertToHexString(const unsigned char *buffer, unsigned int len) const {
                    std::stringstream ss;
                    ss << std::hex << std::setfill('0');
                    for (unsigned int i = 0; i < len; i++) {
                        ss << std::hex << std::setw(2) << (unsigned int) buffer[i];
                    }

                    return (ss.str());
                }

                std::string EC2RequestSigner::hmacSHA256(const std::string &key, const std::string &msg) const {
                    char hash[33];

                    unsigned int len = hmacSHA256Bytes(key, msg, (unsigned char *) hash);

                    hash[len] = '\0';

                    return hash;
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const void *key, int keyLen, const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmacSHA256Bytes(key, keyLen, (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const std::string &key, const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmacSHA256Bytes(&key[0], (int) key.length(), (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const std::vector<unsigned char> &key,
                                                               const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmacSHA256Bytes(&key[0], (int) key.size(), (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const void *keyBuffer, int keyLen,
                                                               const unsigned char *data,
                                                               size_t dataLen,
                                                               unsigned char *hash) const {
                    HMAC_CTX hmac;
                    HMAC_CTX_init(&hmac);
                    HMAC_Init_ex(&hmac, keyBuffer, keyLen, EVP_sha256(), NULL);
                    HMAC_Update(&hmac, data, dataLen);
                    unsigned int len = 32;
                    HMAC_Final(&hmac, hash, &len);
                    HMAC_CTX_cleanup(&hmac);
                    return len;
                }

                std::string EC2RequestSigner::sha256Hashhex(const std::string &in) const {
                    unsigned char hash[SHA256_DIGEST_LENGTH];
                    SHA256_CTX sha256;
                    SHA256_Init(&sha256);
                    SHA256_Update(&sha256, in.c_str(), in.size());
                    SHA256_Final(hash, &sha256);

                    return convertToHexString(hash, SHA256_DIGEST_LENGTH);
                }
            }
        }
    }
}
#endif // HZ_BUILD_WITH_SSL

