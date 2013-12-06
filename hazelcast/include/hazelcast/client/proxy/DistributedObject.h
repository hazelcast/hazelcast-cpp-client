//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DistributedObject
#define HAZELCAST_DistributedObject

#include <string>

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
        }
        namespace proxy {

            class HAZELCAST_API DistributedObject {
            public:
                DistributedObject(const std::string &serviceName, const std::string &objectName, spi::ClientContext *context);

                const std::string &getServiceName() const;

                const std::string &getName() const;

                spi::ClientContext &getContext();

                void destroy();

                virtual void onDestroy() = 0;

            private:
                const std::string serviceName;
                const std::string name;
                spi::ClientContext *context;
            };
        }
    }
}

#endif //HAZELCAST_DistributedObject
