//
// Created by sancar koyunlu on 21/08/14.
//

#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/InvocationService.h"
#include <climits>

namespace hazelcast {
    namespace client {
        namespace connection {


            CallFuture::CallFuture()
            : invocationService(NULL)
            , heartBeatTimeout(0) {

            }

            CallFuture::CallFuture(boost::shared_ptr<CallPromise> promise, boost::shared_ptr<Connection> connection, int heartBeatTimeout, spi::InvocationService *invocationService)
            : promise(promise)
            , connection(connection)
            , invocationService(invocationService)
            , heartBeatTimeout(heartBeatTimeout) {

            }

            serialization::pimpl::Data CallFuture::get() {
                return get(INT_MAX);
            }

            serialization::pimpl::Data CallFuture::get(int timeoutInSeconds) {
                while (timeoutInSeconds > 0) {
                    time_t beg = time(NULL);
                    try {
                        int min = std::min(timeoutInSeconds, heartBeatTimeout);
                        return promise->getFuture()->get(min);
                    } catch (exception::TimeoutException& exception) {
                        if (!connection->isHeartBeating()) {
                            std::string address = util::IOUtil::to_string(connection->getRemoteEndpoint());
                            invocationService->tryResend(promise, address);
                        }
                    }
                    time_t elapsed = time(NULL) - beg;
                    timeoutInSeconds -= elapsed;
                }
                throw exception::TimeoutException("CallFuture::get(int timeoutInSeconds)", "Wait is timed out");
            }

        }
    }
}

