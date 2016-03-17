/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 03/01/14.
//
#ifndef HAZELCAST_ClientCallPromise
#define HAZELCAST_ClientCallPromise

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/util/AtomicInt.h"

#include <memory>

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;
        }
        namespace impl {
            class BaseEventHandler;
        };
        namespace connection {
            class CallPromise {
            public:
                CallPromise();

                void setResponse(std::auto_ptr<protocol::ClientMessage> message);

                void setException(const std::string& exceptionName, const std::string& exceptionDetails);

                void setRequest(std::auto_ptr<protocol::ClientMessage> request);

                protocol::ClientMessage *getRequest() const;

                util::Future<std::auto_ptr<protocol::ClientMessage> > &getFuture();

                void setEventHandler(std::auto_ptr<impl::BaseEventHandler> eventHandler);

                impl::BaseEventHandler *getEventHandler() const;

                int incrementAndGetResendCount();

                void resetFuture();
            private:
                util::Future<std::auto_ptr<protocol::ClientMessage> > future;
                std::auto_ptr<protocol::ClientMessage> request;
                std::auto_ptr<impl::BaseEventHandler> eventHandler;
                util::AtomicInt resendCount;
            };
        }
    }

}

#endif //HAZELCAST_ClientCallPromise

