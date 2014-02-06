#ifndef HAZELCAST_CLIENT_CONFIG
#define HAZELCAST_CLIENT_CONFIG

#include "hazelcast/client/Address.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/impl/RoundRobinLB.h"
#include <vector>
#include <set>
#include <memory>

namespace hazelcast {
    namespace client {
        class MembershipListener;

        class InitialMembershipListener;

        class LifecycleListener;

        class SocketInterceptor;

        /**
         * HazelcastClient configuration class.
         */
        class HAZELCAST_API ClientConfig {
        public:

            /**
             * Constructor with default values.
             * smart(true)
             * redoOperation(false)
             * connectionTimeout(60000)
             * connectionAttemptLimit(2)
             * attemptPeriod(3000)
             * defaultLoadBalancer(impl::RoundRobinLB)
             */
            ClientConfig();

            /**
             * Destructor
             */
            ~ClientConfig();

            /**
             * Adds an address to list of the initial addresses.
             * Client will use this list to find a running Member, connect to it.
             *
             * @param address
             * @return itself ClientConfig
             */
            ClientConfig &addAddress(const Address &address);

            /**
             * Adds all address in given vector to list of the initial addresses.
             * Client will use this list to find a running Member, connect to it.
             *
             * @param addresses vector of addresses
             * @return itself ClientConfig
            */
            ClientConfig &addAddresses(const std::vector<Address> &addresses);

            /**
             * Returns vector of the initial addresses.
             * Client will use this vector to find a running Member, connect to it.
             *
             * @param address
             * @return vector of addresses
             */
            std::vector<Address> &getAddresses();

            /**
             * The Group Configuration properties like:
             * Name and Password that is used to connect to the cluster.
             *
             * @param groupConfig
             * @return itself ClientConfig
             */
            ClientConfig &setGroupConfig(GroupConfig &groupConfig);

            /**
             *
             * @return groupConfig
             */
            GroupConfig &getGroupConfig();

            /**
             * Can be used instead of GroupConfig in Hazelcast EE.
             */
            void setCredentials(Credentials *credentials);

            /**
             * Can be used instead of GroupConfig in Hazelcast EE.
             */
            Credentials &getCredentials();

            /**
             * While client is trying to connect initially to one of the members in the ClientConfig#addressList,
             * all might be not available. Instead of giving up, throwing Exception and stopping client, it will
             * attempt to retry as much as ClientConfig#connectionAttemptLimit times.
             *
             * @param connectionAttemptLimit
             * @return itself ClientConfig
             */
            ClientConfig &setConnectionAttemptLimit(int connectionAttemptLimit);

            /**
             * While client is trying to connect initially to one of the members in the ClientConfig#addressList,
             * all might be not available. Instead of giving up, throwing Exception and stopping client, it will
             * attempt to retry as much as ClientConfig#connectionAttemptLimit times.
             *
             * return int connectionAttemptLimit
             */
            int getConnectionAttemptLimit() const;

            /**
             * Client will be sending heartbeat messages to members and this is the timeout. If there is no any message
             * passing between client and member within the ClientConfig#connectionTimeout milliseconds the connection
             * will be closed.
             *
             * @param int connectionTimeoutInMillis
             * @return itself ClientConfig
             */
            ClientConfig &setConnectionTimeout(int connectionTimeoutInMillis);

            /**
            * Client will be sending heartbeat messages to members and this is the timeout. If there is no any message
            * passing between client and member within the ClientConfig#connectionTimeout milliseconds the connection
            * will be closed.
            *
            * @return int connectionTimeoutInMillis
            */
            int getConnectionTimeout() const;

            /**
             * Period for the next attempt to find a member to connect. (see ClientConfig#connectionAttemptLimit ).
             *
             * @param int attemptPeriodInMillis
             * @return itself ClientConfig
             */
            ClientConfig &setAttemptPeriod(int attemptPeriodInMillis);

            /**
             * Period for the next attempt to find a member to connect. (see ClientConfig#connectionAttemptLimit ).
             *
             * @return int attemptPeriodInMillis
             */
            int getAttemptPeriod() const;

            /**
             * If true, client will redo the operations that were executing on the server and client lost the connection.
             * This can be because of network, or simply because the member died. However it is not clear whether the
             * application is performed or not. For idempotent operations this is harmless, but for non idempotent ones
             * retrying can cause to undesirable effects. Note that the redo can perform on any member.
             *
             * If false, the operation will throw IOException.
             *
             * @param bool redoOperation
             * return itself ClientConfig
             */
            ClientConfig &setRedoOperation(bool redoOperation);

            /**
             *
             * see setRedoOperation
             * returns redoOperation
             */
            bool isRedoOperation() const;

            /**
             * @return true if client configured as smart
             * see setSmart()
             */
            bool isSmart() const;

            /**
             * If true, client will route the key based operations to owner of the key at the best effort.
             * Note that it uses a cached version of PartitionService#getPartitions() and doesn't
             * guarantee that the operation will always be executed on the owner.
             * The cached table is updated every 10 seconds.
             *
             * @param smart
             */
            void setSmart(bool smart);

            /**
             * Will be called with the Socket, each time client creates a connection to any Member.
             */
            void setSocketInterceptor(SocketInterceptor *socketInterceptor);

            /**
             * Will be called with the Socket, each time client creates a connection to any Member.
             */
            std::auto_ptr<SocketInterceptor> getSocketInterceptor();

            /**
             * Adds a listener to configuration to be registered when HazelcastClient starts.
             *
             * @param listener LifecycleListener *listener
             * @return itself ClientConfig
             */
            ClientConfig &addListener(LifecycleListener *listener);

            /**
             *
             * @return registered lifecycleListeners
             */
            const std::set<LifecycleListener *> &getLifecycleListeners() const;

            /**
             * Adds a listener to configuration to be registered when HazelcastClient starts.
             *
             * @param listener MembershipListener *listener
             * @return itself ClientConfig
             */
            ClientConfig &addListener(MembershipListener *listener);

            /**
             * Returns registered membershipListeners
             *
             * @return registered membershipListeners
             */
            const std::set<MembershipListener *> &getMembershipListeners() const;

            /**
             * Adds a listener to configuration to be registered when HazelcastClient starts.
             *
             * @param listener InitialMembershipListener *listener
             * @return itself ClientConfig
             */
            ClientConfig &addListener(InitialMembershipListener *listener);

            /**
             * Returns registered initialMembershipListeners
             *
             * @return registered initialMembershipListeners
             */
            const std::set<InitialMembershipListener *> &getInitialMembershipListeners() const;

            /**
             * Used to distribute the operations to multiple Endpoints.
             *
             * @return loadBalancer
             */
            LoadBalancer *const getLoadBalancer();

            /**
             * Used to distribute the operations to multiple Endpoints.
             * If not set, RoundRobin based load balancer is used
             *
             * @param LoadBalancer
             */
            void setLoadBalancer(LoadBalancer *loadBalancer);


        private:

            GroupConfig groupConfig;

            std::vector<Address> addressList;

            LoadBalancer *loadBalancer;

            std::auto_ptr<impl::RoundRobinLB> defaultLoadBalancer;

            std::set<MembershipListener *> membershipListeners;

            std::set<InitialMembershipListener *> initialMembershipListeners;

            std::set<LifecycleListener *> lifecycleListeners;

            bool smart;

            bool redoOperation;

            int connectionTimeout;

            int connectionAttemptLimit;

            int attemptPeriod;

            std::auto_ptr<SocketInterceptor> socketInterceptor;

            Credentials *credentials;

        };

    }
}
#endif /* HAZELCAST_CLIENT_CONFIG */