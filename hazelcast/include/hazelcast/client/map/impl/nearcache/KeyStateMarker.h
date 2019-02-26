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
#ifndef HAZELCAST_CLIENT_MAP_IMPL_NEARCACHE_KEYSTATEMARKER_H_
#define HAZELCAST_CLIENT_MAP_IMPL_NEARCACHE_KEYSTATEMARKER_H_

#include <memory>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace map {
            namespace impl {
                namespace nearcache {
                    /**
                     * Used to assign a {@link STATE} to a key.
                     *
                     * That {@link STATE} is used when deciding whether or not a key can be puttable to a Near Cache.
                     * Because there is a possibility that an invalidation for a key can be received before putting that
                     * key into Near Cache, in that case, key should not be put into Near Cache.
                     */
                    class HAZELCAST_API KeyStateMarker {
                    public:
                        virtual ~KeyStateMarker() {}

                        virtual bool tryMark(const serialization::pimpl::Data &key) = 0;

                        virtual bool tryUnmark(const serialization::pimpl::Data &key) = 0;

                        virtual bool tryRemove(const serialization::pimpl::Data &key) = 0;

                        virtual void forceUnmark(const serialization::pimpl::Data &key) = 0;

                        virtual void init() = 0;

                        enum STATE {
                            UNMARKED,
                            MARKED,
                            REMOVED
                        };

                        static const std::auto_ptr<KeyStateMarker> TRUE_MARKER;
                    };

                    class HAZELCAST_API TrueMarkerImpl : public KeyStateMarker {
                    public:
                        bool tryMark(const serialization::pimpl::Data &key);

                        bool tryUnmark(const serialization::pimpl::Data &key);

                        bool tryRemove(const serialization::pimpl::Data &key);

                        void forceUnmark(const serialization::pimpl::Data &key);

                        void init();
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MAP_IMPL_NEARCACHE_KEYSTATEMARKER_H_ */

