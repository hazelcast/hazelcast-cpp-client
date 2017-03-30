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
#ifndef HAZELCAST_CLIENT_INTERNAL_CONCURRENT_FUTUREIMPL_H_
#define HAZELCAST_CLIENT_INTERNAL_CONCURRENT_FUTUREIMPL_H_

#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <hazelcast/client/connection/CallFuture.h>
#include <hazelcast/client/serialization/pimpl/SerializationService.h>

#include "hazelcast/client/Future.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace concurrent {
                template <typename V, typename CODEC>
                class FutureImpl : public Future<V> {
                public:
                    FutureImpl(const connection::CallFuture &callFuture,
                               serialization::pimpl::SerializationService &serializationService) : callFuture(
                            callFuture), serializationService(serializationService) {
                    }

                    virtual ~FutureImpl() {
                    }

                    virtual boost::shared_ptr<V> get() {
                        return get(INT64_MAX);
                    }

                    virtual boost::shared_ptr<V> get(int64_t timeoutInMilliseconds) {
                        std::auto_ptr<protocol::ClientMessage> responseMsg = callFuture.get(timeoutInMilliseconds);

                        std::auto_ptr<serialization::pimpl::Data> response = CODEC::decode(*responseMsg).response;

                        return boost::shared_ptr<V>(serializationService.toObject<V>(response.get()));

                    }

                    virtual bool isDone() const {
                        return callFuture.isDone();
                    }
                private:
                    connection::CallFuture callFuture;
                    serialization::pimpl::SerializationService &serializationService;
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_INTERNAL_CONCURRENT_FUTUREIMPL_H_ */
