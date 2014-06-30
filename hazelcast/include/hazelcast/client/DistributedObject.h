//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DistributedObject
#define HAZELCAST_DistributedObject

#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace impl {
            class BaseEventHandler;

            class BaseRemoveListenerRequest;
        }

        /**
         * Base class for all distributed objects.
         *
         * @see IMap
         * @see MultiMap
         * @see IQueue
         * @see IList
         * @see ISet
         * @see ITopic
         * @see ILock
         * @see ISemaphore
         * @see ICountDownLatch
         * @see IdGenerator
         * @see IAtomicLong
         * @see TransactionalMap
         * @see TransactionalQueue
         * @see TransactionalMultiMap
         * @see TransactionalSet
         * @see TransactionalList
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
            /**
             * Constructor.
             */
            DistributedObject(const std::string &serviceName, const std::string &objectName, spi::ClientContext *context);

            /**
             * @returns ClientContext.
             */
            spi::ClientContext &getContext();

            /**
             * method to be called when cluster-wide destroy method is called.
             */
            virtual void onDestroy() = 0;

            template<typename Response>
            /**
             * Internal API.
             * method to be called by distributed objects.
             * memory ownership is moved to DistributedObject.
             *
             * @param partitionId that given request will be send to.
             * @param request PortableRequest ptr.
             */
            boost::shared_ptr<Response> invoke(const impl::PortableRequest *request, int partitionId) {
                spi::InvocationService &invocationService = getContext().getInvocationService();
                boost::shared_ptr< util::Future<serialization::pimpl::Data> >  future = invocationService.invokeOnKeyOwner(request, partitionId);
                return context->getSerializationService().template toObject<Response>(future->get());
            };

            /**
             * Internal API.
             * method to be called by distributed objects.
             * memory ownership is moved to DistributedObject.
             *
             * @param request PortableRequest ptr.
             */
            template<typename Response>
            boost::shared_ptr<Response> invoke(const impl::PortableRequest *request) {
                boost::shared_ptr< util::Future<serialization::pimpl::Data> >  future = getContext().getInvocationService().invokeOnRandomTarget(request);
                return context->getSerializationService().template toObject<Response>(future->get());
            };

            /**
             * Internal API.
             *
             * @param registrationRequest PortableRequest ptr.
             * @param partitionId
             * @param handler
             */
            std::string listen(const impl::PortableRequest *registrationRequest, int partitionId, impl::BaseEventHandler *handler);

            /**
             * Internal API.
             *
             * @param registrationRequest PortableRequest ptr.
             * @param handler
             */
            std::string listen(const impl::PortableRequest *registrationRequest, impl::BaseEventHandler *handler);

            /**
             * Internal API.
             *
             * @param request PortableRequest ptr.
             * @param registrationId
             */
            bool stopListening(impl::BaseRemoveListenerRequest *request, const std::string &registrationId);

            /**
             * Internal API.
             * @param key
             */
            int getPartitionId(const serialization::pimpl::Data &key);

        private:
            const std::string name;
            const std::string serviceName;
            spi::ClientContext *context;
        };
    }
}

#endif //HAZELCAST_DistributedObject

