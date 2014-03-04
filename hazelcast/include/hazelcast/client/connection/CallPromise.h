//
// Created by sancar koyunlu on 03/01/14.
//




#ifndef HAZELCAST_ClientCallPromise
#define HAZELCAST_ClientCallPromise


#include "hazelcast/util/HazelcastDll.h"
#include <memory>
#include <boost/thread/future.hpp>
#include <boost/atomic.hpp>

namespace hazelcast {

    namespace client {
        class Address;
        namespace impl {
            class PortableRequest;

            class BaseEventHandler;
        }
        namespace serialization {
            namespace pimpl{
                class Data;
            }
        }

        namespace spi {
            class InvocationService;
        }

        namespace connection {
            class HAZELCAST_API CallPromise {
            public:
                CallPromise();

                void setResponse(const serialization::pimpl::Data &data);

                template<typename E>
                void setException(E const &e) {
                    promise.set_exception(boost::copy_exception(e));
                }

                void setRequest(std::auto_ptr<const impl::PortableRequest> request);

                const impl::PortableRequest &getRequest() const;

                boost::shared_future<serialization::pimpl::Data> getFuture();

                void setEventHandler(std::auto_ptr<impl::BaseEventHandler> eventHandler);

                impl::BaseEventHandler *getEventHandler() const;

                int incrementAndGetResendCount();

            private:
                boost::promise<serialization::pimpl::Data> promise;
                std::auto_ptr<const impl::PortableRequest> request;
                std::auto_ptr<impl::BaseEventHandler> eventHandler;
                boost::atomic<int> resendCount;
            };
        }
    }

}

#endif //HAZELCAST_ClientCallPromise
