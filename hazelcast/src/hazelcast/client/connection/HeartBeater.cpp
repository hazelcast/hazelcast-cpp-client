//
// Created by sancar koyunlu on 20/08/14.
//

#include "hazelcast/client/connection/HeartBeater.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/ThreadArgs.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/impl/ClientPingRequest.h"
#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/util/IOUtil.h"
#include <ctime>

namespace hazelcast {
    namespace client {
        namespace connection {

            HeartBeater::HeartBeater(spi::ClientContext& clientContext)
            : live(true)
            , clientContext(clientContext) {
                ClientProperties& properties = clientContext.getClientProperties();
                heartBeatTimeoutSeconds = properties.getHeartbeatTimeout().getInteger();
                heartBeatIntervalSeconds = properties.getHeartbeatInterval().getInteger();
                if (heartBeatTimeoutSeconds <= 0) {
                    heartBeatTimeoutSeconds = util::IOUtil::to_value<int>((std::string)ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT);
                }
                if (heartBeatIntervalSeconds <= 0) {
                    heartBeatIntervalSeconds = util::IOUtil::to_value<int>((std::string)ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT);
                }
            }

            void HeartBeater::staticStart(hazelcast::util::ThreadArgs& args) {
                HeartBeater *heartBeater = (HeartBeater *)args.arg0;
                heartBeater->run(args.currentThread);
            }

            void HeartBeater::run(hazelcast::util::Thread *currentThread) {
                currentThread->interruptibleSleep(heartBeatIntervalSeconds);
                spi::InvocationService& invocationService = clientContext.getInvocationService();
                connection::ConnectionManager& connectionManager = clientContext.getConnectionManager();
                while (live) {
                    std::vector<boost::shared_ptr<Connection> > connections = connectionManager.getConnections();
                    std::vector<boost::shared_ptr<Connection> >::iterator it;

                    time_t now = time(NULL);
                    for (it = connections.begin(); it != connections.end(); ++it) {
                        boost::shared_ptr<Connection> connection = *it;
                        if (now - connection->lastRead > heartBeatTimeoutSeconds) {
                            connection->heartBeatingFailed();
                        }

                        if (now - connection->lastRead > heartBeatIntervalSeconds) {
                            impl::ClientPingRequest *request = new impl::ClientPingRequest();
                            invocationService.invokeOnConnection(request, connection);
                        } else {
                            connection->heartBeatingSucceed();
                        }
                    }

                    currentThread->interruptibleSleep(heartBeatIntervalSeconds);
                }
            }

            void HeartBeater::shutdown() {
                live = false;
            }
        }
    }
}
