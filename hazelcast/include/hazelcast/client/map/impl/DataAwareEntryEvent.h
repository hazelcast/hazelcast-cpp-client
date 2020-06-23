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
#pragma once

#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            namespace impl {

                template<typename K, typename V>
                class DataAwareEntryEvent : public EntryEvent<K, V> {
                public:
                    DataAwareEntryEvent(const std::string &name, const Member &member, const EntryEventType &eventType,
                                        const std::shared_ptr<serialization::pimpl::Data> &dataKey,
                                        const std::shared_ptr<serialization::pimpl::Data> &dataNewValue,
                                        const std::shared_ptr<serialization::pimpl::Data> &dataOldValue,
                                        const std::shared_ptr<serialization::pimpl::Data> &dataMergingValue,
                                        serialization::pimpl::SerializationService &serializationService)
                            : EntryEvent<K, V>(name, member, eventType), keyData(dataKey), newValueData(dataNewValue),
                              oldValueData(dataOldValue), mergingValueData(dataMergingValue),
                              serializationService(serializationService) {
                        EntryEvent<K, V>::key = serializationService.toObject<K>(keyData.get());

                        EntryEvent<K, V>::value = serializationService.toObject<V>(newValueData.get());

                        EntryEvent<K, V>::oldValue = serializationService.toObject<V>(oldValueData.get());

                        EntryEvent<K, V>::mergingValue = serializationService.toObject<V>(mergingValueData.get());
                    }

                    const std::shared_ptr<serialization::pimpl::Data> &getKeyData() const {
                        return keyData;
                    }

                    const std::shared_ptr<serialization::pimpl::Data> &getNewValueData() const {
                        return newValueData;
                    }

                    const std::shared_ptr<serialization::pimpl::Data> &getOldValueData() const {
                        return oldValueData;
                    }

                    const std::shared_ptr<serialization::pimpl::Data> &getMergingValueData() const {
                        return mergingValueData;
                    }

                private:
                    const std::shared_ptr<serialization::pimpl::Data> keyData;

                    const std::shared_ptr<serialization::pimpl::Data> newValueData;

                    const std::shared_ptr<serialization::pimpl::Data> oldValueData;

                    const std::shared_ptr<serialization::pimpl::Data> mergingValueData;

                    serialization::pimpl::SerializationService &serializationService;
                };
            }
        }
    }
}



