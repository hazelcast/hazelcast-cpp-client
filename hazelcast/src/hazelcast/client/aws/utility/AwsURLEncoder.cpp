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
#include <string>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <regex>

#include "hazelcast/client/aws/utility/AwsURLEncoder.h"

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace utility {
                std::string AwsURLEncoder::urlEncode(const std::string &value) {
                    std::string result = escapeEncode(value);
                    std::regex re("\\+");
                    return std::regex_replace(result, re, "%20");
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
            }
        }
    }
}

