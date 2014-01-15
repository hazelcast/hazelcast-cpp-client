//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DistributedObject
#define HAZELCAST_DistributedObject

#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include <string>

namespace hazelcast {
    namespace client {

        namespace impl {
            class BaseEventHandler;
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

                virtual ~DistributedObject();

            protected:
                template<typename Response>
                boost::shared_ptr<Response> invoke(const impl::PortableRequest *request, serialization::Data &keyData) {
                    boost::shared_future<serialization::Data> future = getContext().getInvocationService().invokeOnKeyOwner(request, keyData);
                    return context->getSerializationService().toObject<Response>(future.get());
                };

                template<typename Response>
                boost::shared_ptr<Response> invoke(const impl::PortableRequest *request) {
                    boost::shared_future<serialization::Data> future = getContext().getInvocationService().invokeOnRandomTarget(request);
                    return context->getSerializationService().toObject<Response>(future.get());
                };

                std::string listen(const impl::PortableRequest *registrationRequest, const serialization::Data *partitionKey, impl::BaseEventHandler *handler);

                std::string listen(const impl::PortableRequest *registrationRequest, impl::BaseEventHandler *handler);

                bool stopListening(const impl::PortableRequest *request, const std::string &registrationId);

            private:
                const std::string serviceName;
                const std::string name;
                spi::ClientContext *context;
            };
        }
    }
}

#endif //HAZELCAST_DistributedObject
