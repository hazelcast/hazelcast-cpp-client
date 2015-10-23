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
// Created by sancar koyunlu on 12/11/13.



#include "hazelcast/client/proxy/TransactionalObject.h"
#include "hazelcast/client/impl/ClientDestroyRequest.h"
#include "hazelcast/client/txn/BaseTxnRequest.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalObject::TransactionalObject(const std::string& serviceName, const std::string& objectName, txn::TransactionProxy *context)
            : serviceName(serviceName), name(objectName), context(context) {

            }

            TransactionalObject::~TransactionalObject() {

            }

            const std::string& TransactionalObject::getServiceName() {
                return serviceName;
            }

            const std::string& TransactionalObject::getName() {
                return name;
            }

            void TransactionalObject::destroy() {
                onDestroy();
                impl::ClientDestroyRequest *request = new impl::ClientDestroyRequest(name, serviceName);
                spi::InvocationService& invocationService = context->getInvocationService();
                invocationService.invokeOnConnection(request, context->getConnection());
            }

            void TransactionalObject::onDestroy() {

            }

            serialization::pimpl::Data TransactionalObject::invoke(txn::BaseTxnRequest *request) {
                request->setTxnId(context->getTxnId());
                request->setThreadId(util::getThreadId());
                spi::InvocationService& invocationService = context->getInvocationService();
                connection::CallFuture future = invocationService.invokeOnConnection(request, context->getConnection());
                return future.get();
            }
        }
    }
}

