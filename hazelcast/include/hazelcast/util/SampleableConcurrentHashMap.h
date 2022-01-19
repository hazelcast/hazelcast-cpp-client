/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/internal/eviction/Expirable.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/Iterator.h"
#include "hazelcast/util/Iterable.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace util {
/**
 * ConcurrentHashMap to extend iterator capability.
 *
 * @param <K> Type of the key
 * @param <V> Type of the value
 */
template<typename K, typename V, typename KS, typename VS>
class SampleableConcurrentHashMap
  : public SynchronizedMap<std::shared_ptr<KS>, VS>
{
public:
    SampleableConcurrentHashMap(int32_t initial_capacity) {}

    /**
     * Entry to define keys and values for sampling.
     */
    class SamplingEntry
    {
    public:
        SamplingEntry(const std::shared_ptr<KS> entry_key,
                      const std::shared_ptr<VS> entry_value)
          : key_(entry_key)
          , value_(entry_value)
        {}

        const std::shared_ptr<KS>& get_entry_key() const { return key_; }

        const std::shared_ptr<VS>& get_entry_value() const { return value_; }

        bool equals(const SamplingEntry& rhs)
        {
            return eq<KS>(key_, rhs.key_) && eq<VS>(value_, rhs.value_);
        }

        int32_t hash_code()
        {
            return (key_ == NULL ? 0 : key_->hashCode()) ^
                   (value_ == NULL ? 0 : value_->hashCode());
        }

        std::string to_string()
        {
            std::ostringstream out;
            if (NULL == key_.get()) {
                out << "null";
            } else {
                out << *key_;
            }
            out << "=";
            if (NULL == value_.get()) {
                out << "null";
            } else {
                out << *value_;
            }

            return out.str();
        }

    protected:
        const std::shared_ptr<KS> key_;
        const std::shared_ptr<VS> value_;

    private:
        template<typename T>
        static bool eq(const std::shared_ptr<T>& o1,
                       const std::shared_ptr<T>& o2)
        {
            return o1.get() == NULL ? (o2.get() == NULL) : (*o1 == *o2);
        }
    };

    /**
     * Fetches keys from given <code>tableIndex</code> as <code>size</code>
     * and puts them into <code>keys</code> list.
     *
     * @param tableIndex    Index (checkpoint) for starting point of fetch
     * operation
     * @param size          Count of how many keys will be fetched
     * @param keys          List that fetched keys will be put into
     *
     * @return the next index (checkpoint) for later fetches
     */
    /*
                int fetchKeys(int tableIndex, int size,
       std::vector<std::shared_ptr<K> > &keys) { const long now =
       Clock.currentTimeMillis(); final Segment<K, V> segment = segments[0];
                        final HashEntry<K, V>[] currentTable = segment.table;
                        int nextTableIndex;
                        if (tableIndex >= 0 && tableIndex <
       segment.table.length) { nextTableIndex = tableIndex; } else {
                            nextTableIndex = currentTable.length - 1;
                        }
                        int counter = 0;
                        while (nextTableIndex >= 0 && counter < size) {
                            HashEntry<K, V> nextEntry =
       currentTable[nextTableIndex--]; while (nextEntry != null) { if
       (nextEntry.key() != null) { final V value = nextEntry.value(); if
       (isValidForFetching(value, now)) { keys.add(nextEntry.key()); counter++;
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
     * @param tableIndex           Index (checkpoint) for starting point of
     * fetch operation
     * @param size                 Count of how many entries will be fetched
     * @param entries              List that fetched entries will be put into
     * @return the next index (checkpoint) for later fetches
     */
    /*
                int fetchEntries(int tableIndex, int size, List<Map.Entry<K, V>>
       entries) { final long now = Clock.currentTimeMillis(); final Segment<K,
       V> segment = segments[0]; final HashEntry<K, V>[] currentTable =
       segment.table; int nextTableIndex; if (tableIndex >= 0 && tableIndex <
       segment.table.length) { nextTableIndex = tableIndex; } else {
                            nextTableIndex = currentTable.length - 1;
                        }
                        int counter = 0;
                        while (nextTableIndex >= 0 && counter < size) {
                            HashEntry<K, V> nextEntry =
       currentTable[nextTableIndex--]; while (nextEntry != null) { if
       (nextEntry.key() != null) { final V value = nextEntry.value(); if
       (isValidForFetching(value, now)) { K key = nextEntry.key();
                                        entries.add(new
       AbstractMap.SimpleEntry<K, V>(key, value)); counter++;
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

    std::unique_ptr<util::Iterable<E>> get_random_samples(
      int sample_count) const
    {
        if (sample_count < 0) {
            BOOST_THROW_EXCEPTION(client::exception::illegal_argument(
              "Sample count cannot be a negative value."));
        }
        if (sample_count == 0 ||
            SynchronizedMap<std::shared_ptr<KS>, VS>::size() == 0) {
            return std::unique_ptr<util::Iterable<E>>();
        }

        return std::unique_ptr<util::Iterable<E>>(
          new LazySamplingEntryIterableIterator(sample_count, *this));
    }

protected:
    virtual bool is_valid_for_fetching(const std::shared_ptr<VS>& value,
                                       int64_t now) const
    {
        const std::shared_ptr<client::internal::eviction::Expirable>&
          expirable =
            std::dynamic_pointer_cast<client::internal::eviction::Expirable>(
              value);
        if (NULL != expirable.get()) {
            return !(expirable->is_expired_at(now));
        }
        return true;
    }

    virtual bool is_valid_for_sampling(const std::shared_ptr<VS>& value) const
    {
        return value.get() != NULL;
    }

    virtual std::shared_ptr<E> create_sampling_entry(
      std::shared_ptr<KS>& key,
      std::shared_ptr<VS>& value) const
    {
        return std::shared_ptr<E>(new SamplingEntry(key, value));
    }

private:
    /**
     * Starts at a random index and iterates through until all the samples can
     * be collected
     */
    class LazySamplingEntryIterableIterator
      : public util::Iterable<E>
      , public util::Iterator<E>
    {
    public:
        LazySamplingEntryIterableIterator(
          int max_count,
          const SampleableConcurrentHashMap<K, V, KS, VS>& sampleable_map)
          : max_entry_count_(max_count)
          , random_number_(std::abs(rand()))
          , returned_entry_count_(0)
          , current_index_(-1)
          , reached_to_end_(false)
          , internal_map_(sampleable_map)
        {
            size_t mapSize = internal_map_.size();
            start_index_ = (int)(random_number_ % mapSize);
            if (start_index_ < 0) {
                start_index_ = 0;
            }
        }

        util::Iterator<E>* iterator() override { return this; }

        void iterate()
        {
            if (returned_entry_count_ >= max_entry_count_ || reached_to_end_) {
                current_sample_.reset();
                return;
            }

            if (current_index_ == -1) {
                current_index_ = start_index_;
            }
            // If current entry is not initialized yet, initialize it
            if (current_entry_.get() == NULL) {
                current_entry_ =
                  internal_map_.get_entry((size_t)current_index_);
            }
            do {
                current_entry_ =
                  internal_map_.get_entry((size_t)current_index_);
                // Advance to next entry
                ++current_index_;
                if ((size_t)current_index_ >= internal_map_.size()) {
                    current_index_ = 0;
                }
                while (current_entry_.get() != NULL) {
                    std::shared_ptr<VS> value = current_entry_->second;
                    std::shared_ptr<KS> key = current_entry_->first;
                    current_entry_ =
                      internal_map_.get_entry((size_t)current_index_);
                    if (internal_map_.is_valid_for_sampling(value)) {
                        current_sample_ =
                          internal_map_.create_sampling_entry(key, value);
                        returned_entry_count_++;
                        return;
                    }
                }
            } while (current_index_ != start_index_);

            reached_to_end_ = true;
            current_sample_.reset();
        }

        bool has_next() override
        {
            iterate();
            return current_sample_.get() != NULL;
        }

        std::shared_ptr<E> next() override
        {
            if (current_sample_.get() != NULL) {
                return current_sample_;
            } else {
                BOOST_THROW_EXCEPTION(client::exception::no_such_element(
                  "No more elements in the iterated collection"));
            }
        }

        void remove() override
        {
            throw client::exception::unsupported_operation(
              "Removing is not supported");
        }

    private:
        const int max_entry_count_;
        const int random_number_;
        std::shared_ptr<std::pair<std::shared_ptr<KS>, std::shared_ptr<VS>>>
          current_entry_;
        int returned_entry_count_;
        int current_index_;
        bool reached_to_end_;
        std::shared_ptr<E> current_sample_;
        const SampleableConcurrentHashMap& internal_map_;
        int start_index_;
    };
};
} // namespace util
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
