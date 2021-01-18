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

#include <string>
#include <memory>
#include <cassert>

#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/query/predicates.h"
#include "hazelcast/client/query/entry_comparator.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                namespace holder {
                    struct paging_predicate_holder;
                }
            }
        }
        class imap;
        namespace query {
            class paging_predicate_marker {};

            /**
             * To differentiate users selection on result collection on map-wide operations
             * like values , keySet , query etc.
             */
            enum struct HAZELCAST_API iteration_type {
                /**
                 * Iterate over keys
                 */
                        KEY = 0,
                /**
                 * Iterate over values
                 */
                        VALUE = 1,
                /**
                 * Iterate over whole entry (so key and value)
                 */
                        ENTRY = 2
            };

            static const std::string IterationNames[] = {"KEY", "VALUE", "ENTRY"};

            struct anchor_data_list {
                std::vector<int32_t> page_list;
                std::vector<std::pair<serialization::pimpl::data, boost::optional<serialization::pimpl::data>>> data_list;
            };

            /**
             * NOTE: paging_predicate can only be used with values(), keySet() and entries() methods!!!
             *
             * This class is a special Predicate which helps to get a page-by-page result of a query.
             * It can be constructed with a page-size, an inner predicate for filtering, and a comparator for sorting.
             * This class is not thread-safe and stateless. To be able to reuse for another query, one should call
             * {@link paging_predicate#reset()}
             * <br/>
             * Here is an example usage.
             * <pre>
             * Predicate lessEqualThanFour = Predicates.lessEqual("this", 4);
             *
             * // We are constructing our paging predicate with a predicate and page size. In this case query results fetched two
             * by two.
             * paging_predicate predicate = new paging_predicate(lessEqualThanFour, 2);
             *
             * // we are initializing our map with integers from 0 to 10 as keys and values.
             * IMap map = hazelcastInstance.getMap(...);
             * for (int i = 0; i < 10; i++) {
             * map.put(i, i);
             * }
             *
             * // invoking the query
             * Collection<Integer> values = map.values(predicate);
             * System.out.println("values = " + values) // will print 'values = [0, 1]'
             * predicate.nextPage(); // we are setting up paging predicate to fetch next page in the next call.
             * values = map.values(predicate);
             * System.out.println("values = " + values);// will print 'values = [2, 3]'
             * Entry anchor = predicate.getAnchor();
             * System.out.println("anchor -> " + anchor); // will print 'anchor -> 1=1',  since the anchor is the last entry of
             * the previous page.
             * predicate.previousPage(); // we are setting up paging predicate to fetch previous page in the next call
             * values = map.values(predicate);
             * System.out.println("values = " + values) // will print 'values = [0, 1]'
             * </pre>
             */
            template<typename K, typename V>
            class paging_predicate : public predicate, public paging_predicate_marker {
                friend imap;
                friend protocol::codec::holder::paging_predicate_holder;
                friend serialization::hz_serializer<query::paging_predicate<K, V>>;
            public:
                ~paging_predicate() = default;

                /**
                 * resets for reuse
                 */
                void reset() {
                    iteration_type_ = iteration_type::VALUE;
                    anchor_data_list_.page_list.clear();
                    anchor_data_list_.data_list.clear();
                    page_ = 0;
                }

                /**
                 * sets the page value to next page
                 */
                void next_page() {
                    ++page_;
                }

                /**
                 * sets the page value to previous page
                 */
                void previous_page() {
                    if (page_ != 0) {
                        --page_;
                    }
                }

                iteration_type get_iteration_type() const {
                    return iteration_type_;
                }

                void set_iteration_type(iteration_type type) {
                    iteration_type_ = type;
                }

                size_t get_page() const {
                    return page_;
                }

                void set_page(size_t page_number) {
                    page_ = page_number;
                }

                size_t get_page_size() const {
                    return page_size_;
                }

                const query::entry_comparator<K, V> *get_comparator() const {
                    return comparator_.get();
                }

                void set_anchor_data_list(anchor_data_list anchor_data_list) {
                    anchor_data_list_ = std::move(anchor_data_list);
                }

            private:
                anchor_data_list anchor_data_list_;
                std::shared_ptr<query::entry_comparator<K, V>> comparator_;
                serialization::object_data_output out_stream_;
                size_t page_size_;
                size_t page_;
                iteration_type iteration_type_;
                boost::optional<serialization::pimpl::data> comparator_data_;
                boost::optional<serialization::pimpl::data> predicate_data_;

                /**
                 * Construct with a pageSize
                 * results will not be filtered
                 * results will be natural ordered
                 * throws illegal_argument {@link illegal_argument} if pageSize is not greater than 0
                 *
                 * @param predicatePageSize size of the page
                 */
                paging_predicate(serialization::pimpl::SerializationService &serialization_service,
                                 size_t predicate_page_size) : out_stream_(serialization_service.new_output_stream()),
                        page_size_(predicate_page_size), page_(0), iteration_type_(iteration_type::VALUE) {
                    out_stream_.write_object<bool>(nullptr);
                    out_stream_.write_object<bool>(nullptr);
                }

                /**
                 * Construct with an inner predicate and pageSize
                 * results will be filtered via inner predicate
                 * results will be natural ordered
                 * throws illegal_argument {@link illegal_argument} if pageSize is not greater than 0
                 * throws illegal_argument {@link illegal_argument} if inner predicate is also {@link paging_predicate}
                 *
                 * @param predicate the inner predicate through which results will be filtered
                 * @param predicatePageSize  the page size
                 */
                template<typename INNER_PREDICATE>
                paging_predicate(serialization::pimpl::SerializationService &serialization_service,
                                 size_t predicate_page_size, const INNER_PREDICATE &predicate)
                        : out_stream_(serialization_service.new_output_stream()), page_size_(predicate_page_size), page_(0),
                        iteration_type_(iteration_type::VALUE) {
                    out_stream_.write_object(predicate);
                    out_stream_.write_object<bool>(nullptr);
                    predicate_data_ = serialization_service.to_data<INNER_PREDICATE>(predicate);
                }

                /**
                 * Construct with a comparator and pageSize
                 * results will not be filtered
                 * results will be ordered via comparator
                 * throws illegal_argument {@link illegal_argument} if pageSize is not greater than 0
                 *
                 * @param comparatorObj the comparator through which results will be ordered
                 * @param predicatePageSize   the page size
                 */
                template<typename COMPARATOR>
                paging_predicate(serialization::pimpl::SerializationService &serialization_service,
                                 COMPARATOR &&comp, size_t predicate_page_size)
                        : out_stream_(serialization_service.new_output_stream()), page_size_(predicate_page_size), page_(0),
                        iteration_type_(iteration_type::VALUE) {
                    out_stream_.write_object<bool>(nullptr);
                    out_stream_.write_object(comp);
                    comparator_data_ = serialization_service.to_data<COMPARATOR>(comp);
                    comparator_ = std::make_shared<COMPARATOR>(std::forward<COMPARATOR>(comp));
                }

                /**
                 * Construct with an inner predicate, comparator and pageSize
                 * results will be filtered via inner predicate
                 * results will be ordered via comparator
                 * throws {@link illegal_argument} if pageSize is not greater than 0
                 * throws {@link illegal_argument} if inner predicate is also {@link paging_predicate}
                 *
                 * @param predicate  the inner predicate through which results will be filtered
                 * @param comparatorObj the comparator through which results will be ordered
                 * @param predicatePageSize   the page size
                 */
                template<typename INNER_PREDICATE, typename COMPARATOR>
                paging_predicate(serialization::pimpl::SerializationService &serialization_service,
                                 const INNER_PREDICATE &predicate, COMPARATOR &&comp, size_t predicate_page_size)
                        : out_stream_(serialization_service.new_output_stream()), page_size_(predicate_page_size), page_(0),
                        iteration_type_(iteration_type::VALUE) {
                    out_stream_.write_object(predicate);
                    out_stream_.write_object(comp);
                    predicate_data_ = serialization_service.to_data<INNER_PREDICATE>(predicate);
                    comparator_data_ = serialization_service.to_data<COMPARATOR>(comp);
                    comparator_ = std::make_shared<COMPARATOR>(std::forward<COMPARATOR>(comp));
                }
            };
        }

        namespace serialization {
            template<typename K, typename V>
            struct hz_serializer<query::paging_predicate<K, V>> : public identified_data_serializer {
                /**
                 * @return factory id
                 */
                static constexpr int32_t get_factory_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::F_ID);
                }

                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::PAGING_PREDICATE);
                }

                /**
                 * Defines how this class will be written.
                 * @param writer object_data_output
                 */
                static void write_data(const query::paging_predicate<K, V> &obj, object_data_output &out) {
                    out.write_bytes(obj.outStream.toByteArray());
                    out.write<int32_t>((int32_t) obj.page);
                    out.write<int32_t>((int32_t) obj.pageSize);
                    out.write<std::string>(obj.IterationNames[static_cast<int32_t>(obj.iterationType)]);
                    out.write<int32_t>((int32_t) obj.anchor_data_list_.data_list.size());
                    const auto &data_list = obj.anchor_data_list_.data_list;
                    const auto &page_list = obj.anchor_data_list_.page_list;
                    for (size_t i = 0; i < obj.anchor_data_list_.data_list.size(); ++i) {
                        out.write<int32_t>(page_list[i]);
                        out.write_object<K>(data_list[i].first);
                        out.write_object<V>(data_list[i].second);
                    }
                }

                /**
                 * Should not be called at the client side!
                 */
                static query::paging_predicate<K, V> read_data(object_data_input &in) {
                    // Not need to read at the client side
                    BOOST_THROW_EXCEPTION(exception::hazelcast_serialization("readData",
                                                                                       "Client should not need to use readdata method!!!"));
                }
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


