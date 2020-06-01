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

#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/query/Predicates.h"
#include "hazelcast/client/exception/IException.h"
#include "hazelcast/client/query/EntryComparator.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace query {
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

            template<typename K, typename V>
            class InvalidComparator :  public query::EntryComparator<K, V> {
            public:
                int compare(const std::pair<const K *, const V *> &lhs, const std::pair<const K *, const V *> &rhs) const {
                    assert(0);
                    return -1;
                }

                int getFactoryId() const {
                    assert(0);
                    return -1;
                }

                int getClassId() const {
                    assert(0);
                    return -1;
                }

                void writeData(serialization::ObjectDataOutput &writer) const {
                    assert(0);
                }

                void readData(serialization::ObjectDataInput &reader) {
                    assert(0);
                }
            };

            static const std::string IterationNames[] = {"KEY", "VALUE", "ENTRY"};

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

            template<typename K, typename V, typename INNER_PREDICATE = char, typename COMPARATOR = query::EntryComparator<K, V>>
            class PagingPredicate : public Predicate {
            public:
                /**
                 * Construct with a pageSize
                 * results will not be filtered
                 * results will be natural ordered
                 * throws IllegalArgumentException {@link IllegalArgumentException} if pageSize is not greater than 0
                 *
                 * @param predicatePageSize size of the page
                 */
                PagingPredicate(size_t predicatePageSize) : pageSize(predicatePageSize), page(0),iterationType(IterationType::VALUE) {
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
                PagingPredicate(INNER_PREDICATE predicate, size_t predicatePageSize) : innerPredicate(std::move(predicate)),
                                                                                             pageSize(predicatePageSize),
                                                                                             page(0),
                                                                                             iterationType(IterationType::VALUE) {
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
                PagingPredicate(COMPARATOR comparatorObj, size_t predicatePageSize) : comparator(
                        std::move(comparatorObj)), pageSize(predicatePageSize), page(0), iterationType(IterationType::VALUE) {
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
                PagingPredicate(INNER_PREDICATE predicate, COMPARATOR comparatorObj,
                                size_t predicatePageSize) : innerPredicate(std::move(predicate)), comparator(std::move(comparatorObj)),
                                                         pageSize(predicatePageSize), page(0), iterationType(IterationType::VALUE) {}

                ~PagingPredicate() {
                    for (typename std::vector<std::pair<size_t, std::pair<K *, V *> > >::const_iterator it = anchorList.begin();
                         it != anchorList.end(); ++it) {
                        delete it->second.first;
                        delete it->second.second;
                    }
                }

                /**
                 * resets for reuse
                 */
                void reset() {
                    iterationType = IterationType::VALUE;
                    for (typename std::vector<std::pair<size_t, std::pair<K *, V *> > >::const_iterator it = anchorList.begin();
                         it != anchorList.end(); ++it) {
                        delete it->second.first;
                        delete it->second.second;
                    }
                    anchorList.clear();
                    page = 0;
                }

                /**
                 * sets the page value to next page
                 */
                void nextPage() {
                    ++page;
                }

                /**
                 * sets the page value to previous page
                 */
                void previousPage() {
                    if (page != 0) {
                        --page;
                    }
                }

                IterationType getIterationType() const {
                    return iterationType;
                }

                void setIterationType(IterationType type) {
                    iterationType = type;
                }

                size_t getPage() const {
                    return page;
                }

                void setPage(size_t pageNumber) {
                    page = pageNumber;
                }

                size_t getPageSize() const {
                    return pageSize;
                }

                const Predicate *getPredicate() const {
                    if (!innerPredicate) {
                        return nullptr;
                    }
                    return &innerPredicate.value();
                }

                const query::EntryComparator<K, V> *getComparator() const {
                    if (!comparator) {
                        return nullptr;
                    }
                    return comparator.get_ptr();
                }


                /**
                 * Retrieve the anchor object which is the last value object on the previous page.
                 * <p/>
                 * Note: This method will return `NULL` on the first page of the query result.
                 *
                 * @return std::pair<K *, V *> the anchor object which is the last value object on the previous page
                 */
                const std::pair<K *, V *> *getAnchor() const {
                    if (0 == anchorList.size()) {
                        return (const std::pair<K *, V *> *)NULL;
                    }

                    return &anchorList[page].second;
                }

                /**
                 * After each query, an anchor entry is set for that page. see {@link #setAnchor(int, Map.Entry)}}
                 * For the next query user may set an arbitrary page. see {@link #setPage(int)}
                 * for example: user queried first 5 pages which means first 5 anchor is available
                 * if the next query is for the 10th page then the nearest anchor belongs to page 5
                 * but if the next query is for the 3rd page then the nearest anchor belongs to page 2
                 *
                 * @return nearest anchored entry for current page
                 */
                const std::pair<size_t, std::pair<K *, V *> > *getNearestAnchorEntry() {
                    size_t anchorCount = anchorList.size();

                    if (page == 0 || anchorCount == 0) {
                        return (const std::pair<size_t, std::pair<K *, V *> > *) NULL;
                    }

                    if (page < anchorCount) {
                        return &anchorList[page - 1];
                    } else {
                        return &anchorList[anchorCount - 1];
                    }
                }

                /**
                 * Defines how this class will be written.
                 * @param writer ObjectDataOutput
                 */
                void writeData(serialization::ObjectDataOutput &out) const {
                    out.writeObject<INNER_PREDICATE>(innerPredicate);
                    out.writeObject<COMPARATOR>(comparator);
                    out.write<int32_t>((int)page);
                    out.write<int32_t>((int)pageSize);
                    out.write<std::string>(IterationNames[static_cast<int32_t>(iterationType)]);
                    out.write<int32_t>((int) anchorList.size());
                    for (typename std::vector<std::pair<size_t, std::pair<K *, V *> > >::const_iterator it = anchorList.begin();
                         it != anchorList.end(); ++it) {
                        out.write<int32_t>((int)it->first);
                        out.writeObject<K>(it->second.first);
                        out.writeObject<V>(it->second.second);
                    }
                }

                void setAnchor(size_t pageNumber, const std::pair<K *, V *> &anchorEntry) {
                    size_t anchorCount = anchorList.size();
                    if (pageNumber < anchorCount) {
                        // release the previous anchoe entry
                        delete anchorList[pageNumber].second.first;
                        delete anchorList[pageNumber].second.second;
                        anchorList[pageNumber] = std::pair<size_t, std::pair<K *, V *> >(pageNumber, anchorEntry);
                    } else if (pageNumber == anchorCount) {
                        anchorList.push_back(std::pair<size_t, std::pair<K *, V *> >(pageNumber, anchorEntry));
                    } else {
                        char msg[200];
                        util::hz_snprintf(msg, 200, "Anchor index is not correct, expected: %d but found: %d", pageNumber,
                                          anchorCount);
                        BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("PagingPredicate::setAnchor", msg));
                    }
                }

            private:
                boost::optional<INNER_PREDICATE> innerPredicate;
                // key is the page number, the value is the map entry as the anchor
                std::vector<std::pair<size_t, std::pair<K *, V *> > > anchorList;
                boost::optional<COMPARATOR> comparator;
                size_t pageSize;
                size_t page;
                IterationType iterationType;
            };
        }

        namespace serialization {
            template<typename K, typename V>
            struct hz_serializer<query::PagingPredicate<K, V>> : public identified_data_serializer {
                /**
                 * @return factory id
                 */
                static constexpr int32_t getFactoryId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::F_ID);
                }

                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::PAGING_PREDICATE);
                }

                /**
                 * Defines how this class will be written.
                 * @param writer ObjectDataOutput
                 */
                static void writeData(const query::PagingPredicate<K, V> &object, ObjectDataOutput &out) {
                    object.writeData(out);
                }

                /**
                 * Should not be called at the client side!
                 */
                static query::PagingPredicate<K, V> readData(ObjectDataInput &in) {
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

