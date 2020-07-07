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
#include <memory>
#include <atomic>

#include "hazelcast/util/HazelcastDll.h"

#include "hazelcast/client/internal/nearcache/NearCacheRecord.h"
#include "hazelcast/util/UUID.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace record {
                        /**
                         * Abstract implementation of {@link NearCacheRecord} with value and
                         * expiration time as internal state.
                         *
                         * @param <V> the type of the value stored by this {@link AbstractNearCacheRecord}
                         */
                        template<typename V>
                        class AbstractNearCacheRecord : public NearCacheRecord<V> {
                        public:
                            typedef V RECORD_TYPE;
                            AbstractNearCacheRecord(const std::shared_ptr<V> &v, int64_t createTime,
                                                    int64_t expiryTime)
                                    : value(v), creationTime(createTime), sequence(0), expirationTime(expiryTime),
                                      accessTime(NearCacheRecord<V>::TIME_NOT_SET), accessHit(0) {
                            }

                            std::shared_ptr<V> getValue() const override {
                                return value;
                            }

                            void setValue(const std::shared_ptr<V> &value) override {
                                AbstractNearCacheRecord::value = value;
                            }

                            int64_t getCreationTime() const override {
                                return creationTime;
                            }

                            void setCreationTime(int64_t creationTime) override {
                                AbstractNearCacheRecord::creationTime = creationTime;
                            }

                            const std::shared_ptr<util::UUID> &getUuid() const {
                                return uuid;
                            }

                            void setUuid(const std::shared_ptr<util::UUID> &uuid) override {
                                AbstractNearCacheRecord::uuid = uuid;
                            }

                            int64_t getExpirationTime() override {
                                return expirationTime;
                            }

                            void setExpirationTime(int64_t expirationTime) override {
                                AbstractNearCacheRecord::expirationTime = expirationTime;
                            }

                            int64_t getLastAccessTime() override {
                                return accessTime;
                            }

                            void setAccessTime(int64_t accessTime) override {
                                AbstractNearCacheRecord::accessTime = accessTime;
                            }

                            int32_t getAccessHit() override {
                                return accessHit;
                            }

                            void setAccessHit(int32_t accessHit) override {
                                AbstractNearCacheRecord::accessHit = accessHit;
                            }

                            bool isExpiredAt(int64_t now) override {
                                int64_t expiration = expirationTime;
                                return (expiration > NearCacheRecord<V>::TIME_NOT_SET) && (expiration <= now);
                            }

                            void incrementAccessHit() override {
                                ++accessHit;
                            }

                            void resetAccessHit() override {
                                accessHit = 0;
                            }

                            bool isIdleAt(int64_t maxIdleMilliSeconds, int64_t now) override {
                                if (maxIdleMilliSeconds > 0) {
                                    if (accessTime > NearCacheRecord<V>::TIME_NOT_SET) {
                                        return accessTime + maxIdleMilliSeconds < now;
                                    } else {
                                        return creationTime + maxIdleMilliSeconds < now;
                                    }
                                } else {
                                    return false;
                                }
                            }

                            int64_t getInvalidationSequence() const override {
                                return sequence;
                            }

                            void setInvalidationSequence(int64_t seq) override {
                                this->sequence = seq;
                            }

                            bool hasSameUuid(const std::shared_ptr<util::UUID> &thatUuid) const override {
                                if (uuid.get() == NULL || thatUuid.get() == NULL) {
                                    return false;
                                }
                                return uuid->equals(*thatUuid);
                            }

                        protected:
                            std::shared_ptr<V> value;
                            int64_t creationTime;
                            int64_t sequence;
                            std::shared_ptr<util::UUID> uuid;

                            std::atomic<int64_t> expirationTime;
                            std::atomic<int64_t> accessTime;
                            std::atomic<int32_t> accessHit;
                        };
                    }
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



