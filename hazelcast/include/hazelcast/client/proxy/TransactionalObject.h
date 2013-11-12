//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TransactionalObject
#define HAZELCAST_TransactionalObject

#include <string>

namespace hazelcast {
    namespace client {
        namespace txn {
            class TransactionProxy;
        }
        namespace proxy {

            class TransactionalObject {
            public:
                TransactionalObject(const std::string &serviceName, const std::string &objectName, txn::TransactionProxy *context);

                const std::string &getServiceName();

                const std::string &getName();

                txn::TransactionProxy &getContext();

                void destroy();

                virtual void onDestroy() = 0;

            private:
                const std::string serviceName;
                const std::string name;
                txn::TransactionProxy *context;
            };
        }
    }
}


#endif //HAZELCAST_TransactionalObject
