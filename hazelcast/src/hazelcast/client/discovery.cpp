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

#include <sstream>
#include <iomanip>

#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "hazelcast/client/aws/utility/AwsURLEncoder.h"
#include "hazelcast/client/aws/impl/Constants.h"
#include "hazelcast/client/config/client_aws_config.h"
#include "hazelcast/client/aws/security/EC2RequestSigner.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/aws/impl/Filter.h"
#include "hazelcast/client/aws/impl/AwsAddressTranslator.h"
#include "hazelcast/client/aws/impl/DescribeInstances.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/util/SyncHttpClient.h"
#include "hazelcast/logger.h"

// openssl include should be after the other so that winsock.h and winsock2.h conflict does not occur at windows
#ifdef HZ_BUILD_WITH_SSL
#include <openssl/ssl.h>
#endif

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace security {
                std::string EC2RequestSigner::NEW_LINE = "\n";
                size_t EC2RequestSigner::DATE_LENGTH = 8;

                EC2RequestSigner::EC2RequestSigner(const config::client_aws_config &aws_config,
                                                   const std::string &timestamp,
                                                   const std::string &endpoint) : aws_config_(aws_config),
                                                                                  timestamp_(timestamp),
                                                                                  endpoint_(endpoint) {
                }

                EC2RequestSigner::~EC2RequestSigner() = default;

                std::string EC2RequestSigner::sign(const std::unordered_map<std::string, std::string> &attributes) {
                    std::string canonicalRequest = get_canonicalized_request(attributes);
                    std::string stringToSign = create_string_to_sign(canonicalRequest);
                    std::vector<unsigned char> signingKey = derive_signing_key();

                    return create_signature(stringToSign, signingKey);
                }

                std::string EC2RequestSigner::create_formatted_credential() const {
                    std::stringstream out;
                    out << aws_config_.get_access_key() << '/' << timestamp_.substr(0, DATE_LENGTH) << '/'
                        << aws_config_.get_region() << '/' << "ec2/aws4_request";
                    return out.str();
                }

                std::string EC2RequestSigner::get_canonicalized_query_string(
                        const std::unordered_map<std::string, std::string> &attributes) const {
                    std::vector<std::string> components = get_list_of_entries(attributes);
                    std::sort(components.begin(), components.end());
                    return get_canonicalized_query_string(components);
                }

                /* Task 1 */
                std::string EC2RequestSigner::get_canonicalized_request(
                        const std::unordered_map<std::string, std::string> &attributes) const {
                    std::ostringstream out;
                    out << impl::Constants::GET << NEW_LINE
                        << '/' << NEW_LINE
                        << get_canonicalized_query_string(attributes) << NEW_LINE
                        << get_canonical_headers() << NEW_LINE
                        << "host" << NEW_LINE
                        << sha256_hashhex("");
                    return out.str();
                }

                std::string EC2RequestSigner::get_canonical_headers() const {
                    std::ostringstream out;
                    out << "host:" << endpoint_ << NEW_LINE;
                    return out.str();
                }

                std::string EC2RequestSigner::get_canonicalized_query_string(const std::vector<std::string> &list) const {
                    std::ostringstream result;
                    std::vector<std::string>::const_iterator it = list.begin();
                    result << (*it);
                    ++it;
                    for (; it != list.end(); ++it) {
                        result << "&" << *it;
                    }
                    return result.str();
                }

                std::vector<std::string> EC2RequestSigner::get_list_of_entries(
                        const std::unordered_map<std::string, std::string> &entries) const {
                    std::vector<std::string> components;
                    for (const auto &entry: entries) {
                        components.push_back(format_attribute(entry.first, entry.second));
                    }
                    return components;
                }

                std::string EC2RequestSigner::format_attribute(const std::string &key, const std::string &value) {
                    std::ostringstream out;
                    out << utility::AwsURLEncoder::url_encode(key) << '=' 
                        << utility::AwsURLEncoder::url_encode(value);
                    return out.str();
                }

                /* Task 2 */
                std::string EC2RequestSigner::create_string_to_sign(const std::string &canonical_request) const {
                    std::ostringstream out;
                    out << impl::Constants::SIGNATURE_METHOD_V4 << NEW_LINE
                        << timestamp_ << NEW_LINE
                        << get_credential_scope() << NEW_LINE
                        << sha256_hashhex(canonical_request);
                    return out.str();
                }

                std::string EC2RequestSigner::get_credential_scope() const {
                    // datestamp/region/service/API_TERMINATOR
                    // dateStamp
                    std::ostringstream out;
                    out << timestamp_.substr(0, DATE_LENGTH) << "/" << aws_config_.get_region() << "/ec2/aws4_request";
                    return out.str();
                }

                /* Task 3 */
                std::vector<unsigned char> EC2RequestSigner::derive_signing_key() const {
                    const std::string &signKey = aws_config_.get_secret_key();
                    std::string dateStamp = timestamp_.substr(0, DATE_LENGTH);
                    // this is derived from
                    // http://docs.aws.amazon.com/general/latest/gr/signature-v4-examples.html#signature-v4-examples-python

                    unsigned char kDate[32];
                    std::string key = std::string("AWS4") + signKey;
                    int kDateLen = hmac_sh_a256_bytes(key, dateStamp, kDate);

                    unsigned char kRegion[32];
                    int kRegionLen = hmac_sh_a256_bytes(kDate, kDateLen, aws_config_.get_region(), kRegion);

                    unsigned char kService[32];
                    int kServiceLen = hmac_sh_a256_bytes(kRegion, kRegionLen, "ec2", kService);

                    std::vector<unsigned char> mSigning(32);
                    hmac_sh_a256_bytes(kService, kServiceLen, "aws4_request", &mSigning[0]);

                    return mSigning;
                }

                std::string EC2RequestSigner::create_signature(const std::string &string_to_sign,
                                                              const std::vector<unsigned char> &signing_key) const {
                    return hmac_sh_a256_hex(signing_key, string_to_sign);
                }

                std::string EC2RequestSigner::hmac_sh_a256_hex(const std::vector<unsigned char> &key,
                                                            const std::string &msg) const {
                    unsigned char hash[32];

                    unsigned int len = hmac_sh_a256_bytes(key, msg, hash);

                    return convert_to_hex_string(hash, len);
                }

                std::string EC2RequestSigner::convert_to_hex_string(const unsigned char *buffer, unsigned int len) const {
                    std::stringstream ss;
                    ss << std::hex << std::setfill('0');
                    for (unsigned int i = 0; i < len; i++) {
                        ss << std::hex << std::setw(2) << (unsigned int) buffer[i];
                    }

                    return (ss.str());
                }

                unsigned int EC2RequestSigner::hmac_sh_a256_bytes(const void *key, int key_len, const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmac_sh_a256_bytes(key, key_len, (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmac_sh_a256_bytes(const std::string &key, const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmac_sh_a256_bytes(&key[0], (int) key.length(), (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmac_sh_a256_bytes(const std::vector<unsigned char> &key,
                                                               const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmac_sh_a256_bytes(&key[0], (int) key.size(), (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmac_sh_a256_bytes(const void *key_buffer, int key_len,
                                                               const unsigned char *data,
                                                               size_t data_len,
                                                               unsigned char *hash) const {
#ifdef HZ_BUILD_WITH_SSL

#if OPENSSL_VERSION_NUMBER > 0x10100000L
                    HMAC_CTX *hmac = HMAC_CTX_new();
#else
                    HMAC_CTX *hmac = new HMAC_CTX;
                    HMAC_CTX_init(hmac);
#endif

                    HMAC_Init_ex(hmac, key_buffer, key_len, EVP_sha256(), NULL);
                    HMAC_Update(hmac, data, data_len);
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
                    util::Preconditions::check_ssl("EC2RequestSigner::hmacSHA256Bytes");
                    return 0;
#endif
                }

                std::string EC2RequestSigner::sha256_hashhex(const std::string &in) const {
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
                        return convert_to_hex_string(hash, hashLen);
#else
                    unsigned char hash[SHA256_DIGEST_LENGTH];
                    SHA256_CTX sha256;
                    SHA256_Init(&sha256);
                    SHA256_Update(&sha256, in.c_str(), in.size());
                    SHA256_Final(hash, &sha256);

                    return convert_to_hex_string(hash, SHA256_DIGEST_LENGTH);
#endif // OPENSSL_FIPS
#else
                    util::Preconditions::check_ssl("EC2RequestSigner::hmacSHA256Bytes");
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

                Filter::Filter() = default;

                /**
                 *
                 * Add a new filter with the given name and value to the query.
                 *
                 * @param name Filter name
                 * @param value Filter value
                 *
                 */
                void Filter::add_filter(const std::string &name, const std::string &value) {
                    std::stringstream out;
                    unsigned long index = filters_.size() + 1;
                    out << "Filter." << index << ".Name";
                    filters_[out.str()] = name;
                    out.str("");
                    out.clear();
                    out << "Filter." << index << ".Value.1";
                    filters_[out.str()] = value;
                }

                const std::unordered_map<std::string, std::string> &Filter::get_filters() {
                    return filters_;
                }

                AwsAddressTranslator::AwsAddressTranslator(config::client_aws_config &aws_config, logger &lg)
                        : logger_(lg) {
                    if (aws_config.is_enabled() && !aws_config.is_inside_aws()) {
                        aws_client_ = std::unique_ptr<AWSClient>(new AWSClient(aws_config, lg));
                    }
                }

                address AwsAddressTranslator::translate(const address &addr) {
                    // if no translation is needed just return the address as it is
                    if (NULL == aws_client_.get()) {
                        return addr;
                    }

                    address translatedAddress = addr;

                    if (find_from_cache(addr, translatedAddress)) {
                        return translatedAddress;
                    }

                    refresh();

                    if (find_from_cache(addr, translatedAddress)) {
                        return translatedAddress;
                    }

                    std::stringstream out;
                    out << "No translation is found for private ip:" << addr;
                    BOOST_THROW_EXCEPTION(exception::io("AwsAddressTranslator::translate", out.str()));
                }

                void AwsAddressTranslator::refresh() {
                    try {
                        private_to_public_ = std::shared_ptr<std::unordered_map<std::string, std::string> >(
                                new std::unordered_map<std::string, std::string>(aws_client_->get_addresses()));
                    } catch (exception::iexception &e) {
                        HZ_LOG(logger_, warning,
                            boost::str(boost::format("AWS addresses failed to load: %1%") % e.what()));
                    }
                }

                bool AwsAddressTranslator::find_from_cache(const address &addr, address &translated_address) {
                    std::shared_ptr<std::unordered_map<std::string, std::string> > mapping = private_to_public_;
                    if (mapping.get() == NULL) {
                        return false;
                    }

                    std::unordered_map<std::string, std::string>::const_iterator publicAddressIt = mapping->find(
                            addr.get_host());
                    if (publicAddressIt != mapping->end()) {
                        const std::string &publicIp = (*publicAddressIt).second;
                        if (!publicIp.empty()) {
                            translated_address = address((*publicAddressIt).second, addr.get_port());
                            return true;
                        }
                    }

                    return false;
                }

                const std::string DescribeInstances::QUERY_PREFIX = "/?";
                const std::string DescribeInstances::IAM_ROLE_ENDPOINT = "169.254.169.254";
                const std::string DescribeInstances::IAM_ROLE_QUERY = "/latest/meta-data/iam/security-credentials/";
                const std::string DescribeInstances::IAM_TASK_ROLE_ENDPOINT = "169.254.170.2";

                DescribeInstances::DescribeInstances(config::client_aws_config &aws_config, const std::string &endpoint,
                                                     logger &lg) : aws_config_(aws_config), endpoint_(endpoint),
                                                                              logger_(lg) {
                    check_keys_from_iam_roles();

                    std::string timeStamp = get_formatted_timestamp();
                    rs_ = std::unique_ptr<security::EC2RequestSigner>(
                            new security::EC2RequestSigner(aws_config, timeStamp, endpoint));
                    attributes_["Action"] = "DescribeInstances";
                    attributes_["Version"] = impl::Constants::DOC_VERSION;
                    attributes_["X-Amz-Algorithm"] = impl::Constants::SIGNATURE_METHOD_V4;
                    attributes_["X-Amz-Credential"] = rs_->create_formatted_credential();
                    attributes_["X-Amz-Date"] = timeStamp;
                    attributes_["X-Amz-SignedHeaders"] = "host";
                    attributes_["X-Amz-Expires"] = "30";
                    add_filters();
                }

                DescribeInstances::~DescribeInstances() = default;

                std::unordered_map<std::string, std::string> DescribeInstances::execute() {
                    std::string signature = rs_->sign(attributes_);
                    attributes_["X-Amz-Signature"] = signature;

                    std::istream &stream = call_service();
                    return utility::CloudUtility::unmarshal_the_response(stream, logger_);
                }

                std::string DescribeInstances::get_formatted_timestamp() {
                    using namespace boost::posix_time;
                    ptime now = second_clock::universal_time();

                    std::ostringstream out;
                    std::locale timeLocale(out.getloc(), new time_facet(impl::Constants::DATE_FORMAT));
                    out.imbue(timeLocale);
                    out << now;
                    return out.str();
                }

                std::istream &DescribeInstances::call_service() {
                    std::string query = rs_->get_canonicalized_query_string(attributes_);
                    https_client_ = std::unique_ptr<util::SyncHttpsClient>(
                            new util::SyncHttpsClient(endpoint_.c_str(), QUERY_PREFIX + query));
                    return https_client_->open_connection();
                }

                void DescribeInstances::check_keys_from_iam_roles() {
                    if (aws_config_.get_access_key().empty() || !aws_config_.get_iam_role().empty()) {
                        try_get_default_iam_role();
                        if (!aws_config_.get_iam_role().empty()) {
                            get_keys_from_iam_role();
                        } else {
                            get_keys_from_iam_task_role();
                        }
                    }
                }

                void DescribeInstances::try_get_default_iam_role() {
                    // if none of the below are true
                    if (!(aws_config_.get_iam_role().empty() || aws_config_.get_iam_role() == "DEFAULT")) {
                        // stop here. No point looking up the default role.
                        return;
                    }
                    try {
                        util::SyncHttpClient httpClient(IAM_ROLE_ENDPOINT, IAM_ROLE_QUERY);
                        std::string roleName;
                        std::istream &responseStream = httpClient.open_connection();
                        responseStream >> roleName;
                        aws_config_.set_iam_role(roleName);
                    } catch (exception::io &e) {
                        BOOST_THROW_EXCEPTION(exception::invalid_configuration("tryGetDefaultIamRole",
                                                                                       std::string(
                                                                                               "Invalid Aws Configuration. ") +
                                                                                       e.what()));
                    }
                }

                void DescribeInstances::get_keys_from_iam_task_role() {
                    // before giving up, attempt to discover whether we're running in an ECS Container,
                    // in which case, AWS_CONTAINER_CREDENTIALS_RELATIVE_URI will exist as an env var.
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    #pragma warning(push)
#pragma warning(disable: 4996) //for 'getenv': This function or variable may be unsafe.
#endif
                    const char *uri = std::getenv(Constants::ECS_CREDENTIALS_ENV_VAR_NAME);
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
                    if (!uri) {
                        BOOST_THROW_EXCEPTION(exception::illegal_argument("getKeysFromIamTaskRole",
                                                                                  "Could not acquire credentials! Did not find declared AWS access key or IAM Role, and could not discover IAM Task Role or default role."));
                    }

                    util::SyncHttpClient httpClient(IAM_TASK_ROLE_ENDPOINT, uri);

                    try {
                        std::istream &istream = httpClient.open_connection();
                        parse_and_store_role_creds(istream);
                    } catch (exception::iexception &e) {
                        std::stringstream out;
                        out << "Unable to retrieve credentials from IAM Task Role. URI: " << uri << ". \n " << e.what();
                        BOOST_THROW_EXCEPTION(
                                exception::invalid_configuration("getKeysFromIamTaskRole", out.str()));
                    }
                }

                void DescribeInstances::get_keys_from_iam_role() {
                    std::string query = "/latest/meta-data/iam/security-credentials/" + aws_config_.get_iam_role();

                    util::SyncHttpClient httpClient(IAM_ROLE_ENDPOINT, query);

                    try {
                        std::istream &istream = httpClient.open_connection();
                        parse_and_store_role_creds(istream);
                    } catch (exception::iexception &e) {
                        std::stringstream out;
                        out << "Unable to retrieve credentials from IAM Task Role. URI: " << query << ". \n "
                            << e.what();
                        BOOST_THROW_EXCEPTION(
                                exception::invalid_configuration("getKeysFromIamRole", out.str()));
                    }
                }

                void DescribeInstances::parse_and_store_role_creds(std::istream &in) {
                    utility::CloudUtility::unmarshal_json_response(in, aws_config_, attributes_);
                }

                /**
                 * Add available filters to narrow down the scope of the query
                 */
                void DescribeInstances::add_filters() {
                    Filter filter;
                    if (!aws_config_.get_tag_key().empty()) {
                        if (!aws_config_.get_tag_value().empty()) {
                            filter.add_filter(std::string("tag:") + aws_config_.get_tag_key(), aws_config_.get_tag_value());
                        } else {
                            filter.add_filter("tag-key", aws_config_.get_tag_key());
                        }
                    } else if (!aws_config_.get_tag_value().empty()) {
                        filter.add_filter("tag-value", aws_config_.get_tag_value());
                    }

                    if (!aws_config_.get_security_group_name().empty()) {
                        filter.add_filter("instance.group-name", aws_config_.get_security_group_name());
                    }

                    filter.add_filter("instance-state-name", "running");
                    const std::unordered_map<std::string, std::string> &filters = filter.get_filters();
                    attributes_.insert(filters.begin(), filters.end());
                }

            }

            namespace utility {
                std::string AwsURLEncoder::url_encode(const std::string &value) {
                    std::string result = escape_encode(value);
                    boost::replace_all(result, "+", "%20");
                    return result;
                }

                std::string AwsURLEncoder::escape_encode(const std::string &value) {
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

                std::unordered_map<std::string, std::string> CloudUtility::unmarshal_the_response(std::istream &stream,
                                                                                      logger &lg) {
                    std::unordered_map<std::string, std::string> privatePublicPairs;

                    pt::ptree tree;
                    try {
                        pt::read_xml(stream, tree);
                    } catch (pt::xml_parser_error &e) {
                        HZ_LOG(lg, warning, 
                            boost::str(boost::format("The parsed xml stream has errors: %1%") % e.what()));
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
                                HZ_LOG(lg, finest, 
                                    boost::str(boost::format("Accepting EC2 instance [%1%][%2%]")
                                               % instanceItem.second.get_optional<std::string>("tagset.item.value").value_or("")
                                               % prIp)
                                );
                            }
                        }
                    }
                    return privatePublicPairs;
                }

                void CloudUtility::unmarshal_json_response(std::istream &stream, config::client_aws_config &aws_config,
                                                         std::unordered_map<std::string, std::string> &attributes) {
                    pt::ptree json;
                    pt::read_json(stream, json);
                    aws_config.set_access_key(json.get_optional<std::string>("AccessKeyId").get_value_or(""));
                    aws_config.set_secret_key(json.get_optional<std::string>("SecretAccessKey").get_value_or(""));
                    attributes["X-Amz-Security-Token"] = json.get_optional<std::string>("Token").get_value_or("");
                }

            }

            AWSClient::AWSClient(config::client_aws_config &aws_config, logger &lg) : aws_config_(aws_config),
                                                                                      logger_(lg) {
                this->endpoint_ = aws_config.get_host_header();
                if (!aws_config.get_region().empty() && aws_config.get_region().length() > 0) {
                    if (aws_config.get_host_header().find("ec2.") != 0) {
                        BOOST_THROW_EXCEPTION(exception::invalid_configuration("AWSClient::AWSClient",
                                                                                       "HostHeader should start with \"ec2.\" prefix"));
                    }
                    boost::replace_all(this->endpoint_, "ec2.", std::string("ec2.") + aws_config.get_region() + ".");
                }
            }

            std::unordered_map<std::string, std::string> AWSClient::get_addresses() {
                return impl::DescribeInstances(aws_config_, endpoint_, logger_).execute();
            }

        }
    }
}
