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

#include "hazelcast/client/internal/eviction/Evictable.h"
#include "hazelcast/client/internal/nearcache/impl/SampleableNearCacheRecordMap.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/SampleableConcurrentHashMap.h"
#include "hazelcast/client/internal/eviction/EvictionCandidate.h"
#include "hazelcast/client/internal/eviction/EvictionListener.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace store {
                        /**
                         * @param R A type that extends NearCacheRecord
                         */
                        template<typename K, typename V, typename KS, typename R>
                        class HeapNearCacheRecordMap
                                : public util::SampleableConcurrentHashMap<K, V, KS, R>,
                                  public SampleableNearCacheRecordMap<K, V, KS, R> {
                        public:
                            typedef eviction::EvictionCandidate<K, V, KS, R> C;

                            HeapNearCacheRecordMap(serialization::pimpl::SerializationService &ss,
                                                   int32_t initialCapacity)
                                    : util::SampleableConcurrentHashMap<K, V, KS, R>(initialCapacity),
                                      serializationService_(ss) {
                            }

                            ~HeapNearCacheRecordMap() override = default;

                            class NearCacheEvictableSamplingEntry
                                    : public util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry,
                                      public C {
                            public:
                                NearCacheEvictableSamplingEntry(const std::shared_ptr<KS> &key,
                                                                const std::shared_ptr<R> &value,
                                                                serialization::pimpl::SerializationService &ss)
                                        : util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry(key,
                                                                                                        value),
                                          serializationService_(ss) {
                                }

                                ~NearCacheEvictableSamplingEntry() override = default;

                                std::shared_ptr<KS> get_accessor() const override {
                                    return util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::key_;
                                }

                                std::shared_ptr<R> get_evictable() const override {
                                    return util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::value_;
                                }

                                std::shared_ptr<K> get_key() const override {
                                    return serializationService_.to_shared_object<K>(
                                            util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::key_);
                                }

                                std::shared_ptr<V> get_value() const override {
                                    return std::shared_ptr<V>(serializationService_.to_shared_object<V>(
                                            util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::value_->get_value()));
                                }

                                int64_t get_creation_time() const override {
                                    return util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::value_->get_creation_time();
                                }

                                int64_t get_last_access_time() const override {
                                    return util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::value_->get_last_access_time();
                                }

                                int64_t get_access_hit() const override {
                                    return util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::value_->get_access_hit();
                                }

                            private:
                                serialization::pimpl::SerializationService &serializationService_;
                            };

                            int evict(std::vector<std::shared_ptr<C> > *evictionCandidates,
                                      eviction::EvictionListener<KS, R> *evictionListener) override {
                                if (evictionCandidates == NULL) {
                                    return 0;
                                }
                                int actualEvictedCount = 0;
                                for (typename std::vector<std::shared_ptr<C> >::const_iterator it = evictionCandidates->begin();
                                     it != evictionCandidates->end(); ++it) {
                                    const std::shared_ptr<C> &evictionCandidate = *it;
                                    if (util::SynchronizedMap<std::shared_ptr<KS>, R>::remove(
                                            evictionCandidate->get_accessor()).get() != NULL) {
                                        actualEvictedCount++;
                                        if (evictionListener != NULL) {
                                            evictionListener->on_evict(evictionCandidate->get_accessor(),
                                                                      evictionCandidate->get_evictable(), false);
                                        }
                                    }
                                }
                                return actualEvictedCount;
                            }

                            std::unique_ptr<util::Iterable<eviction::EvictionCandidate<K, V, KS, R> > > sample(
                                    int32_t sampleCount) const {
                                std::unique_ptr<util::Iterable<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> > samples = util::SampleableConcurrentHashMap<K, V, KS, R>::get_random_samples(
                                        sampleCount);
                                if (NULL == samples.get()) {
                                    return std::unique_ptr<util::Iterable<eviction::EvictionCandidate<K, V, KS, R> > >();
                                }
                                return std::unique_ptr<util::Iterable<eviction::EvictionCandidate<K, V, KS, R> > >(
                                        new EvictionCandidateAdapter(samples));
                            }

                            class EvictionCandidateAdapter
                                    : public util::Iterable<eviction::EvictionCandidate<K, V, KS, R> > {
                            public:
                                EvictionCandidateAdapter(
                                        std::unique_ptr<util::Iterable<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> > &samplesIterable)
                                        : adaptedIterable_(std::move(samplesIterable)) {
                                    adaptedIterator_ = std::unique_ptr<util::Iterator<eviction::EvictionCandidate<K, V, KS, R> > >(
                                            new EvictionCandidateIterator(*adaptedIterable_->iterator()));
                                }

                                class EvictionCandidateIterator
                                        : public util::Iterator<eviction::EvictionCandidate<K, V, KS, R> > {
                                public:
                                    EvictionCandidateIterator(
                                            util::Iterator<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> &adaptedIterator)
                                            : it_(adaptedIterator) {
                                    }

                                    bool has_next() override {
                                        return it_.has_next();
                                    }

                                    std::shared_ptr<eviction::EvictionCandidate<K, V, KS, R> > next() override {
                                        std::shared_ptr<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> obj = it_.next();
                                        std::shared_ptr<NearCacheEvictableSamplingEntry> heapObj = std::static_pointer_cast<NearCacheEvictableSamplingEntry>(
                                                obj);
                                        return std::static_pointer_cast<eviction::EvictionCandidate<K, V, KS, R> >(
                                                heapObj);
                                    }

                                    void remove() override {
                                        it_.remove();
                                    }

                                private:
                                    util::Iterator<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> &it_;
                                };

                                util::Iterator<eviction::EvictionCandidate<K, V, KS, R> > *iterator() override {
                                    return adaptedIterator_.get();
                                }

                            private:
                                std::unique_ptr<util::Iterable<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> > adaptedIterable_;
                                std::unique_ptr<util::Iterator<eviction::EvictionCandidate<K, V, KS, R> > > adaptedIterator_;
                            };

                        protected:
                            std::shared_ptr<typename util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry> create_sampling_entry(
                                    std::shared_ptr<KS> &key,
                                    std::shared_ptr<R> &value) const override {
                                return std::shared_ptr<typename util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry>(
                                        new NearCacheEvictableSamplingEntry(key, value, serializationService_));
                            }

                        private:
                            serialization::pimpl::SerializationService &serializationService_;
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



