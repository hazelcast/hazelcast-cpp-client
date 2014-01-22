//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TransactionalObject
#define HAZELCAST_TransactionalObject

#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/util/HazelcastDll.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace txn {
            class TransactionProxy;
        }
        namespace proxy {

            class HAZELCAST_API TransactionalObject {
            public:
                TransactionalObject(const std::string &serviceName, const std::string &objectName, txn::TransactionProxy *context);

                const std::string &getServiceName();

                const std::string &getName();

                txn::TransactionProxy &getContext();

                void destroy();

                virtual void onDestroy() = 0;

            protected:
                template<typename T>
                serialization::Data toData(const T &object) {
                    return context->getSerializationService().template toData<T>(&object);
                };

                template<typename T>
                boost::shared_ptr<T> toObject(const serialization::Data &data) {
                    return context->getSerializationService().template toObject<T>(data);
                };

                template<typename Response >
                boost::shared_ptr<Response> invoke(const impl::PortableRequest *request) {
                    spi::InvocationService &invocationService = context->getInvocationService();
                    serialization::SerializationService &ss = context->getSerializationService();
                    boost::shared_future<serialization::Data> future = invocationService.invokeOnConnection(request, *(context->getConnection()));
                    return ss.toObject<Response>(future.get());
                };
            private:
                const std::string serviceName;
                const std::string name;
                txn::TransactionProxy *context;
            };
        }
    }
}


#endif //HAZELCAST_TransactionalObject
