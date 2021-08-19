/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/query/predicates.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace query {
            const char * const query_constants::KEY_ATTRIBUTE_NAME = "__key";
            const char * const query_constants::THIS_ATTRIBUTE_NAME = "this";

            base_predicate::base_predicate(hazelcast_client &client) : out_stream(spi::ClientContext(
                    client).get_serialization_service().new_output_stream()) {}

            named_predicate::named_predicate(hazelcast_client &client, const std::string &attribute_name) : base_predicate(
                    client) {
                out_stream.write(attribute_name);
            }

            instance_of_predicate::instance_of_predicate(hazelcast_client &client, const std::string &java_class_name)
                    : base_predicate(client) {
                out_stream.write(java_class_name);
            }

            sql_predicate::sql_predicate(hazelcast_client &client, const std::string &sql)
                    : base_predicate(client) {
                out_stream.write(sql);
            }

            like_predicate::like_predicate(hazelcast_client &client, const std::string &attribute,
                                           const std::string &expression) : named_predicate(client, attribute) {
                out_stream.write(expression);
            }

            ilike_predicate::ilike_predicate(hazelcast_client &client, const std::string &attribute,
                                             const std::string &expression) : named_predicate(client, attribute) {
                out_stream.write(expression);
            }

            regex_predicate::regex_predicate(hazelcast_client &client, const std::string &attribute,
                                             const std::string &expression) : named_predicate(client, attribute) {
                out_stream.write(expression);
            }

            true_predicate::true_predicate(hazelcast_client &client) : base_predicate(client) {}

            false_predicate::false_predicate(hazelcast_client &client) : base_predicate(client) {}
        }
    }
}

