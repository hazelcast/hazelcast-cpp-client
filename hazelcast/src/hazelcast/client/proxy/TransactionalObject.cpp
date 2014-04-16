//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/proxy/TransactionalObject.h"
#include "hazelcast/client/impl/ClientDestroyRequest.h"

namespace hazelcast {
    namespace client {
        namespace proxy {

            TransactionalObject::TransactionalObject(const std::string &serviceName, const std::string &objectName, txn::TransactionProxy *context)
            : serviceName(serviceName), name(objectName), context(context) {

            }

            TransactionalObject::~TransactionalObject() {

            }

            const std::string &TransactionalObject::getServiceName() {
                return serviceName;
            }

            const std::string &TransactionalObject::getName() {
                return name;
            }

            void TransactionalObject::destroy() {
                onDestroy();
                impl::ClientDestroyRequest *request = new impl::ClientDestroyRequest(name, serviceName);
                spi::InvocationService &invocationService = context->getInvocationService();
                invocationService.invokeOnConnection(request, context->getConnection());
            }
        }
    }
}

