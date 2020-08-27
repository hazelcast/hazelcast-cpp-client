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

#include <unordered_map>
#include <boost/thread/future.hpp>

#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/protocol/ClientMessage.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }
        namespace spi {
            class ClientContext;
            namespace impl {
                class ClientPartitionServiceImpl;
            }
        }
        typedef std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data>> EntryVector;

        namespace proxy {
            /**
             * This base proxy can perform serialization and invocation related operations.
             */
            class HAZELCAST_API SerializingProxy {
            protected:
                SerializingProxy(spi::ClientContext &context, const std::string &objectName);
                
                boost::future<protocol::ClientMessage> invoke(protocol::ClientMessage &request);

                boost::future<protocol::ClientMessage> invokeOnConnection(protocol::ClientMessage &request,
                                                                          std::shared_ptr<connection::Connection> connection);

                boost::future<protocol::ClientMessage>
                invokeOnPartition(protocol::ClientMessage &request, int partitionId);

                boost::future<protocol::ClientMessage>
                invokeOnKeyOwner(protocol::ClientMessage &request,
                                 const serialization::pimpl::Data &keyData);

                boost::future<protocol::ClientMessage> invokeOnMember(protocol::ClientMessage &request,
                                                                      boost::uuids::uuid uuid);

                int getPartitionId(const serialization::pimpl::Data &key);

                template<typename T>
                serialization::pimpl::Data toData(const T *object) {
                    return serializationService_.template toData<T>(object);
                }

                template<typename T>
                serialization::pimpl::Data toData(const T &object) {
                    return toData<T>(&object);
                }

                template<typename T>
                inline boost::optional<T> toObject(const serialization::pimpl::Data &data) {
                    return serializationService_.template toObject<T>(data);
                }

                template<typename T>
                inline boost::optional<T> toObject(const serialization::pimpl::Data *data) {
                    if (!data) {
                        return boost::none;
                    } else {
                        return toObject<T>(*data);
                    }
                }

                template<typename T>
                inline boost::optional<T> toObject(std::unique_ptr<serialization::pimpl::Data> &&data) {
                    return toObject<T>(data.get());
                }

                template<typename T>
                inline boost::optional<T> toObject(std::unique_ptr<serialization::pimpl::Data> &data) {
                    return toObject<T>(data.get());
                }

                template<typename T>
                inline boost::optional<T> toObject(const boost::optional<serialization::pimpl::Data> &data) {
                    return toObject<T>(data.get_ptr());
                }


                template<typename T>
                inline boost::future<boost::optional<T>> toObject(boost::future<boost::optional<serialization::pimpl::Data>> f) {
                    return f.then([=] (boost::future<boost::optional<serialization::pimpl::Data>> f) {
                        return toObject<T>(f.get().get_ptr());
                    });
                }

                template<typename T>
                typename std::enable_if<std::is_same<char *, typename std::remove_cv<T>::type>::value, boost::optional<std::string>>::type
                inline toObject(const serialization::pimpl::Data &data) {
                    return toObject<std::string>(data);
                }

                template<typename T>
                typename std::enable_if<std::is_array<T>::value &&
                                        std::is_same<typename std::remove_all_extents<T>::type, char>::value, boost::optional<std::string>>::type
                inline toObject(const serialization::pimpl::Data &data) {
                    return toObject<std::string>(data);
                }

                template<typename T>
                boost::future<boost::optional<T>> toObject(boost::future<serialization::pimpl::Data> f) {
                    return f.then(boost::launch::deferred, [=] (boost::future<serialization::pimpl::Data> f) {
                        return toObject<T>(f.get());
                    });
                }

                template<typename T>
                boost::future<boost::optional<T>> toObject(boost::future<std::unique_ptr<serialization::pimpl::Data>> f) {
                    return f.then(boost::launch::deferred, [=] (boost::future<std::unique_ptr<serialization::pimpl::Data>> f) {
                        return toObject<T>(f.get());
                    });
                }

                template<typename T>
                inline boost::future<std::vector<T>>
                toObjectVector(boost::future<std::vector<serialization::pimpl::Data>> dataFuture) {
                    return dataFuture.then(boost::launch::deferred,
                                           [=](boost::future<std::vector<serialization::pimpl::Data>> f) {
                                               auto dataResult = f.get();
                                               std::vector<T> result;
                                               result.reserve(dataResult.size());
                                               std::for_each(dataResult.begin(), dataResult.end(),
                                                             [&](const serialization::pimpl::Data &keyData) {
                                                                 // The object is guaranteed to exist (non-null)
                                                                 result.push_back(std::move(toObject<T>(keyData).value()));
                                                             });
                                               return result;
                                           });
                }

                template<typename K, typename V>
                boost::future<std::unordered_map<K, boost::optional<V>>> toObjectMap(boost::future<EntryVector> entriesData) {
                    return entriesData.then(boost::launch::deferred, [=](boost::future<EntryVector> f) {
                        auto entries = f.get();
                        std::unordered_map<K, boost::optional<V>> result;
                        result.reserve(entries.size());
                        std::for_each(entries.begin(), entries.end(), [&](std::pair<serialization::pimpl::Data, serialization::pimpl::Data> entry) {
                            result.insert({std::move(toObject<K>(entry.first)).value(), toObject<V>(entry.second)});
                        });
                        return result;
                    });
                }

                template<typename K, typename V>
                inline boost::future<std::vector<std::pair<K, V>>>
                toEntryObjectVector(boost::future<EntryVector> dataFuture) {
                    return dataFuture.then(boost::launch::deferred, [=](boost::future<EntryVector> f) {
                        auto dataEntryVector = f.get();
                        std::vector<std::pair<K, V>> result;
                        result.reserve(dataEntryVector.size());
                        std::for_each(dataEntryVector.begin(), dataEntryVector.end(),
                                      [&](const std::pair<serialization::pimpl::Data, serialization::pimpl::Data> &entryData) {
                                          // please note that the key and value will never be null
                                          result.emplace_back(std::move(toObject<K>(entryData.first)).value(),
                                                                 std::move(toObject<V>(entryData.second)).value());
                                      });
                        return result;
                    });
                }

                template<typename T>
                boost::future<void> toVoidFuture(boost::future<T> messageFuture) {
                    return messageFuture.then(boost::launch::deferred, [](boost::future<T> f) {
                        f.get(); }
                        );
                }

                template<typename T>
                std::vector<serialization::pimpl::Data> toDataCollection(const std::vector<T> &elements) {
                    std::vector<serialization::pimpl::Data> dataCollection;
                    dataCollection.reserve(elements.size());
                    std::for_each(elements.begin(), elements.end(),
                                  [&](const T &item) { dataCollection.push_back(toData(item)); });
                    return dataCollection;
                }

                template<typename K, typename V>
                EntryVector toDataEntries(const std::unordered_map<K, V> &m) {
                    EntryVector entries;
                    entries.reserve(m.size());
                    std::for_each(m.begin(), m.end(), [&] (const typename std::unordered_map<K, V>::value_type &entry) {
                        entries.emplace_back(toData<K>(entry.first), toData<V>(entry.second));
                    });
                    return entries;
                }

                template<typename T>
                boost::future<T> invokeAndGetFuture(protocol::ClientMessage &request) {
                    return decode<T>(invoke(request));
                }

                template<typename T>
                boost::future<T> invokeAndGetFuture(protocol::ClientMessage &request, int partitionId) {
                    return decode<T>(invokeOnPartition(request, partitionId));
                }

                template<typename T>
                boost::future<T> invokeAndGetFuture(protocol::ClientMessage &request,
                                     const serialization::pimpl::Data &key) {
                    return decode<T>(invokeOnKeyOwner(request, key));
                }

            protected:
                template<typename T>
                static boost::future<boost::optional<T>>
                decode_optional_var_sized(boost::future<protocol::ClientMessage> f) {
                    return f.then(boost::launch::deferred, [](boost::future<protocol::ClientMessage> f) {
                        auto msg = f.get();
                        return msg.get_first_optional_var_sized_field<T>();
                    });
                }

                template<typename T>
                typename std::enable_if<std::is_trivial<T>::value, boost::future<T>>::type
                static decode(boost::future<protocol::ClientMessage> f) {
                    return f.then(boost::launch::deferred, [](boost::future<protocol::ClientMessage> f) {
                        auto msg = f.get();
                        return msg.get_first_fixed_sized_field<T>();
                    });
                }

                template<typename T>
                typename std::enable_if<!std::is_trivial<T>::value, boost::future<T>>::type
                static decode(boost::future<protocol::ClientMessage> f) {
                    return f.then(boost::launch::deferred, [](boost::future<protocol::ClientMessage> f) {
                        auto msg = f.get();
                        return *msg.get_first_var_sized_field<T>();
                    });
                }

                serialization::pimpl::SerializationService &serializationService_;
                spi::impl::ClientPartitionServiceImpl &partitionService_;
                std::string name_;
                spi::ClientContext &context_;
            };

            template<>
            boost::future<boost::optional<serialization::pimpl::Data>>
            HAZELCAST_API SerializingProxy::invokeAndGetFuture(protocol::ClientMessage &request);

            template<>
            boost::future<boost::optional<map::DataEntryView>>
            HAZELCAST_API SerializingProxy::invokeAndGetFuture(protocol::ClientMessage &request,
                                                 const serialization::pimpl::Data &key);

            template<>
            boost::future<boost::optional<serialization::pimpl::Data>>
            HAZELCAST_API SerializingProxy::invokeAndGetFuture(protocol::ClientMessage &request, int partitionId);


            template<>
            boost::future<boost::optional<serialization::pimpl::Data>>
            HAZELCAST_API SerializingProxy::invokeAndGetFuture(protocol::ClientMessage &request,
                                                 const serialization::pimpl::Data &key);

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
