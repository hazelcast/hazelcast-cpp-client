//
// Created by sancar koyunlu on 03/01/14.
//




#ifndef HAZELCAST_ClientCallPromise
#define HAZELCAST_ClientCallPromise


#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include <memory>

namespace hazelcast {

    namespace client {

        class Address;

        namespace impl {

            class ServerException;
        }
        namespace serialization {

            namespace pimpl {

                class Data;

            }
        }

        namespace spi {

            class InvocationService;

        }

        namespace connection {
            class CallPromise {
            public:
                CallPromise();

                void setResponse(const serialization::pimpl::Data& data);

                void setException(const std::string& exceptionName, const std::string& exceptionDetails);

                void setRequest(std::auto_ptr<const impl::ClientRequest> request);

                const impl::ClientRequest& getRequest() const;

                boost::shared_ptr<util::Future<serialization::pimpl::Data> > getFuture();

                void setEventHandler(std::auto_ptr<impl::BaseEventHandler> eventHandler);

                impl::BaseEventHandler *getEventHandler() const;

                int incrementAndGetResendCount();

            private:
                boost::shared_ptr<util::Future<serialization::pimpl::Data> > future;
                std::auto_ptr<const impl::ClientRequest> request;
                std::auto_ptr<impl::BaseEventHandler> eventHandler;
                util::AtomicInt resendCount;
            };
        }
    }

}

#endif //HAZELCAST_ClientCallPromise

