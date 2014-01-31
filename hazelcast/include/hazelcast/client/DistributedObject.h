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

        /**
         * Base interface for all distributed objects.
         *
         * @see IMap
         * @see IQueue
         * @see MultiMap
         * @see ITopic
         * @see ILock
         * @see IExecutorService
         * @see TransactionalMap
         * @see TransactionalQueue
         * @see TransactionalMultiMap
         */
        class HAZELCAST_API DistributedObject {
            friend class HazelcastClient;

        public:
            /**
             * Returns the service name for this object.
             */
            const std::string &getServiceName() const;

            /**
             * Returns the unique name for this DistributedObject.
             *
             * @return the unique name for this object.
             */
            const std::string &getName() const;

            /**
             * Destroys this object cluster-wide.
             * Clears and releases all resources for this object.
             */
            void destroy();

            /**
            * Destructor
            */
            virtual ~DistributedObject();

        protected:
            DistributedObject(const std::string &serviceName, const std::string &objectName, spi::ClientContext *context);

            spi::ClientContext &getContext();

            virtual void onDestroy() = 0;

            template<typename Response>
            boost::shared_ptr<Response> invoke(const impl::PortableRequest *request, int partitionId) {
                spi::InvocationService &invocationService = getContext().getInvocationService();
                boost::shared_future<serialization::Data> future = invocationService.invokeOnKeyOwner(request, partitionId);
                return context->getSerializationService().toObject<Response>(future.get());
            };

            template<typename Response>
            boost::shared_ptr<Response> invoke(const impl::PortableRequest *request) {
                boost::shared_future<serialization::Data> future = getContext().getInvocationService().invokeOnRandomTarget(request);
                return context->getSerializationService().toObject<Response>(future.get());
            };

            std::string listen(const impl::PortableRequest *registrationRequest, int partitionId, impl::BaseEventHandler *handler);

            std::string listen(const impl::PortableRequest *registrationRequest, impl::BaseEventHandler *handler);

            bool stopListening(const impl::PortableRequest *request, const std::string &registrationId);

            int getPartitionId(const serialization::Data &key);

        private:
            const std::string serviceName;
            const std::string name;
            spi::ClientContext *context;
        };
    }
}

#endif //HAZELCAST_DistributedObject
