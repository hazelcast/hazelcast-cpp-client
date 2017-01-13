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
#include "hazelcast/client/internal/nearcache/impl/KeyStateMarkerImpl.h"
#include "hazelcast/util/HashUtil.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    KeyStateMarkerImpl::KeyStateMarkerImpl(int count) : markCount(count), marks(count) {
                    }

                    KeyStateMarkerImpl::~KeyStateMarkerImpl() {
                    }

                    bool KeyStateMarkerImpl::tryMark(const serialization::pimpl::Data &key) {
                        return casState(key, UNMARKED, MARKED);
                    }

                    bool KeyStateMarkerImpl::tryUnmark(const serialization::pimpl::Data &key) {
                        return casState(key, MARKED, UNMARKED);
                    }

                    bool KeyStateMarkerImpl::tryRemove(const serialization::pimpl::Data &key) {
                        return casState(key, MARKED, REMOVED);
                    }

                    void KeyStateMarkerImpl::forceUnmark(const serialization::pimpl::Data &key) {
                        int slot = getSlot(key);
                        marks[slot] = UNMARKED;
                    }

                    void KeyStateMarkerImpl::init() {
                        for (int i = 0; i < markCount; ++i) {
                            marks[i] = UNMARKED;
                        }
                    }

                    bool KeyStateMarkerImpl::casState(const serialization::pimpl::Data &key, STATE expect, STATE update) {
                        int slot = getSlot(key);
                        return marks[slot].compareAndSet(expect, update);
                    }

                    int KeyStateMarkerImpl::getSlot(const serialization::pimpl::Data &key) {
                        return util::HashUtil::hashToIndex(key.getPartitionHash(), markCount);
                    }
                }
            }
        }
    }
}

