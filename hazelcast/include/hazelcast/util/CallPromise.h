//
// Created by sancar koyunlu on 03/01/14.
//




#ifndef HAZELCAST_ClientCallPromise
#define HAZELCAST_ClientCallPromise


#include "hazelcast/util/HazelcastDll.h"
#include <memory>
#include <boost/thread/future.hpp>

namespace hazelcast {
    namespace client {
        class Address;
        namespace impl {
            class PortableRequest;

            class EventHandlerWrapper;
        }
        namespace serialization {
            class Data;
        }
    }
    namespace util {
        class HAZELCAST_API CallPromise {
        public:

            void setResponse(const client::serialization::Data &data);

            void targetDisconnected(const client::Address &address);

            void setRequest(const client::impl::PortableRequest *request);

            const client::impl::PortableRequest &getRequest() const;

            boost::shared_future<client::serialization::Data> getFuture();

            void setEventHandler(client::impl::EventHandlerWrapper *eventHandler);

            client::impl::EventHandlerWrapper *getEventHandler() const;

        private:
            boost::promise<client::serialization::Data> promise;
            std::auto_ptr<const client::impl::PortableRequest> request;
            std::auto_ptr<client::impl::EventHandlerWrapper> eventHandler;
        };
    }
}

#endif //HAZELCAST_ClientCallPromise
