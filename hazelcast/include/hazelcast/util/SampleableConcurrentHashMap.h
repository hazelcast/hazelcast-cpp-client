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

#ifndef HAZELCAST_UTIL_SAMPLEABLECONCURRENTHASHMAP_H_
#define HAZELCAST_UTIL_SAMPLEABLECONCURRENTHASHMAP_H_

#include "hazelcast/client/internal/eviction/Expirable.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/Iterator.h"
#include "hazelcast/util/Iterable.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace util {
        /**
         * ConcurrentHashMap to extend iterator capability.
         *
         * @param <K> Type of the key
         * @param <V> Type of the value
         */
        template<typename K, typename V, typename KS, typename VS, typename Comparator  = std::less<hazelcast::util::SharedPtr<KS> > >
        class SampleableConcurrentHashMap : public SynchronizedMap<hazelcast::util::SharedPtr<KS>, VS, Comparator> {
        public:
            SampleableConcurrentHashMap(int32_t initialCapacity) {
            }

            /**
             * Entry to define keys and values for sampling.
             */
            class SamplingEntry {
            public:
                SamplingEntry(const hazelcast::util::SharedPtr<KS> entryKey, const hazelcast::util::SharedPtr<VS> entryValue) : key(
                        entryKey), value(entryValue) {
                }

                const hazelcast::util::SharedPtr<KS> &getEntryKey() const {
                    return key;
                }

                const hazelcast::util::SharedPtr<VS> &getEntryValue() const {
                    return value;
                }

                bool equals(const SamplingEntry &rhs) {
                    return eq<KS>(key, rhs.key) && eq<VS>(value, rhs.value);
                }

                int32_t hashCode() {
                    return 0;
                }

                std::string toString() {
                    std::ostringstream out;
                    if (NULL == key.get()) {
                        out << "null";
                    } else {
                        out << *key;
                    }
                    out << "=";
                    if (NULL == value.get()) {
                        out << "null";
                    } else {
                        out << *value;
                    }

                    return out.str();
                }

            protected:
                const hazelcast::util::SharedPtr<KS> key;
                const hazelcast::util::SharedPtr<VS> value;
            private:
                template<typename T>
                static bool eq(const hazelcast::util::SharedPtr<T> &o1, const hazelcast::util::SharedPtr<T> &o2) {
                    return o1.get() == NULL ? (o2.get() == NULL) : (*o1 == *o2);
                }
            };

            /**
             * Fetches keys from given <code>tableIndex</code> as <code>size</code>
             * and puts them into <code>keys</code> list.
             *
             * @param tableIndex    Index (checkpoint) for starting point of fetch operation
             * @param size          Count of how many keys will be fetched
             * @param keys          List that fetched keys will be put into
             *
             * @return the next index (checkpoint) for later fetches
             */
/*
            int fetchKeys(int tableIndex, int size, std::vector<hazelcast::util::SharedPtr<K> > &keys) {
                    const long now = Clock.currentTimeMillis();
                    final Segment<K, V> segment = segments[0];
                    final HashEntry<K, V>[] currentTable = segment.table;
                    int nextTableIndex;
                    if (tableIndex >= 0 && tableIndex < segment.table.length) {
                        nextTableIndex = tableIndex;
                    } else {
                        nextTableIndex = currentTable.length - 1;
                    }
                    int counter = 0;
                    while (nextTableIndex >= 0 && counter < size) {
                        HashEntry<K, V> nextEntry = currentTable[nextTableIndex--];
                        while (nextEntry != null) {
                            if (nextEntry.key() != null) {
                                final V value = nextEntry.value();
                                if (isValidForFetching(value, now)) {
                                    keys.add(nextEntry.key());
                                    counter++;
                                }
                            }
                            nextEntry = nextEntry.next;
                        }
                    }
                    return nextTableIndex;
                }
*/

            /**
             * Fetches entries from given <code>tableIndex</code> as <code>size</code>
             * and puts them into <code>entries</code> list.
             *
             * @param tableIndex           Index (checkpoint) for starting point of fetch operation
             * @param size                 Count of how many entries will be fetched
             * @param entries              List that fetched entries will be put into
             * @return the next index (checkpoint) for later fetches
             */
/*
            int fetchEntries(int tableIndex, int size, List<Map.Entry<K, V>> entries) {
                    final long now = Clock.currentTimeMillis();
                    final Segment<K, V> segment = segments[0];
                    final HashEntry<K, V>[] currentTable = segment.table;
                    int nextTableIndex;
                    if (tableIndex >= 0 && tableIndex < segment.table.length) {
                        nextTableIndex = tableIndex;
                    } else {
                        nextTableIndex = currentTable.length - 1;
                    }
                    int counter = 0;
                    while (nextTableIndex >= 0 && counter < size) {
                        HashEntry<K, V> nextEntry = currentTable[nextTableIndex--];
                        while (nextEntry != null) {
                            if (nextEntry.key() != null) {
                                final V value = nextEntry.value();
                                if (isValidForFetching(value, now)) {
                                    K key = nextEntry.key();
                                    entries.add(new AbstractMap.SimpleEntry<K, V>(key, value));
                                    counter++;
                                }
                            }
                            nextEntry = nextEntry.next;
                        }
                    }
                    return nextTableIndex;
                }

*/
            /**
             * Gets and returns samples as <code>sampleCount</code>.
             *
             * @param sampleCount Count of samples
             * @return the sampled {@link SamplingEntry} list
             */
            typedef typename SampleableConcurrentHashMap<K, V, KS, VS>::SamplingEntry E;

            std::auto_ptr<util::Iterable<E> > getRandomSamples(int sampleCount) const {
                if (sampleCount < 0) {
                    throw client::exception::IllegalArgumentException("Sample count cannot be a negative value.");
                }
                if (sampleCount == 0 || SynchronizedMap<hazelcast::util::SharedPtr<KS>, VS>::size() == 0) {
                    return std::auto_ptr<util::Iterable<E> >();
                }

                return std::auto_ptr<util::Iterable<E> >(new LazySamplingEntryIterableIterator(sampleCount, *this));
            }

        protected:
            virtual bool isValidForFetching(const hazelcast::util::SharedPtr<VS> &value, int64_t now) const {
                const hazelcast::util::SharedPtr<client::internal::eviction::Expirable> &expirable =
                        value.template castShare<client::internal::eviction::Expirable>();
                if (NULL != expirable.get()) {
                    return !(expirable->isExpiredAt(now));
                }
                return true;
            }

            virtual bool isValidForSampling(const hazelcast::util::SharedPtr<VS> &value) const {
                return value.get() != NULL;
            }

            virtual hazelcast::util::SharedPtr<E> createSamplingEntry(hazelcast::util::SharedPtr<KS> &key,
                                                             hazelcast::util::SharedPtr<VS> &value) const {
                return hazelcast::util::SharedPtr<E>(new SamplingEntry(key, value));
            }
        private:
            /**
             * Starts at a random index and iterates through until all the samples can be collected
             */
            class LazySamplingEntryIterableIterator
                    : public util::Iterable<E>, public util::Iterator<E> {
            public:
                LazySamplingEntryIterableIterator(int maxCount, const SampleableConcurrentHashMap<K, V, KS, VS> &sampleableMap)
                        : maxEntryCount(maxCount), randomNumber(std::abs(rand())), returnedEntryCount(0),
                          currentIndex(-1),
                          reachedToEnd(false), internalMap(sampleableMap) {
                    size_t mapSize = internalMap.size();
                    startIndex = (int) (randomNumber % mapSize);
                    if (startIndex < 0) {
                        startIndex = 0;
                    }
                }

                //@Override
                util::Iterator<E> *iterator() {
                    return this;
                }

                void iterate() {
                    if (returnedEntryCount >= maxEntryCount || reachedToEnd) {
                        currentSample.reset();
                        return;
                    }

                    if (currentIndex == -1) {
                        currentIndex = startIndex;
                    }
                    // If current entry is not initialized yet, initialize it
                    if (currentEntry.get() == NULL) {
                        currentEntry = internalMap.getEntry((size_t) currentIndex);
                    }
                    do {
                        currentEntry = internalMap.getEntry((size_t) currentIndex);
                        // Advance to next entry
                        ++currentIndex;
                        if ((size_t) currentIndex >= internalMap.size()) {
                            currentIndex = 0;
                        }
                        while (currentEntry.get() != NULL) {
                            hazelcast::util::SharedPtr<VS> value = currentEntry->second;
                            hazelcast::util::SharedPtr<KS> key = currentEntry->first;
                            currentEntry = internalMap.getEntry((size_t) currentIndex);
                            if (internalMap.isValidForSampling(value)) {
                                currentSample = internalMap.createSamplingEntry(key, value);
                                returnedEntryCount++;
                                return;
                            }
                        }
                    } while (currentIndex != startIndex);

                    reachedToEnd = true;
                    currentSample.reset();
                }

                //@Override
                bool hasNext() {
                    iterate();
                    return currentSample.get() != NULL;
                }

                //@Override
                hazelcast::util::SharedPtr<E> next() {
                    if (currentSample.get() != NULL) {
                        return currentSample;
                    } else {
                        throw client::exception::NoSuchElementException("No more elements in the iterated collection");
                    }
                }

                //@Override
                void remove() {
                    throw client::exception::UnsupportedOperationException("Removing is not supported");
                }

            private:
                const int maxEntryCount;
                const int randomNumber;
                hazelcast::util::SharedPtr<std::pair<hazelcast::util::SharedPtr<KS>, hazelcast::util::SharedPtr<VS> > > currentEntry;
                int returnedEntryCount;
                int currentIndex;
                bool reachedToEnd;
                hazelcast::util::SharedPtr<E> currentSample;
                const SampleableConcurrentHashMap &internalMap;
                int startIndex;
            };
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_SAMPLEABLECONCURRENTHASHMAP_H_

