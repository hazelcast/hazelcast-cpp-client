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

#include <stdint.h>
#include <atomic>

#include "hazelcast/client/map/impl/nearcache/KeyStateMarker.h"


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    class HAZELCAST_API KeyStateMarkerImpl : public map::impl::nearcache::KeyStateMarker {
                    public:
                        KeyStateMarkerImpl(int count);

                        ~KeyStateMarkerImpl() override;

                        bool tryMark(const serialization::pimpl::Data &key) override;

                        bool tryUnmark(const serialization::pimpl::Data &key) override;

                        bool tryRemove(const serialization::pimpl::Data &key) override;

                        void forceUnmark(const serialization::pimpl::Data &key) override;

                        void init() override;
                    private:
                        bool casState(const serialization::pimpl::Data &key, STATE expect, STATE update);

                        int getSlot(const serialization::pimpl::Data &key);

                        const int markCount;
                        std::atomic<int32_t> *marks;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



