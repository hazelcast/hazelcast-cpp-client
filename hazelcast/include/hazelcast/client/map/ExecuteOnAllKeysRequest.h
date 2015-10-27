/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 8/13/13.




#ifndef HAZELCAST_ExecuteOnEntriesRequest
#define HAZELCAST_ExecuteOnEntriesRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            template<typename EntryProcessor>
            class ExecuteOnAllKeysRequest : public impl::ClientRequest {
            public:
                ExecuteOnAllKeysRequest(const std::string name, const EntryProcessor &entryProcessor)
                :name(name)
                , entryProcessor(entryProcessor) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::EXECUTE_ON_ALL_KEYS;
                }

                void write(serialization::PortableWriter &writer) const {
                    writer.writeUTF("n", &name);
                    serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                    out.writeObject<EntryProcessor>(&entryProcessor);
                }

            private:
                std::string name;
                const EntryProcessor &entryProcessor;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ExecuteOnEntriesRequest

