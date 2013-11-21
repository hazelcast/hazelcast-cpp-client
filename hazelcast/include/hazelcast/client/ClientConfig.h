#ifndef HAZELCAST_CLIENT_CONFIG
#define HAZELCAST_CLIENT_CONFIG

#include "hazelcast/client/Address.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/protocol/Credentials.h"
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/impl/RoundRobinLB.h"
#include "hazelcast/client/spi/EventListener.h"
#include <vector>
#include <set>
#include <memory>

namespace hazelcast {
    namespace client {

        namespace connection {

            class SocketInterceptor;

        }
        class ClientConfig {
        public:

            ClientConfig();

            ~ClientConfig();

            ClientConfig &addAddress(const Address &address);

            ClientConfig &addAddresses(const std::vector<Address> &addresses);

            std::vector<Address> &getAddresses();

            ClientConfig &setGroupConfig(GroupConfig &groupConfig);

            GroupConfig &getGroupConfig();

            void setCredentials(protocol::Credentials *credentials);

            protocol::Credentials &getCredentials();

            ClientConfig &setConnectionAttemptLimit(int connectionAttemptLimit);

            int getConnectionAttemptLimit() const;

            ClientConfig &setConnectionTimeout(int connectionTimeoutInMillis);

            int getConnectionTimeout() const;

            ClientConfig &setAttemptPeriod(int attemptPeriodInMillis);

            int getAttemptPeriod() const;

            ClientConfig &setRedoOperation(bool redoOperation);

            bool isRedoOperation() const;

            void setSocketInterceptor(connection::SocketInterceptor *socketInterceptor);

            bool isSmart() const;

            void setSmart(bool smart);

            std::auto_ptr<connection::SocketInterceptor> getSocketInterceptor();

            /**
             * Adds a listener object to configuration to be registered when {@code HazelcastClient} starts.
             *
             * @param listener one of {@link com.hazelcast.core.LifecycleListener}, {@link com.hazelcast.core.DistributedObjectListener}
             *                 or {@link com.hazelcast.core.MembershipListener}
             * @return
            */
            ClientConfig &addListener(spi::EventListener *listener);

            std::set<spi::EventListener *> getListeners() const;

            LoadBalancer *const getLoadBalancer();

            void setLoadBalancer(LoadBalancer *loadBalancer);


        private:

            /**
             * The Group Configuration properties like:
             * Name and Password that is used to connect to the cluster.
             */

            GroupConfig groupConfig;


            /**
             * List of the initial set of addresses.
             * Client will use this list to find a running Member, connect to it.
             */
            std::vector<Address> addressList;

            /**
             * Used to distribute the operations to multiple Endpoints.
             */
            LoadBalancer *loadBalancer;

            std::auto_ptr<impl::RoundRobinLB> defaultLoadBalancer;

            /**
             * List of listeners that Hazelcast will automatically add as a part of initialization process.
             * Currently only supports {@link com.hazelcast.core.LifecycleListener}.
             */
            std::set<spi::EventListener *> listeners;
            /**
             * If true, client will route the key based operations to owner of the key at the best effort.
             * Note that it uses a cached version of {@link com.hazelcast.core.PartitionService#getPartitions()} and doesn't
             * guarantee that the operation will always be executed on the owner. The cached table is updated every second.
             */
            bool smart;

            /**
             * If true, client will redo the operations that were executing on the server and client lost the connection.
             * This can be because of network, or simply because the member died. However it is not clear whether the
             * application is performed or not. For idempotent operations this is harmless, but for non idempotent ones
             * retrying can cause to undesirable effects. Note that the redo can perform on any member.
             * <p/>
             * If false, the operation will throw {@link RuntimeException} that is wrapping {@link java.io.IOException}.
             */
            bool redoOperation;

            /**
             * limit for the Pool size that is used to pool the connections to the members.
             */
            int poolSize;

            /**
             * Client will be sending heartbeat messages to members and this is the timeout. If there is no any message
             * passing between client and member within the {@link ClientConfig#connectionTimeout} milliseconds the connection
             * will be closed.
             */
            int connectionTimeout;

            /**
             * While client is trying to connect initially to one of the members in the {@link ClientConfig#addressList},
             * all might be not available. Instead of giving up, throwing Exception and stopping client, it will
             * attempt to retry as much as {@link ClientConfig#connectionAttemptLimit} times.
             */
            int connectionAttemptLimit;

            /**
             * Period for the next attempt to find a member to connect. (see {@link ClientConfig#connectionAttemptLimit}).
             */
            int attemptPeriod;


//            SocketOptions socketOptions;


//             ProxyFactoryConfig proxyFactoryConfig ;

            /**
             * Will be called with the Socket, each time client creates a connection to any Member.
             */
            std::auto_ptr<connection::SocketInterceptor> socketInterceptor;

            /**
             * Can be used instead of {@link GroupConfig} in Hazelcast EE.
             */
            protocol::Credentials *credentials;

            protocol::Credentials defaultCredentials;

        };

    }
}
#endif /* HAZELCAST_CLIENT_CONFIG */