//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DistributedObject
#define HAZELCAST_DistributedObject

#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/ClusterService.h"
#include <string>

namespace hazelcast {
    namespace client {
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
                boost::shared_ptr<Response> invoke(const impl::PortableRequest &request, serialization::Data &keyData) {
                    boost::shared_future<serialization::Data> future = getContext().getInvocationService().invokeOnKeyOwner(request, keyData);
                    return context->getSerializationService().toObject<Response>(future.get());
                };

                template<typename Response>
                boost::shared_ptr<Response> invoke(const impl::PortableRequest &request) {
                    boost::shared_future<serialization::Data> future = getContext().getInvocationService().invokeOnRandomTarget(request);
                    return context->getSerializationService().toObject<Response>(future.get());
                };

                std::string listen(const impl::PortableRequest &registrationRequest, const serialization::Data *partitionKey, impl::EventHandlerWrapper *handler) {
                    boost::shared_future<serialization::Data> future;
                    if (partitionKey == NULL) {
                        future = context->getInvocationService().invokeOnRandomTarget(registrationRequest, handler);
                    } else {
                        future = context->getInvocationService().invokeOnKeyOwner(registrationRequest, handler, *partitionKey);
                    }
                    boost::shared_ptr<std::string> registrationId = context->getSerializationService().toObject<std::string>(future.get());
                    context->getClusterService().registerListener(*registrationId, registrationRequest.callId);
                    return *registrationId;
                }

                std::string listen(const impl::PortableRequest &registrationRequest, impl::EventHandlerWrapper *handler) {
                    return listen(registrationRequest, NULL, handler);
                }

                bool stopListening(const impl::PortableRequest &request, const std::string &registrationId) {
//                    return ListenerUtil.stopListening(context, request, registrationId); TODO
                    return true;
                }

            private:
                const std::string serviceName;
                const std::string name;
                spi::ClientContext *context;
            };
        }
    }
}

#endif //HAZELCAST_DistributedObject
