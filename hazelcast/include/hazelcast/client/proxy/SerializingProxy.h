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
        typedef std::vector<std::pair<serialization::pimpl::data, serialization::pimpl::data>> EntryVector;

        namespace proxy {
            /**
             * This base proxy can perform serialization and invocation related operations.
             */
            class HAZELCAST_API SerializingProxy {
            public:
                template<typename T>
                static boost::future<void> to_void_future(boost::future<T> message_future) {
                    return message_future.then(boost::launch::async, [](boost::future<T> f) {
                                                   f.get();
                                               }
                    );
                }

            protected:
                SerializingProxy(spi::ClientContext &context, const std::string &object_name);
                
                boost::future<protocol::ClientMessage> invoke(protocol::ClientMessage &request);

                boost::future<protocol::ClientMessage> invoke_on_connection(protocol::ClientMessage &request,
                                                                          std::shared_ptr<connection::Connection> connection);

                boost::future<protocol::ClientMessage>
                invoke_on_partition(protocol::ClientMessage &request, int partition_id);

                boost::future<protocol::ClientMessage>
                invoke_on_key_owner(protocol::ClientMessage &request,
                                 const serialization::pimpl::data &key_data);

                boost::future<protocol::ClientMessage> invoke_on_member(protocol::ClientMessage &request,
                                                                      boost::uuids::uuid uuid);

                int get_partition_id(const serialization::pimpl::data &key);

                template<typename T>
                serialization::pimpl::data to_data(const T *object) {
                    return serialization_service_.template to_data<T>(object);
                }

                template<typename T>
                serialization::pimpl::data to_data(const T &object) {
                    return to_data<T>(&object);
                }

                template<typename T>
                inline boost::optional<T> to_object(const serialization::pimpl::data &data) {
                    return serialization_service_.template to_object<T>(data);
                }

                template<typename T>
                inline boost::optional<T> to_object(const serialization::pimpl::data *data) {
                    if (!data) {
                        return boost::none;
                    } else {
                        return to_object<T>(*data);
                    }
                }

                template<typename T>
                inline boost::optional<T> to_object(std::unique_ptr<serialization::pimpl::data> &&data) {
                    return to_object<T>(data.get());
                }

                template<typename T>
                inline boost::optional<T> to_object(std::unique_ptr<serialization::pimpl::data> &data) {
                    return to_object<T>(data.get());
                }

                template<typename T>
                inline boost::optional<T> to_object(const boost::optional<serialization::pimpl::data> &data) {
                    return to_object<T>(data.get_ptr());
                }


                template<typename T>
                inline boost::future<boost::optional<T>> to_object(boost::future<boost::optional<serialization::pimpl::data>> f) {
                    return f.then([=] (boost::future<boost::optional<serialization::pimpl::data>> f) {
                        return to_object<T>(f.get().get_ptr());
                    });
                }

                template<typename T>
                typename std::enable_if<std::is_same<char *, typename std::remove_cv<T>::type>::value, boost::optional<std::string>>::type
                inline to_object(const serialization::pimpl::data &data) {
                    return to_object<std::string>(data);
                }

                template<typename T>
                typename std::enable_if<std::is_array<T>::value &&
                                        std::is_same<typename std::remove_all_extents<T>::type, char>::value, boost::optional<std::string>>::type
                inline to_object(const serialization::pimpl::data &data) {
                    return to_object<std::string>(data);
                }

                template<typename T>
                boost::future<boost::optional<T>> to_object(boost::future<serialization::pimpl::data> f) {
                    return f.then(boost::launch::async, [=](boost::future<serialization::pimpl::data> f) {
                        return to_object<T>(f.get());
                    });
                }

                template<typename T>
                boost::future<boost::optional<T>> to_object(boost::future<std::unique_ptr<serialization::pimpl::data>> f) {
                    return f.then(boost::launch::async,
                                  [=](boost::future<std::unique_ptr<serialization::pimpl::data>> f) {
                                      return to_object<T>(f.get());
                                  });
                }

                template<typename T>
                inline boost::future<std::vector<T>>
                to_object_vector(boost::future<std::vector<serialization::pimpl::data>> data_future) {
                    return data_future.then(boost::launch::async,
                                            [=](boost::future<std::vector<serialization::pimpl::data>> f) {
                                                auto dataResult = f.get();
                                                std::vector<T> result;
                                                result.reserve(dataResult.size());
                                                for (const auto &d : dataResult) {
                                                    // The object is guaranteed to exist (non-null)
                                                    result.push_back(std::move(to_object<T>(d).value()));
                                                }
                                                return result;
                                            });
                }

                template<typename K, typename V>
                boost::future<std::unordered_map<K, boost::optional<V>>> to_object_map(boost::future<EntryVector> entries_data) {
                    return entries_data.then(boost::launch::async, [=](boost::future<EntryVector> f) {
                        auto entries = f.get();
                        std::unordered_map<K, boost::optional<V>> result;
                        result.reserve(entries.size());
                        for (const auto &e : entries) {
                            result.insert({std::move(to_object<K>(e.first)).value(), to_object<V>(e.second)});
                        }
                        return result;
                    });
                }

                template<typename K, typename V>
                inline boost::future<std::vector<std::pair<K, V>>>
                to_entry_object_vector(boost::future<EntryVector> data_future) {
                    return data_future.then(boost::launch::async, [=](boost::future<EntryVector> f) {
                        auto dataEntryVector = f.get();
                        std::vector<std::pair<K, V>> result;
                        result.reserve(dataEntryVector.size());
                        for (const auto &e : dataEntryVector) {
                            // please note that the key and value will never be null
                            result.emplace_back(std::move(to_object<K>(e.first)).value(),
                                                std::move(to_object<V>(e.second)).value());
                        }
                        return result;
                    });
                }

                template<typename T>
                std::vector<serialization::pimpl::data> to_data_collection(const std::vector<T> &elements) {
                    std::vector<serialization::pimpl::data> dataCollection;
                    dataCollection.reserve(elements.size());
                    for (const auto &e : elements) {
                        dataCollection.push_back(to_data(e));
                    }
                    return dataCollection;
                }

                template<typename K, typename V>
                EntryVector to_data_entries(const std::unordered_map<K, V> &m) {
                    EntryVector entries;
                    entries.reserve(m.size());
                    for (const auto &e : m) {
                        entries.emplace_back(to_data<K>(e.first), to_data<V>(e.second));
                    }
                    return entries;
                }

                template<typename T>
                boost::future<T> invoke_and_get_future(protocol::ClientMessage &request) {
                    return decode<T>(invoke(request));
                }

                template<typename T>
                boost::future<T> invoke_and_get_future(protocol::ClientMessage &request, int partition_id) {
                    return decode<T>(invoke_on_partition(request, partition_id));
                }

                template<typename T>
                boost::future<T> invoke_and_get_future(protocol::ClientMessage &request,
                                     const serialization::pimpl::data &key) {
                    return decode<T>(invoke_on_key_owner(request, key));
                }

            protected:
                template<typename T>
                static boost::future<boost::optional<T>>
                decode_optional_var_sized(boost::future<protocol::ClientMessage> f) {
                    return f.then(boost::launch::async, [](boost::future<protocol::ClientMessage> f) {
                        auto msg = f.get();
                        return msg.get_first_optional_var_sized_field<T>();
                    });
                }

                template<typename T>
                typename std::enable_if<std::is_trivial<T>::value, boost::future<T>>::type
                static decode(boost::future<protocol::ClientMessage> f) {
                    return f.then(boost::launch::async, [](boost::future<protocol::ClientMessage> f) {
                        auto msg = f.get();
                        return msg.get_first_fixed_sized_field<T>();
                    });
                }

                template<typename T>
                typename std::enable_if<!std::is_trivial<T>::value, boost::future<T>>::type
                static decode(boost::future<protocol::ClientMessage> f) {
                    return f.then(boost::launch::async, [](boost::future<protocol::ClientMessage> f) {
                        auto msg = f.get();
                        return *msg.get_first_var_sized_field<T>();
                    });
                }

                serialization::pimpl::SerializationService &serialization_service_;
                spi::impl::ClientPartitionServiceImpl &partition_service_;
                std::string object_name_;
                spi::ClientContext &client_context_;
            };

            template<>
            boost::future<boost::optional<serialization::pimpl::data>>
            HAZELCAST_API SerializingProxy::invoke_and_get_future(protocol::ClientMessage &request);

            template<>
            boost::future<boost::optional<map::data_entry_view>>
            HAZELCAST_API SerializingProxy::invoke_and_get_future(protocol::ClientMessage &request,
                                                 const serialization::pimpl::data &key);

            template<>
            boost::future<boost::optional<serialization::pimpl::data>>
            HAZELCAST_API SerializingProxy::invoke_and_get_future(protocol::ClientMessage &request, int partition_id);


            template<>
            boost::future<boost::optional<serialization::pimpl::data>>
            HAZELCAST_API SerializingProxy::invoke_and_get_future(protocol::ClientMessage &request,
                                                 const serialization::pimpl::data &key);

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
