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

#include <string>
#include <memory>
#include <cassert>

#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/query/Predicates.h"
#include "hazelcast/client/query/EntryComparator.h"

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
        class IMap;
        namespace query {
            class PagingPredicateMarker {};

            /**
             * To differentiate users selection on result collection on map-wide operations
             * like values , keySet , query etc.
             */
            enum struct HAZELCAST_API IterationType {
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
                std::vector<std::pair<serialization::pimpl::Data, boost::optional<serialization::pimpl::Data>>> data_list;
            };

            /**
             * NOTE: PagingPredicate can only be used with values(), keySet() and entries() methods!!!
             *
             * This class is a special Predicate which helps to get a page-by-page result of a query.
             * It can be constructed with a page-size, an inner predicate for filtering, and a comparator for sorting.
             * This class is not thread-safe and stateless. To be able to reuse for another query, one should call
             * {@link PagingPredicate#reset()}
             * <br/>
             * Here is an example usage.
             * <pre>
             * Predicate lessEqualThanFour = Predicates.lessEqual("this", 4);
             *
             * // We are constructing our paging predicate with a predicate and page size. In this case query results fetched two
             * by two.
             * PagingPredicate predicate = new PagingPredicate(lessEqualThanFour, 2);
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
            class PagingPredicate : public Predicate, public PagingPredicateMarker {
                friend IMap;
                friend protocol::codec::holder::paging_predicate_holder;
                friend serialization::hz_serializer<query::PagingPredicate<K, V>>;
            public:
                ~PagingPredicate() = default;

                /**
                 * resets for reuse
                 */
                void reset() {
                    iterationType_ = IterationType::VALUE;
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

                IterationType get_iteration_type() const {
                    return iterationType_;
                }

                void set_iteration_type(IterationType type) {
                    iterationType_ = type;
                }

                size_t get_page() const {
                    return page_;
                }

                void set_page(size_t page_number) {
                    page_ = page_number;
                }

                size_t get_page_size() const {
                    return pageSize_;
                }

                const query::EntryComparator<K, V> *get_comparator() const {
                    return comparator_.get();
                }

                void set_anchor_data_list(anchor_data_list anchor_data_list) {
                    anchor_data_list_ = std::move(anchor_data_list);
                }

            private:
                anchor_data_list anchor_data_list_;
                std::shared_ptr<query::EntryComparator<K, V>> comparator_;
                serialization::ObjectDataOutput outStream_;
                size_t pageSize_;
                size_t page_;
                IterationType iterationType_;
                boost::optional<serialization::pimpl::Data> comparator_data_;
                boost::optional<serialization::pimpl::Data> predicate_data_;

                /**
                 * Construct with a pageSize
                 * results will not be filtered
                 * results will be natural ordered
                 * throws IllegalArgumentException {@link IllegalArgumentException} if pageSize is not greater than 0
                 *
                 * @param predicatePageSize size of the page
                 */
                PagingPredicate(serialization::pimpl::SerializationService &serialization_service,
                        size_t predicate_page_size) : outStream_(serialization_service.new_output_stream()),
                        pageSize_(predicate_page_size), page_(0), iterationType_(IterationType::VALUE) {
                    outStream_.write_object<bool>(nullptr);
                    outStream_.write_object<bool>(nullptr);
                }

                /**
                 * Construct with an inner predicate and pageSize
                 * results will be filtered via inner predicate
                 * results will be natural ordered
                 * throws IllegalArgumentException {@link IllegalArgumentException} if pageSize is not greater than 0
                 * throws IllegalArgumentException {@link IllegalArgumentException} if inner predicate is also {@link PagingPredicate}
                 *
                 * @param predicate the inner predicate through which results will be filtered
                 * @param predicatePageSize  the page size
                 */
                template<typename INNER_PREDICATE>
                PagingPredicate(serialization::pimpl::SerializationService &serialization_service,
                        size_t predicate_page_size, const INNER_PREDICATE &predicate)
                        : outStream_(serialization_service.new_output_stream()), pageSize_(predicate_page_size), page_(0),
                        iterationType_(IterationType::VALUE) {
                    outStream_.write_object(predicate);
                    outStream_.write_object<bool>(nullptr);
                    predicate_data_ = serialization_service.to_data<INNER_PREDICATE>(predicate);
                }

                /**
                 * Construct with a comparator and pageSize
                 * results will not be filtered
                 * results will be ordered via comparator
                 * throws IllegalArgumentException {@link IllegalArgumentException} if pageSize is not greater than 0
                 *
                 * @param comparatorObj the comparator through which results will be ordered
                 * @param predicatePageSize   the page size
                 */
                template<typename COMPARATOR>
                PagingPredicate(serialization::pimpl::SerializationService &serialization_service,
                        COMPARATOR &&comp, size_t predicate_page_size)
                        : outStream_(serialization_service.new_output_stream()), pageSize_(predicate_page_size), page_(0),
                        iterationType_(IterationType::VALUE) {
                    outStream_.write_object<bool>(nullptr);
                    outStream_.write_object(comp);
                    comparator_data_ = serialization_service.to_data<COMPARATOR>(comp);
                    comparator_ = std::make_shared<COMPARATOR>(std::forward<COMPARATOR>(comp));
                }

                /**
                 * Construct with an inner predicate, comparator and pageSize
                 * results will be filtered via inner predicate
                 * results will be ordered via comparator
                 * throws {@link IllegalArgumentException} if pageSize is not greater than 0
                 * throws {@link IllegalArgumentException} if inner predicate is also {@link PagingPredicate}
                 *
                 * @param predicate  the inner predicate through which results will be filtered
                 * @param comparatorObj the comparator through which results will be ordered
                 * @param predicatePageSize   the page size
                 */
                template<typename INNER_PREDICATE, typename COMPARATOR>
                PagingPredicate(serialization::pimpl::SerializationService &serialization_service,
                        const INNER_PREDICATE &predicate, COMPARATOR &&comp, size_t predicate_page_size)
                        : outStream_(serialization_service.new_output_stream()), pageSize_(predicate_page_size), page_(0),
                        iterationType_(IterationType::VALUE) {
                    outStream_.write_object(predicate);
                    outStream_.write_object(comp);
                    predicate_data_ = serialization_service.to_data<INNER_PREDICATE>(predicate);
                    comparator_data_ = serialization_service.to_data<COMPARATOR>(comp);
                    comparator_ = std::make_shared<COMPARATOR>(std::forward<COMPARATOR>(comp));
                }
            };
        }

        namespace serialization {
            template<typename K, typename V>
            struct hz_serializer<query::PagingPredicate<K, V>> : public identified_data_serializer {
                /**
                 * @return factory id
                 */
                static constexpr int32_t get_factory_id() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::F_ID);
                }

                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::PAGING_PREDICATE);
                }

                /**
                 * Defines how this class will be written.
                 * @param writer ObjectDataOutput
                 */
                static void write_data(const query::PagingPredicate<K, V> &obj, ObjectDataOutput &out) {
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
                static query::PagingPredicate<K, V> read_data(ObjectDataInput &in) {
                    // Not need to read at the client side
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("readData",
                                                                                     "Client should not need to use readData method!!!"));
                }
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


