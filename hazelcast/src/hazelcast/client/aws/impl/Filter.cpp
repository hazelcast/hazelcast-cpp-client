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
#include "hazelcast/client/aws/impl/Filter.h"

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
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
            }
        }
    }
}
