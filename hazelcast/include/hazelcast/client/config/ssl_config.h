/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

#ifdef HZ_BUILD_WITH_SSL
#include <boost/asio/ssl/context.hpp>
#endif

#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace socket {
class SocketFactory;
}
} // namespace internal
namespace config {
#ifdef HZ_BUILD_WITH_SSL
/** @deprecated Use ssl_config::set_context method instead. */
enum HAZELCAST_API ssl_protocol
{
    /// Generic SSL version 2.
    sslv2 = 0, // boost::asio::ssl::context_base::sslv2

    /// Generic SSL version 3.
    sslv3 = 3, // boost::asio::ssl::context_base::sslv3

    /// Generic TLS version 1.
    tlsv1 = 6, // boost::asio::ssl::context_base::tlsv1

    /// Generic SSL/TLS.
    sslv23 = 9, // boost::asio::ssl::context_base::sslv23

    /// Generic TLS version 1.1.
    tlsv11 = 12, // boost::asio::ssl::context_base::tlsv11,

    /// Generic TLS version 1.2.
    tlsv12 = 15, // boost::asio::ssl::context_base::tlsv12
};
#endif
/**
 * Contains configuration parameters for ssl related behaviour
 */
class HAZELCAST_API ssl_config
{
public:
    ssl_config();

#ifdef HZ_BUILD_WITH_SSL
    /**
     *
     * @param context The ssl context to be used.
     * @return the ssl_config object reference.
     */
    ssl_config& set_context(boost::asio::ssl::context context);

    /**
     * Returns if this configuration is enabled.
     *
     * @return true if enabled, false otherwise
     */
    bool is_enabled() const;

    /**
     * @deprecated Use ssl_config::set_context method instead.
     *
     * Enables and disables this configuration.
     *
     * @param isEnabled true to enable, false to disable
     */
    ssl_config& set_enabled(bool is_enabled);

    /**
     * @deprecated Use ssl_config::set_context method instead.
     *
     * Sets the ssl protocol to be used for this SSL socket.
     *
     * @param protocol One of the supported protocols
     */
    ssl_config& set_protocol(ssl_protocol protocol);

    /**
     * @deprecated Use ssl_config::get_context method instead.
     *
     * @return The configured SSL protocol
     */
    ssl_protocol get_protocol() const;

    /**
     * @deprecated Use ssl_config::set_context method instead.
     *
     * @return The list of all configured certificate verify files for the
     * client.
     */
    const std::vector<std::string>& get_verify_files() const;

    /**
     * @deprecated Use ssl_config::set_context method instead.
     *
     * This API calls the OpenSSL SSL_CTX_load_verify_locations method
     * underneath while starting the client with this configuration. The
     * validity of the files are checked only when the client starts. Hence,
     * this call will not do any error checking. Error checking is performed
     * only when the certificates are actually loaded during client start.
     *
     * @param filename the name of a file containing certification authority
     * certificates in PEM format.
     */
    ssl_config& add_verify_file(const std::string& filename);

    /**
     * @return Returns the use configured cipher list string.
     */
    const std::string& get_cipher_list() const;

    /**
     * @param ciphers The list of ciphers to be used. During client start, if
     * this API was set then the SSL_CTX_set_cipher_list
     * (https://www.openssl.org/docs/man1.0.2/ssl/SSL_set_cipher_list.html) is
     * called with the provided ciphers string. The values and the format of the
     * ciphers are described here:
     * https://www.openssl.org/docs/man1.0.2/apps/ciphers.html Some examples
     * values for the string are: "HIGH", "MEDIUM", "LOW", etc.
     *
     * If non of the provided ciphers could be selected the client
     * initialization will fail.
     *
     */
    ssl_config& set_cipher_list(const std::string& ciphers);

private:
    friend class internal::socket::SocketFactory;

    bool enabled_;
    ssl_protocol ssl_protocol_;
    std::vector<std::string> client_verify_files_;
    std::string cipher_list_;
    std::shared_ptr<boost::asio::ssl::context> ssl_context_;
    void check_context_enabled_already() const;
#endif
};
} // namespace config
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
