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

#include "hazelcast/client/serialization/pimpl/Data.h"

#include <stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API DataEntryView {
            public:
                DataEntryView(serialization::pimpl::Data &&key, serialization::pimpl::Data &&value,
                              int64_t cost, int64_t creation_time, int64_t expiration_time, int64_t hits,
                              int64_t last_access_time, int64_t last_stored_time, int64_t last_update_time, int64_t version,
                              int64_t ttl, int64_t max_idle);

                const serialization::pimpl::Data &get_key() const;

                const serialization::pimpl::Data &get_value() const;

                int64_t get_cost() const;

                int64_t get_creation_time() const;

                int64_t get_expiration_time() const;

                int64_t get_hits() const;

                int64_t get_last_access_time() const;

                int64_t get_last_stored_time() const;

                int64_t get_last_update_time() const;

                int64_t get_version() const;

                int64_t get_ttl() const;

                int64_t get_max_idle() const;

            private:
                serialization::pimpl::Data key_;
                serialization::pimpl::Data value_;
                int64_t cost_;
                int64_t creationTime_;
                int64_t expirationTime_;
                int64_t hits_;
                int64_t lastAccessTime_;
                int64_t lastStoredTime_;
                int64_t lastUpdateTime_;
                int64_t version_;
                int64_t ttl_;
                int64_t maxIdle_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


