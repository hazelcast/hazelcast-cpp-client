//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DistributedObject
#define HAZELCAST_DistributedObject

#include "hazelcast/util/HazelcastDll.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#define DESERIALIZE(data, RETURN_TYPE) boost::shared_ptr<RETURN_TYPE> result = getContext().getSerializationService().toObject<RETURN_TYPE>(data);

namespace hazelcast {
    namespace client {
        namespace impl {
            class BaseEventHandler;

            class BaseRemoveListenerRequest;

            class ClientRequest;
        }

        namespace spi{
            class ClientContext;
        }

        namespace serialization{
            namespace pimpl{
                class Data;
            }
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

            /**
             * Internal API.
             * method to be called by distributed objects.
             * memory ownership is moved to DistributedObject.
             *
             * @param partitionId that given request will be send to.
             * @param request ClientRequest ptr.
             */
            serialization::pimpl::Data invoke(const impl::ClientRequest *request, int partitionId);

            /**
             * Internal API.
             * method to be called by distributed objects.
             * memory ownership is moved to DistributedObject.
             *
             * @param request ClientRequest ptr.
             */
            serialization::pimpl::Data invoke(const impl::ClientRequest *request);

            /**
             * Internal API.
             *
             * @param registrationRequest ClientRequest ptr.
             * @param partitionId
             * @param handler
             */
            std::string listen(const impl::ClientRequest *registrationRequest, int partitionId, impl::BaseEventHandler *handler);

            /**
             * Internal API.
             *
             * @param registrationRequest ClientRequest ptr.
             * @param handler
             */
            std::string listen(const impl::ClientRequest *registrationRequest, impl::BaseEventHandler *handler);

            /**
             * Internal API.
             *
             * @param request ClientRequest ptr.
             * @param registrationId
             */
            bool stopListening(impl::BaseRemoveListenerRequest *request, const std::string &registrationId);

            /**
             * Internal API.
             * @param key
             */
            int getPartitionId(const serialization::pimpl::Data &key);

        private:
            spi::ClientContext *context;
            const std::string name;
            const std::string serviceName;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_DistributedObject

