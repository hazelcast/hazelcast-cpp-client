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
#ifndef HAZELCAST_CLIENT_TYPEDDATA_H_
#define HAZELCAST_CLIENT_TYPEDDATA_H_

#include <memory>

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class HAZELCAST_API TypedData {
        public:
            TypedData() : ss(NULL) {
            }

            TypedData(std::auto_ptr<serialization::pimpl::Data> data,
                      serialization::pimpl::SerializationService &serializationService) : data(data),
                                                                                          ss(&serializationService) {
            }

            TypedData(const TypedData &rhs) : ss(rhs.ss) {
                if (rhs.data.get()) {
                    data = std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*rhs.data));
                }
            }

            TypedData &operator=(const TypedData& rhs) {
                if (rhs.data.get()) {
                    data = std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*rhs.data));
                } else {
                    data.reset();
                }
                ss = rhs.ss;
                return *this;
            }

            virtual ~TypedData() { }

            const serialization::pimpl::ObjectType getType() const {
                return ss->getObjectType(data.get());
            }

            template <typename T>
            std::auto_ptr<T> get() {
                return ss->toObject<T>(data.get());
            }

            const serialization::pimpl::Data *getData() const {
                return data.get();
            }

        private:
            std::auto_ptr<serialization::pimpl::Data> data;
            serialization::pimpl::SerializationService *ss;
        };
    }
}

#endif //HAZELCAST_CLIENT_TYPEDDATA_H_

