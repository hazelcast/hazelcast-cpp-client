/*
 * Copyright (c) 2008-2016, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_HEAPNEARCACHERECORDMAP_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_HEAPNEARCACHERECORDMAP_H_

#include <stdint.h>

#include "hazelcast/client/internal/eviction/Evictable.h"
#include "hazelcast/client/internal/nearcache/impl/SampleableNearCacheRecordMap.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
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
                                      serializationService(ss) {
                            }

                            virtual ~HeapNearCacheRecordMap() {
                            }

                            class NearCacheEvictableSamplingEntry
                                    : public util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry,
                                      public C {
                            public:
                                NearCacheEvictableSamplingEntry(const boost::shared_ptr<KS> &key,
                                                                const boost::shared_ptr<R> &value,
                                                                serialization::pimpl::SerializationService &ss)
                                        : util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry(key,
                                                                                                        value),
                                          serializationService(ss) {
                                }

                                virtual ~NearCacheEvictableSamplingEntry() {
                                }

                                //@Override
                                boost::shared_ptr<KS> getAccessor() const {
                                    return util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::key;
                                }

                                //@Override
                                boost::shared_ptr<R> getEvictable() const {
                                    return util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::value;
                                }

                                //@Override
                                boost::shared_ptr<K> getKey() const {
                                    return boost::shared_ptr<K>(serializationService.toSharedObject<K>(
                                            util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::key));
                                }

                                //@Override
                                boost::shared_ptr<V> getValue() const {
                                    return boost::shared_ptr<V>(serializationService.toSharedObject<V>(
                                            util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::value->getValue()));
                                }

                                //@Override
                                int64_t getCreationTime() const {
                                    return util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::value->getCreationTime();
                                }

                                //@Override
                                int64_t getLastAccessTime() const {
                                    return util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::value->getLastAccessTime();
                                }

                                //@Override
                                int64_t getAccessHit() const {
                                    return util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry::value->getAccessHit();
                                }

                            private:
                                serialization::pimpl::SerializationService &serializationService;
                            };

                            //@Override
                            int evict(std::vector<boost::shared_ptr<C> > *evictionCandidates,
                                      eviction::EvictionListener<KS, R> *evictionListener) {
                                if (evictionCandidates == NULL) {
                                    return 0;
                                }
                                int actualEvictedCount = 0;
                                for (typename std::vector<boost::shared_ptr<C> >::const_iterator it = evictionCandidates->begin();
                                     it != evictionCandidates->end(); ++it) {
                                    const boost::shared_ptr<C> &evictionCandidate = *it;
                                    if (util::SynchronizedMap<boost::shared_ptr<KS>, R>::remove(
                                            evictionCandidate->getAccessor()).get() != NULL) {
                                        actualEvictedCount++;
                                        if (evictionListener != NULL) {
                                            evictionListener->onEvict(evictionCandidate->getAccessor(),
                                                                      evictionCandidate->getEvictable(), false);
                                        }
                                    }
                                }
                                return actualEvictedCount;
                            }

                            //@Override
                            std::auto_ptr<util::Iterable<eviction::EvictionCandidate<K, V, KS, R> > > sample(
                                    int32_t sampleCount) const {
                                std::auto_ptr<util::Iterable<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> > samples = util::SampleableConcurrentHashMap<K, V, KS, R>::getRandomSamples(
                                        sampleCount);
                                if (NULL == samples.get()) {
                                    return std::auto_ptr<util::Iterable<eviction::EvictionCandidate<K, V, KS, R> > >();
                                }
                                return std::auto_ptr<util::Iterable<eviction::EvictionCandidate<K, V, KS, R> > >(
                                        new EvictionCandidateAdapter(samples));
                            }

                            class EvictionCandidateAdapter
                                    : public util::Iterable<eviction::EvictionCandidate<K, V, KS, R> > {
                            public:
                                EvictionCandidateAdapter(
                                        std::auto_ptr<util::Iterable<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> > &samplesIterable)
                                        : adaptedIterable(samplesIterable) {
                                    adaptedIterator = std::auto_ptr<util::Iterator<eviction::EvictionCandidate<K, V, KS, R> > >(
                                            new EvictionCandidateIterator(*adaptedIterable->iterator()));
                                }

                                class EvictionCandidateIterator
                                        : public util::Iterator<eviction::EvictionCandidate<K, V, KS, R> > {
                                public:
                                    EvictionCandidateIterator(
                                            util::Iterator<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> &adaptedIterator)
                                            : it(adaptedIterator) {
                                    }

                                    bool hasNext() {
                                        return it.hasNext();
                                    }

                                    virtual boost::shared_ptr<eviction::EvictionCandidate<K, V, KS, R> > next() {
                                        boost::shared_ptr<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> obj = it.next();
                                        boost::shared_ptr<NearCacheEvictableSamplingEntry> heapObj = boost::static_pointer_cast<NearCacheEvictableSamplingEntry>(
                                                obj);
                                        return boost::static_pointer_cast<eviction::EvictionCandidate<K, V, KS, R> >(
                                                heapObj);
                                    }

                                    virtual void remove() {
                                        it.remove();
                                    }

                                private:
                                    util::Iterator<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> &it;
                                };

                                //@override
                                util::Iterator<eviction::EvictionCandidate<K, V, KS, R> > *iterator() {
                                    return adaptedIterator.get();
                                }

                            private:
                                std::auto_ptr<util::Iterable<typename util::SampleableConcurrentHashMap<K, V, KS, R>::E> > adaptedIterable;
                                std::auto_ptr<util::Iterator<eviction::EvictionCandidate<K, V, KS, R> > > adaptedIterator;
                            };

                        protected:
                            //@Override
                            boost::shared_ptr<typename util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry> createSamplingEntry(
                                    boost::shared_ptr<KS> &key,
                                    boost::shared_ptr<R> &value) const {
                                return boost::shared_ptr<typename util::SampleableConcurrentHashMap<K, V, KS, R>::SamplingEntry>(
                                        new NearCacheEvictableSamplingEntry(key, value, serializationService));
                            }

                        private:
                            serialization::pimpl::SerializationService &serializationService;
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_HEAPNEARCACHERECORDMAP_H_ */

