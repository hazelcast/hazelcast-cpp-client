//
// Created by sancar koyunlu on 03/01/14.
//

#include "hazelcast/util/CallPromise.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/EventHandlerWrapper.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/exception/TargetDisconnectedException.h"

namespace hazelcast {
    namespace util {


        CallPromise::CallPromise(client::spi::ClusterService &clusterService)
        : clusterService(clusterService)
        , resendCount(0) {

        }

        void CallPromise::setResponse(const client::serialization::Data &data) {
            this->promise.set_value(data);
        }


        void CallPromise::setException(std::exception const &exception) {
            promise.set_exception(exception);
        }


        void CallPromise::targetDisconnected(const client::Address &address) {
            if (isRetryable(*request) || clusterService.isRedoOperation()) {
                if (resend())
                    return;
            }
            client::exception::TargetDisconnectedException targetDisconnectedException(address);
            promise.set_exception(targetDisconnectedException);
        }

        void CallPromise::targetIsNotAlive(const client::Address &address) {
            if (resend())
                return;
            client::exception::TargetDisconnectedException targetDisconnectedException(address);
            promise.set_exception(targetDisconnectedException);  // TargetNotMemberException

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


        bool CallPromise::resend() {
            resendCount++;
            if (resendCount > client::spi::ClusterService::RETRY_COUNT) {
                return false;
            }
            try {
                clusterService.resend(this);
            } catch(std::exception &e) {
                promise.set_exception(e);
            }
            return true;
        }
    }
}
