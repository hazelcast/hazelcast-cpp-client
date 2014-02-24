//
// Created by sancar koyunlu on 03/01/14.
//

#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/connection/CallPromise.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            CallPromise::CallPromise(spi::InvocationService &invocationService)
            : invocationService(invocationService)
            , resendCount(0) {
            }

            void CallPromise::setResponse(const serialization::pimpl::Data &data) {
                this->promise.set_value(data);
            }

            void CallPromise::setRequest(std::auto_ptr<const impl::PortableRequest> request) {
                this->request = request;
            }

            const impl::PortableRequest &CallPromise::getRequest() const {
                return *request;
            }

            boost::shared_future<serialization::pimpl::Data> CallPromise::getFuture() {
                return (boost::shared_future<serialization::pimpl::Data>) promise.get_future();
            }

            void CallPromise::setEventHandler(std::auto_ptr<impl::BaseEventHandler> eventHandler) {
                this->eventHandler = eventHandler;
            }

            impl::BaseEventHandler *CallPromise::getEventHandler() const {
                return eventHandler.get();
            }

            int CallPromise::incrementAndGetResendCount() {
                return ++resendCount;
            }
        }
    }
}
