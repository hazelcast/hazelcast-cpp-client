//
// Created by sancar koyunlu on 03/01/14.
//




#ifndef HAZELCAST_ClientCallPromise
#define HAZELCAST_ClientCallPromise


#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/util/Promise.h"
#include "hazelcast/util/AtomicInt.h"
#include <memory>

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
                    promise.getFuture()->set_exception(e);
                }

                void setRequest(std::auto_ptr<const impl::PortableRequest> request);

                const impl::PortableRequest &getRequest() const;

                boost::shared_ptr< util::Future<serialization::pimpl::Data> >  getFuture();

                void setEventHandler(std::auto_ptr<impl::BaseEventHandler> eventHandler);

                impl::BaseEventHandler *getEventHandler() const;

                int incrementAndGetResendCount();

            private:
                util::Promise<serialization::pimpl::Data> promise;
                std::auto_ptr<const impl::PortableRequest> request;
                std::auto_ptr<impl::BaseEventHandler> eventHandler;
                util::AtomicInt resendCount;
            };
        }
    }

}

#endif //HAZELCAST_ClientCallPromise
