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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_NEARCACHE_KEYSTATEMARKER_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_NEARCACHE_KEYSTATEMARKER_H_

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    template <typename K>
                    class HAZELCAST_API KeyStateMarker {
                    public:
                        KeyStateMarker(int count) : markerCount(count) {
                        }

                        bool tryMark(const K &key) {
                            // TODO
                            return false;
                        }

                        bool tryUnmark(const K &key) {
                            //TODO
                            return false;
                        }

                        bool tryRemove(const K &key) {
                            //TODO
                            return false;
                        }

                        void forceUnmark(const K &key) {
                            // TODO
                        }

                        void init() {
                            // TODO
                        }

                        enum STATE {
                            UNMARKED,
                            MARKED,
                            REMOVED
                        };
                    private:
                        int markerCount;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_NEARCACHE_KEYSTATEMARKER_H_ */

