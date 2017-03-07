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

#include "hazelcast/client/config/SSLConfig.h"

namespace hazelcast {
    namespace client {
        namespace config {
            SSLConfig::SSLConfig() : enabled(false), sslProtocol(tlsv12) {
            }

            bool SSLConfig::isEnabled() const {
                return enabled;
            }

            SSLConfig &SSLConfig::setEnabled(bool enabled) {
                this->enabled = enabled;
                return *this;
            }

            SSLConfig &SSLConfig::setProtocol(SSLProtocol protocol) {
                this->sslProtocol = protocol;
                return *this;
            }

            SSLProtocol SSLConfig::getProtocol() const {
                return sslProtocol;
            }

            const std::string &SSLConfig::getCertificateAuthorityFilePath() const {
                return certificateAuthorityFilePath;
            }

            void SSLConfig::setCertificateAuthorityFilePath(const std::string &certificateAuthorityFilePath) {
                this->certificateAuthorityFilePath = certificateAuthorityFilePath;
            }
        }
    }
}
#endif // HZ_BUILD_WITH_SSL

