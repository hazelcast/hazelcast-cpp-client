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


#include <boost/make_shared.hpp>

#include "hazelcast/client/serialization/json/HazelcastJsonValue.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace json {
                HazelcastJsonValue::HazelcastJsonValue() {
                }

                HazelcastJsonValue::HazelcastJsonValue(const std::string &jsonString) : jsonString(
                        boost::make_shared<std::string>(jsonString)) {
                }

                HazelcastJsonValue::HazelcastJsonValue(std::auto_ptr<std::string> jsonString) : jsonString(jsonString) {
                }

                HazelcastJsonValue::~HazelcastJsonValue() {
                }

                const boost::shared_ptr<std::string> HazelcastJsonValue::toJsonString() const {
                    return jsonString;
                }

                bool HazelcastJsonValue::operator==(const HazelcastJsonValue &rhs) const {

                    if (jsonString == rhs.jsonString) {
                        return true;
                    }

                    if (!jsonString) {
                        return false;
                    }

                    return *jsonString == *rhs.jsonString;
                }

                bool HazelcastJsonValue::operator!=(const HazelcastJsonValue &rhs) const {
                    return !(rhs == *this);
                }
            }
        }
    }
}
