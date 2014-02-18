//
// Created by sancar koyunlu on 03/01/14.
//

#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/util/CallPromise.h"

namespace hazelcast {
    namespace util {
        CallPromise::CallPromise(client::spi::InvocationService &invocationService)
        : invocationService(invocationService)
        , resendCount(0) {
        }

        void CallPromise::setResponse(const client::serialization::Data &data) {
            this->promise.set_value(data);
        }

        void CallPromise::setRequest(std::auto_ptr<const client::impl::PortableRequest>request) {
            this->request = request;
        }

        const client::impl::PortableRequest &CallPromise::getRequest() const {
            return *request;
        }

        boost::shared_future<client::serialization::Data> CallPromise::getFuture() {
            return (boost::shared_future<client::serialization::Data>) promise.get_future();
        }

        void CallPromise::setEventHandler(std::auto_ptr<client::impl::BaseEventHandler> eventHandler) {
            this->eventHandler = eventHandler;
        }

        client::impl::BaseEventHandler *CallPromise::getEventHandler() const {
            return eventHandler.get();
        }

        int CallPromise::incrementAndGetResendCount() {
            return ++resendCount;
        }
    }
}
