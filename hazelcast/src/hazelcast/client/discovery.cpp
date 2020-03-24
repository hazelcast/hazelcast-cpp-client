/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#include <sstream>
#include <iomanip>

#ifdef HZ_BUILD_WITH_SSL

#include <openssl/ssl.h>

#endif

#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "hazelcast/client/aws/utility/AwsURLEncoder.h"
#include "hazelcast/client/aws/impl/Constants.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/aws/security/EC2RequestSigner.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/aws/impl/Filter.h"
#include "hazelcast/client/aws/impl/AwsAddressTranslator.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/aws/impl/DescribeInstances.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/util/SyncHttpClient.h"

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

                EC2RequestSigner::~EC2RequestSigner() {
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
                    out << utility::AwsURLEncoder::urlEncode(key) << '=' << utility::AwsURLEncoder::urlEncode(
                            (const_cast<std::map<std::string, std::string> &>(attributes))[key]);
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
#ifdef HZ_BUILD_WITH_SSL

#if OPENSSL_VERSION_NUMBER > 0x10100000L
                    HMAC_CTX *hmac = HMAC_CTX_new();
#else
                    HMAC_CTX *hmac = new HMAC_CTX;
                    HMAC_CTX_init(hmac);
#endif

                    HMAC_Init_ex(hmac, keyBuffer, keyLen, EVP_sha256(), NULL);
                    HMAC_Update(hmac, data, dataLen);
                    unsigned int len = 32;
                    HMAC_Final(hmac, hash, &len);

#if OPENSSL_VERSION_NUMBER > 0x10100000L
                    HMAC_CTX_free(hmac);
#else
                    HMAC_CTX_cleanup(hmac);
                    delete hmac;
#endif

                    return len;
#else
                    util::Preconditions::checkSSL("EC2RequestSigner::hmacSHA256Bytes");
                    return 0;
#endif
                }

                std::string EC2RequestSigner::sha256Hashhex(const std::string &in) const {
#ifdef HZ_BUILD_WITH_SSL
#ifdef OPENSSL_FIPS
                    unsigned int hashLen = 0;
                        unsigned char hash[EVP_MAX_MD_SIZE];
                        EVP_MD_CTX ctx;
                        EVP_MD_CTX_init(&ctx);
                        EVP_DigestInit_ex(&ctx, EVP_sha256(), NULL);
                        EVP_DigestUpdate(&ctx, in.c_str(), in.size());
                        EVP_DigestFinal_ex(&ctx, hash, &hashLen);
                        EVP_MD_CTX_cleanup(&ctx);
                        return convertToHexString(hash, hashLen);
#else
                    unsigned char hash[SHA256_DIGEST_LENGTH];
                    SHA256_CTX sha256;
                    SHA256_Init(&sha256);
                    SHA256_Update(&sha256, in.c_str(), in.size());
                    SHA256_Final(hash, &sha256);

                    return convertToHexString(hash, SHA256_DIGEST_LENGTH);
#endif // OPENSSL_FIPS
#else
                    util::Preconditions::checkSSL("EC2RequestSigner::hmacSHA256Bytes");
                    return "";
#endif // HZ_BUILD_WITH_SSL
                }
            }

            namespace impl {
                const char *Constants::DATE_FORMAT = "%Y%m%dT%H%M%SZ";
                const char *Constants::DOC_VERSION = "2016-11-15";
                const char *Constants::SIGNATURE_METHOD_V4 = "AWS4-HMAC-SHA256";
                const char *Constants::GET = "GET";
                const char *Constants::ECS_CREDENTIALS_ENV_VAR_NAME = "AWS_CONTAINER_CREDENTIALS_RELATIVE_URI";

                Filter::Filter() {
                }

                /**
                 *
                 * Add a new filter with the given name and value to the query.
                 *
                 * @param name Filter name
                 * @param value Filter value
                 *
                 */
                void Filter::addFilter(const std::string &name, const std::string &value) {
                    std::stringstream out;
                    unsigned long index = filters.size() + 1;
                    out << "Filter." << index << ".Name";
                    filters[out.str()] = name;
                    out.str("");
                    out.clear();
                    out << "Filter." << index << ".Value.1";
                    filters[out.str()] = value;
                }

                const std::map<std::string, std::string> &Filter::getFilters() {
                    return filters;
                }

                AwsAddressTranslator::AwsAddressTranslator(config::ClientAwsConfig &awsConfig, util::ILogger &logger)
                        : logger(logger) {
                    if (awsConfig.isEnabled() && !awsConfig.isInsideAws()) {
                        awsClient = std::unique_ptr<AWSClient>(new AWSClient(awsConfig, logger));
                    }
                }

                Address AwsAddressTranslator::translate(const Address &address) {
                    // if no translation is needed just return the address as it is
                    if (NULL == awsClient.get()) {
                        return address;
                    }

                    Address translatedAddress = address;

                    if (findFromCache(address, translatedAddress)) {
                        return translatedAddress;
                    }

                    refresh();

                    if (findFromCache(address, translatedAddress)) {
                        return translatedAddress;
                    }

                    std::stringstream out;
                    out << "No translation is found for private ip:" << address;
                    throw exception::IOException("AwsAddressTranslator::translate", out.str());
                }

                void AwsAddressTranslator::refresh() {
                    try {
                        privateToPublic = std::shared_ptr<std::map<std::string, std::string> >(
                                new std::map<std::string, std::string>(awsClient->getAddresses()));
                    } catch (exception::IException &e) {
                        logger.warning(std::string("AWS addresses failed to load: ") + e.what());
                    }
                }

                bool AwsAddressTranslator::findFromCache(const Address &address, Address &translatedAddress) {
                    std::shared_ptr<std::map<std::string, std::string> > mapping = privateToPublic;
                    if (mapping.get() == NULL) {
                        return false;
                    }

                    std::map<std::string, std::string>::const_iterator publicAddressIt = mapping->find(
                            address.getHost());
                    if (publicAddressIt != mapping->end()) {
                        const std::string &publicIp = (*publicAddressIt).second;
                        if (!publicIp.empty()) {
                            translatedAddress = Address((*publicAddressIt).second, address.getPort());
                            return true;
                        }
                    }

                    return false;
                }

                const std::string DescribeInstances::QUERY_PREFIX = "/?";
                const std::string DescribeInstances::IAM_ROLE_ENDPOINT = "169.254.169.254";
                const std::string DescribeInstances::IAM_ROLE_QUERY = "/latest/meta-data/iam/security-credentials/";
                const std::string DescribeInstances::IAM_TASK_ROLE_ENDPOINT = "169.254.170.2";

                DescribeInstances::DescribeInstances(config::ClientAwsConfig &awsConfig, const std::string &endpoint,
                                                     util::ILogger &logger) : awsConfig(awsConfig), endpoint(endpoint),
                                                                              logger(logger) {
                    checkKeysFromIamRoles();

                    std::string timeStamp = getFormattedTimestamp();
                    rs = std::unique_ptr<security::EC2RequestSigner>(
                            new security::EC2RequestSigner(awsConfig, timeStamp, endpoint));
                    attributes["Action"] = "DescribeInstances";
                    attributes["Version"] = impl::Constants::DOC_VERSION;
                    attributes["X-Amz-Algorithm"] = impl::Constants::SIGNATURE_METHOD_V4;
                    attributes["X-Amz-Credential"] = rs->createFormattedCredential();
                    attributes["X-Amz-Date"] = timeStamp;
                    attributes["X-Amz-SignedHeaders"] = "host";
                    attributes["X-Amz-Expires"] = "30";
                    addFilters();
                }

                DescribeInstances::~DescribeInstances() {
                }

                std::map<std::string, std::string> DescribeInstances::execute() {
                    std::string signature = rs->sign(attributes);
                    attributes["X-Amz-Signature"] = signature;

                    std::istream &stream = callService();
                    return utility::CloudUtility::unmarshalTheResponse(stream, logger);
                }

                std::string DescribeInstances::getFormattedTimestamp() {
                    using namespace boost::posix_time;
                    ptime now = second_clock::universal_time();

                    std::ostringstream out;
                    std::locale timeLocale(out.getloc(), new time_facet(impl::Constants::DATE_FORMAT));
                    out.imbue(timeLocale);
                    out << now;
                    return out.str();
                }

                std::istream &DescribeInstances::callService() {
                    std::string query = rs->getCanonicalizedQueryString(attributes);
                    httpsClient = std::unique_ptr<util::SyncHttpsClient>(
                            new util::SyncHttpsClient(endpoint.c_str(), QUERY_PREFIX + query));
                    return httpsClient->openConnection();
                }

                void DescribeInstances::checkKeysFromIamRoles() {
                    if (awsConfig.getAccessKey().empty() || !awsConfig.getIamRole().empty()) {
                        tryGetDefaultIamRole();
                        if (!awsConfig.getIamRole().empty()) {
                            getKeysFromIamRole();
                        } else {
                            getKeysFromIamTaskRole();
                        }
                    }
                }

                void DescribeInstances::tryGetDefaultIamRole() {
                    // if none of the below are true
                    if (!(awsConfig.getIamRole().empty() || awsConfig.getIamRole() == "DEFAULT")) {
                        // stop here. No point looking up the default role.
                        return;
                    }
                    try {
                        util::SyncHttpClient httpClient(IAM_ROLE_ENDPOINT, IAM_ROLE_QUERY);
                        std::string roleName;
                        std::istream &responseStream = httpClient.openConnection();
                        responseStream >> roleName;
                        awsConfig.setIamRole(roleName);
                    } catch (exception::IOException &e) {
                        throw exception::InvalidConfigurationException("tryGetDefaultIamRole",
                                                                       std::string("Invalid Aws Configuration. ") +
                                                                       e.what());
                    }
                }

                void DescribeInstances::getKeysFromIamTaskRole() {
                    // before giving up, attempt to discover whether we're running in an ECS Container,
                    // in which case, AWS_CONTAINER_CREDENTIALS_RELATIVE_URI will exist as an env var.
                    const char *uri = getenv(Constants::ECS_CREDENTIALS_ENV_VAR_NAME);
                    if (!uri) {
                        throw exception::IllegalArgumentException("getKeysFromIamTaskRole",
                                                                  "Could not acquire credentials! Did not find declared AWS access key or IAM Role, and could not discover IAM Task Role or default role.");
                    }

                    util::SyncHttpClient httpClient(IAM_TASK_ROLE_ENDPOINT, uri);

                    try {
                        std::istream &istream = httpClient.openConnection();
                        parseAndStoreRoleCreds(istream);
                    } catch (exception::IException &e) {
                        std::stringstream out;
                        out << "Unable to retrieve credentials from IAM Task Role. URI: " << uri << ". \n " << e.what();
                        throw exception::InvalidConfigurationException("getKeysFromIamTaskRole", out.str());
                    }
                }

                void DescribeInstances::getKeysFromIamRole() {
                    std::string query = "/latest/meta-data/iam/security-credentials/" + awsConfig.getIamRole();

                    util::SyncHttpClient httpClient(IAM_ROLE_ENDPOINT, query);

                    try {
                        std::istream &istream = httpClient.openConnection();
                        parseAndStoreRoleCreds(istream);
                    } catch (exception::IException &e) {
                        std::stringstream out;
                        out << "Unable to retrieve credentials from IAM Task Role. URI: " << query << ". \n "
                            << e.what();
                        throw exception::InvalidConfigurationException("getKeysFromIamRole", out.str());
                    }
                }

                void DescribeInstances::parseAndStoreRoleCreds(std::istream &in) {
                    utility::CloudUtility::unmarshalJsonResponse(in, awsConfig, attributes);
                }

                /**
                 * Add available filters to narrow down the scope of the query
                 */
                void DescribeInstances::addFilters() {
                    Filter filter;
                    if (!awsConfig.getTagKey().empty()) {
                        if (!awsConfig.getTagValue().empty()) {
                            filter.addFilter(std::string("tag:") + awsConfig.getTagKey(), awsConfig.getTagValue());
                        } else {
                            filter.addFilter("tag-key", awsConfig.getTagKey());
                        }
                    } else if (!awsConfig.getTagValue().empty()) {
                        filter.addFilter("tag-value", awsConfig.getTagValue());
                    }

                    if (!awsConfig.getSecurityGroupName().empty()) {
                        filter.addFilter("instance.group-name", awsConfig.getSecurityGroupName());
                    }

                    filter.addFilter("instance-state-name", "running");
                    const std::map<std::string, std::string> &filters = filter.getFilters();
                    attributes.insert(filters.begin(), filters.end());
                }

            }

            namespace utility {
                std::string AwsURLEncoder::urlEncode(const std::string &value) {
                    std::string result = escapeEncode(value);
                    boost::replace_all(result, "+", "%20");
                    return result;
                }

                std::string AwsURLEncoder::escapeEncode(const std::string &value) {
                    std::ostringstream escaped;
                    escaped.fill('0');
                    escaped << std::hex;

                    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
                        std::string::value_type c = (*i);

                        // Keep alphanumeric and other accepted characters intact
                        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                            escaped << c;
                            continue;
                        }

                        // Any other characters are percent-encoded
                        escaped << std::uppercase;
                        escaped << '%' << std::setw(2) << int((unsigned char) c);
                        escaped << std::nouppercase;
                    }

                    return escaped.str();
                }

                std::map<std::string, std::string> CloudUtility::unmarshalTheResponse(std::istream &stream,
                                                                                      util::ILogger &logger) {
                    std::map<std::string, std::string> privatePublicPairs;

                    pt::ptree tree;
                    try {
                        pt::read_xml(stream, tree);
                    } catch (pt::xml_parser_error &e) {
                        logger.warning(
                                std::string("The parsed xml stream has errors: ") + e.what());
                        return privatePublicPairs;
                    }

                    // Use get_child to find the node containing the reservation set, and iterate over
                    // its children.
                    for (pt::ptree::value_type &item : tree.get_child("DescribeInstancesResponse.reservationSet")) {
                        for (pt::ptree::value_type &instanceItem : item.second.get_child("instancesSet")) {
                            auto privateIp = instanceItem.second.get_optional<std::string>("privateIpAddress");
                            auto publicIp = instanceItem.second.get_optional<std::string>("ipAddress");

                            auto prIp = privateIp.value_or("");
                            auto pubIp = publicIp.value_or("");

                            if (privateIp) {
                                privatePublicPairs[prIp] = pubIp;
                                if (logger.isFinestEnabled()) {
                                    auto instName = instanceItem.second.get_optional<std::string>(
                                            "tagset.item.value").value_or("");

                                    logger.finest(
                                            std::string("Accepting EC2 instance [") + instName +
                                            "][" + prIp + "]");
                                }
                            }
                        }
                    }
                    return privatePublicPairs;
                }

                void CloudUtility::unmarshalJsonResponse(std::istream &stream, config::ClientAwsConfig &awsConfig,
                                                         std::map<std::string, std::string> &attributes) {
                    pt::ptree json;
                    pt::read_json(stream, json);
                    awsConfig.setAccessKey(json.get_optional<std::string>("AccessKeyId").get_value_or(""));
                    awsConfig.setSecretKey(json.get_optional<std::string>("SecretAccessKey").get_value_or(""));
                    attributes["X-Amz-Security-Token"] = json.get_optional<std::string>("Token").get_value_or("");
                }

            }

            AWSClient::AWSClient(config::ClientAwsConfig &awsConfig, util::ILogger &logger) : awsConfig(awsConfig),
                                                                                              logger(logger) {
                this->endpoint = awsConfig.getHostHeader();
                if (!awsConfig.getRegion().empty() && awsConfig.getRegion().length() > 0) {
                    if (awsConfig.getHostHeader().find("ec2.") != 0) {
                        throw exception::InvalidConfigurationException("AWSClient::AWSClient",
                                                                       "HostHeader should start with \"ec2.\" prefix");
                    }
                    boost::replace_all(this->endpoint, "ec2.", std::string("ec2.") + awsConfig.getRegion() + ".");
                }
            }

            std::map<std::string, std::string> AWSClient::getAddresses() {
                return impl::DescribeInstances(awsConfig, endpoint, logger).execute();
            }

        }
    }
}
