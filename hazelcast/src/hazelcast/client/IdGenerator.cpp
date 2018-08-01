/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/IdGenerator.h"

namespace hazelcast {
    namespace client {
        bool IdGenerator::init(int64_t id) {
            return impl->init(id);
        }

        int64_t IdGenerator::newId() {
            return impl->newId();
        }

        IdGenerator::IdGenerator(const boost::shared_ptr<impl::IdGeneratorInterface> &impl) : impl(impl) {}

        IdGenerator::~IdGenerator() {
        }
    }
}
