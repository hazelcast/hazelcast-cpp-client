//
// Created by sancar koyunlu on 03/01/14.
//

#include "hazelcast/util/CallPromise.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/exception/TargetDisconnectedException.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/EventHandlerWrapper.h"

namespace hazelcast {
    namespace util {

        void CallPromise::setResponse(const client::serialization::Data &data) {
            this->promise.set_value(data);
        }

        void CallPromise::targetDisconnected(const client::Address &address) {
            client::exception::TargetDisconnectedException targetDisconnectedException(address);
            promise.set_exception(targetDisconnectedException);
        }


        void CallPromise::setRequest(const client::impl::PortableRequest *request) {
            this->request.reset(request);
        }

        const client::impl::PortableRequest &CallPromise::getRequest() const {
            return *request;
        }

        boost::shared_future<client::serialization::Data> CallPromise::getFuture() {
            return (boost::shared_future<client::serialization::Data>) promise.get_future();
        }

        void CallPromise::setEventHandler(client::impl::EventHandlerWrapper *eventHandler) {
            this->eventHandler.reset(eventHandler);
        }

        client::impl::EventHandlerWrapper *CallPromise::getEventHandler() const {
            return eventHandler.get();
        }
    }
}
