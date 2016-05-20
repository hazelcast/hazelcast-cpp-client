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
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            CallPromise::CallPromise()
            : resendCount(0) {
            }

            void CallPromise::setResponse(std::auto_ptr<protocol::ClientMessage> message) {
                this->future.set_value(message);
            }

            void CallPromise::setException(std::auto_ptr<exception::IException> exception) {
                future.set_exception(exception);
            }

            void CallPromise::setRequest(std::auto_ptr<protocol::ClientMessage> request) {
                this->request = request;
            }

            protocol::ClientMessage *CallPromise::getRequest() const {
                return request.get();
            }

            util::Future<std::auto_ptr<protocol::ClientMessage> >  &CallPromise::getFuture() {
                return future;
            }

            void CallPromise::setEventHandler(std::auto_ptr<impl::BaseEventHandler> eventHandler) {
                this->eventHandler = eventHandler;
            }

            impl::BaseEventHandler *CallPromise::getEventHandler() const {
                return eventHandler.get();
            }

            int CallPromise::incrementAndGetResendCount() {
                return ++resendCount;
            }

            void CallPromise::resetFuture() {
                future.reset();
            }
        }
    }
}

