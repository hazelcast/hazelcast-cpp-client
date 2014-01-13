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

        namespace spi {
            class InvocationService;
        }
    }
    namespace util {
        class HAZELCAST_API CallPromise {
        public:
            CallPromise(client::spi::InvocationService &invocationService);

            void setResponse(const client::serialization::Data &data);

            void setException(std::exception const &);

            void setRequest(const client::impl::PortableRequest *request);

            const client::impl::PortableRequest &getRequest() const;

            boost::shared_future<client::serialization::Data> getFuture();

            void setEventHandler(client::impl::EventHandlerWrapper *eventHandler);

            client::impl::EventHandlerWrapper *getEventHandler() const;

            int incrementAndGetResendCount();

        private:
            client::spi::InvocationService &invocationService;
            boost::promise<client::serialization::Data> promise;
            std::auto_ptr<const client::impl::PortableRequest> request;
            std::auto_ptr<client::impl::EventHandlerWrapper> eventHandler;
            int resendCount;
        };
    }
}

#endif //HAZELCAST_ClientCallPromise
