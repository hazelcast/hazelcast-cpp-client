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

#include "hazelcast/client/query/Predicates.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace query {
            constexpr const char *QueryConstants::KEY_ATTRIBUTE_NAME;
            constexpr const char *QueryConstants::THIS_ATTRIBUTE_NAME;

            BasePredicate::BasePredicate(HazelcastClient &client) : outStream(spi::ClientContext(
                    client).getSerializationService().newOutputStream()) {}

            NamedPredicate::NamedPredicate(HazelcastClient &client, const std::string &attributeName) : BasePredicate(
                    client) {
                outStream.write(attributeName);
            }

            InstanceOfPredicate::InstanceOfPredicate(HazelcastClient &client, const std::string &javaClassName)
                    : BasePredicate(client) {
                outStream.write(javaClassName);
            }

            SqlPredicate::SqlPredicate(HazelcastClient &client, const std::string &sql)
                    : BasePredicate(client) {
                outStream.write(sql);
            }

            LikePredicate::LikePredicate(HazelcastClient &client, const std::string &attribute,
                                         const std::string &expression) : NamedPredicate(client, attribute) {
                outStream.write(expression);
            }

            ILikePredicate::ILikePredicate(HazelcastClient &client, const std::string &attribute,
                                           const std::string &expression) : NamedPredicate(client, attribute) {
                outStream.write(expression);
            }

            RegexPredicate::RegexPredicate(HazelcastClient &client, const std::string &attribute,
                                           const std::string &expression) : NamedPredicate(client, attribute) {
                outStream.write(expression);
            }

            TruePredicate::TruePredicate(HazelcastClient &client) : BasePredicate(client) {}

            FalsePredicate::FalsePredicate(HazelcastClient &client) : BasePredicate(client) {}
        }
    }
}

