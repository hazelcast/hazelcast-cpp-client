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

#include "hazelcast/util/hazelcast_dll.h"

#include "hazelcast/client/internal/nearcache/NearCacheRecord.h"
#include <boost/uuid/uuid.hpp>

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
                            AbstractNearCacheRecord(const std::shared_ptr<V> &v, int64_t create_time,
                                                    int64_t expiry_time)
                                    : value_(v), creation_time_(create_time), sequence_(0), expiration_time_(expiry_time),
                                      access_time_(NearCacheRecord<V>::TIME_NOT_SET), access_hit_(0) {
                            }

                            std::shared_ptr<V> get_value() const override {
                                return value_;
                            }

                            void set_value(const std::shared_ptr<V> &value) override {
                                AbstractNearCacheRecord::value_ = value;
                            }

                            int64_t get_creation_time() const override {
                                return creation_time_;
                            }

                            void set_creation_time(int64_t creation_time) override {
                                AbstractNearCacheRecord::creation_time_ = creation_time;
                            }

                            boost::uuids::uuid get_uuid() const {
                                return uuid_;
                            }

                            void set_uuid(boost::uuids::uuid uuid) override {
                                AbstractNearCacheRecord::uuid_ = uuid;
                            }

                            int64_t get_expiration_time() const override {
                                return expiration_time_;
                            }

                            void set_expiration_time(int64_t expiration_time) override {
                                AbstractNearCacheRecord::expiration_time_ = expiration_time;
                            }

                            int64_t get_last_access_time() override {
                                return access_time_;
                            }

                            void set_access_time(int64_t access_time) override {
                                AbstractNearCacheRecord::access_time_ = access_time;
                            }

                            int32_t get_access_hit() override {
                                return access_hit_;
                            }

                            void set_access_hit(int32_t access_hit) override {
                                AbstractNearCacheRecord::access_hit_ = access_hit;
                            }

                            bool is_expired_at(int64_t now) const override {
                                int64_t expiration = expiration_time_;
                                return (expiration > NearCacheRecord<V>::TIME_NOT_SET) && (expiration <= now);
                            }

                            void increment_access_hit() override {
                                ++access_hit_;
                            }

                            void reset_access_hit() override {
                                access_hit_ = 0;
                            }

                            bool is_idle_at(int64_t max_idle_milli_seconds, int64_t now) override {
                                if (max_idle_milli_seconds > 0) {
                                    if (access_time_ > NearCacheRecord<V>::TIME_NOT_SET) {
                                        return access_time_ + max_idle_milli_seconds < now;
                                    } else {
                                        return creation_time_ + max_idle_milli_seconds < now;
                                    }
                                } else {
                                    return false;
                                }
                            }

                            int64_t get_invalidation_sequence() const override {
                                return sequence_;
                            }

                            void set_invalidation_sequence(int64_t seq) override {
                                this->sequence_ = seq;
                            }

                            bool has_same_uuid(boost::uuids::uuid that_uuid) const override {
                                return uuid_ == that_uuid;
                            }

                        protected:
                            std::shared_ptr<V> value_;
                            int64_t creation_time_;
                            int64_t sequence_;
                            boost::uuids::uuid uuid_;

                            std::atomic<int64_t> expiration_time_;
                            std::atomic<int64_t> access_time_;
                            std::atomic<int32_t> access_hit_;
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



