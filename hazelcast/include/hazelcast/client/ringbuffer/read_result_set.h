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

#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace rb {
            class HAZELCAST_API read_result_set {
            public:
                /**
                 * Value returned from methods returning a sequence number when the
                 * information is not available (e.g. because of rolling upgrade and some
                 * members not returning the sequence).
                 */
                static const int64_t SEQUENCE_UNAVAILABLE = -1;

                read_result_set(int32_t read_count, std::vector<serialization::pimpl::data> &&data_items,
                                serialization::pimpl::SerializationService &serialization_service,
                                boost::optional<std::vector<int64_t>> &item_seqs, int64_t next_seq)
                              : items_read_count_(read_count), item_seqs_(std::move(item_seqs)), next_seq_(next_seq) {
                    for (auto &&item : data_items) {
                        items_.emplace_back(item, serialization_service);
                    }
                }

                /**
                 * Returns the number of items that have been read before filtering.
                 * <p>
                 * If no filter is set, then the {@code readCount} will be equal to
                 * {@link #size}.
                 * But if a filter is applied, it could be that items are read, but are
                 * filtered out. So if you are trying to make another read based on the
                 * {@link read_result_set} then you should increment the sequence by
                 * {@code readCount} and not by {@link #size()}.
                 * Otherwise you will be re-reading the same filtered messages.
                 *
                 * @return the number of items read (including the filtered ones).
                 */
                int32_t read_count() const {
                    return items_read_count_;
                }

                const std::vector<typed_data> &get_items() const {
                    return items_;
                }

                /**
                 * Return the sequence number for the item at the given index.
                 *
                 * @param index the index
                 * @return the sequence number for the ringbuffer item
                 * @throws illegal_argument if index out of bounds.
                 * @since 3.9
                 */
                int64_t get_sequence(int32_t index) const {
                    if (index >= (int32_t) item_seqs_->size() || index < 0) {
                        BOOST_THROW_EXCEPTION((exception::exception_builder<exception::illegal_argument>(
                                "read_result_set::getSequence") << "Index " << index
                                                              << " is out of bounds. Sequences size is:"
                                                              << item_seqs_->size()).build());
                    }

                    return (*item_seqs_)[index];
                }

                /**
                 * Returns the sequence of the item following the last read item. This
                 * sequence can then be used to read items following the ones returned by
                 * this result set.
                 * Usually this sequence is equal to the sequence used to retrieve this
                 * result set incremented by the {@link #readCount()}. In cases when the
                 * reader tolerates lost items, this is not the case.
                 * For instance, if the reader requests an item with a stale sequence (one
                 * which has already been overwritten), the read will jump to the oldest
                 * sequence and read from there.
                 * Similarly, if the reader requests an item in the future (e.g. because
                 * the partition was lost and the reader was unaware of this), the read
                 * method will jump back to the newest available sequence.
                 * Because of these jumps and only in the case when the reader is loss
                 * tolerant, the next sequence must be retrieved using this method.
                 * A return value of {@value SEQUENCE_UNAVAILABLE} means that the
                 * information is not available.
                 *
                 * @return the sequence of the item following the last item in the result set
                 * @since 3.10
                 */
                int64_t get_next_sequence_to_read_from() const {
                    return next_seq_;
                }

            private:
                int32_t items_read_count_;
                std::vector<typed_data> items_;
                boost::optional<std::vector<int64_t>> item_seqs_;
                int64_t next_seq_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
