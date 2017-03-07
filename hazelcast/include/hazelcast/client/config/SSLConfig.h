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
#ifndef HAZELCAST_CLIENT_CONFIG_SSLCONFIG_H_
#define HAZELCAST_CLIENT_CONFIG_SSLCONFIG_H_

#ifdef HZ_BUILD_WITH_SSL

#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <WinSock2.h>
#endif

#include <asio/ssl/context_base.hpp>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            enum HAZELCAST_API SSLProtocol
            {
                /// Generic SSL version 2.
                        sslv2 = asio::ssl::context_base::sslv2,

                /// Generic SSL version 3.
                        sslv3 = asio::ssl::context_base::sslv3,

                /// Generic TLS version 1.
                        tlsv1 = asio::ssl::context_base::tlsv1,

                /// Generic SSL/TLS.
                        sslv23 = asio::ssl::context_base::sslv23,

                /// Generic TLS version 1.1.
                        tlsv11 = asio::ssl::context_base::tlsv11,

                /// Generic TLS version 1.2.
                        tlsv12 = asio::ssl::context_base::tlsv12
            };

            /**
             * Contains configuration parameters for client network related behaviour
             */
            class HAZELCAST_API SSLConfig {
            public:
                /**
                 * Default protocol is tlsv12 and ssl is disabled by default
                 */
                SSLConfig();

                /**
                 * Returns if this configuration is enabled.
                 *
                 * @return true if enabled, false otherwise
                 */
                bool isEnabled() const;

                /**
                 * Enables and disables this configuration.
                 *
                 * @param enabled true to enable, false to disable
                 */
                SSLConfig &setEnabled(bool enabled);

                /**
                 * Sets the ssl protocol to be used for this SSL socket.
                 *
                 * @param protocol One of the supported protocols
                 */
                SSLConfig &setProtocol(SSLProtocol protocol);

                /**
                 * Default protocol is tlsv1_client
                 * @return The configured SSL protocol
                 */
                SSLProtocol getProtocol() const;

                /**
                 * @return The path of the valid CA file path. This file is being validated.
                 */
                const std::string &getCertificateAuthorityFilePath() const;

                /**
                 * @param certificateAuthorityFilePath The file path for the certificate authority
                 */
                void setCertificateAuthorityFilePath(const std::string &certificateAuthorityFilePath);
            private:
                bool enabled;
                SSLProtocol sslProtocol;
                std::string certificateAuthorityFilePath;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HZ_BUILD_WITH_SSL

#endif /* HAZELCAST_CLIENT_CONFIG_SSLCONFIG_H_ */
