/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <boost/utility/string_view.hpp>

#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /*
         * You can use native C++ Client to connect to hazelcast nodes and make almost all operations that a node does.
         * Different from nodes, clients do not hold data.
         *
         * Some of features of C++ Clients are:
         * * Access to distributed data structures like IMap, IQueue, MultiMap, ITopic etc... For complete list see the classes
         * extending DistributedObject
         * * Access to transactional distributed data structures like TransactionalMap, TransactionalQueue etc...
         * * Ability to add cluster listeners to a cluster and entry/item listeners to distributed data structures.
         * @see MembershipListener, IMap#addEntryListener , IQueue#addItemListener etc .
         * * C++ Client is smart by default, which means that it knows where the data is and asks directly to correct node.
         * Note that you can turn this feature off ( ClientConfig#setSmart), if you don't want your clients to connect every
         * node.
         *
         * Our C++ client is completely open source and the source code is freely available at https://github.com/hazelcast/hazelcast-cpp-client .
         * Please feel free to contribute. You can join our community at https://groups.google.com/forum/#!forum/hazelcast where
         * you can find answers to your questions.
         */
        class HAZELCAST_API HazelcastClient {
            friend class spi::ClientContext;
        public:
            /**
            * Constructs a hazelcastClient with default configurations.
            */
            HazelcastClient();
            
            /**
            * Constructs a hazelcastClient with given ClientConfig.
            * Note: ClientConfig will be copied.
            * @param config client configuration to start the client with
            */
            explicit HazelcastClient(ClientConfig config);

            virtual ~HazelcastClient();

            /**
             * Returns the name of this Hazelcast instance.
             *
             * @return name of this Hazelcast instance
             */
            const std::string &getName() const;

            /**
            *
            * @tparam T type of the distributed object
            * @param name name of the distributed object.
            * @returns distributed object
            */
            template<typename T>
            std::shared_ptr<T> getDistributedObject(const std::string& name) {
                return clientImpl->getDistributedObject<T>(name);
            }

            /**
            *
            * Returns the distributed map instance with the specified name.
            *
            * @tparam K key type
            * @tparam V value type
            * @param name name of the distributed map
            * @return distributed map instance with the specified name
            */
            std::shared_ptr<IMap> getMap(const std::string &name) {
                return clientImpl->getDistributedObject<IMap>(name);
            }

            /**
            * Returns the distributed multimap instance with the specified name.
            *
            * @param name name of the distributed multimap
            * @return distributed multimap instance with the specified name
            */
            std::shared_ptr<MultiMap> getMultiMap(const std::string& name) {
                return clientImpl->getDistributedObject<MultiMap>(name);
            }

            std::shared_ptr<ReplicatedMap> getReplicatedMap(const std::string &name) {
                return clientImpl->getDistributedObject<ReplicatedMap>(name);
            }

            /**
            * Returns the distributed queue instance with the specified name and entry type E.
            *
            * @param name name of the distributed queue
            * @return distributed queue instance with the specified name
            */
            std::shared_ptr<IQueue> getQueue(const std::string& name) {
                return clientImpl->getDistributedObject<IQueue>(name);
            }

            /**
            * Returns the distributed set instance with the specified name and entry type E.
            * Set is ordered unique set of entries. similar to std::unordered_set
            *
            * @param name name of the distributed set
            * @return distributed set instance with the specified name
            */
            std::shared_ptr<ISet> getSet(const std::string& name) {
                return clientImpl->getDistributedObject<ISet>(name);
            }

            /**
            * Returns the distributed list instance with the specified name.
            * List is ordered set of entries. similar to std::vector
            *
            * @param name name of the distributed list
            * @return distributed list instance with the specified name
            */
            std::shared_ptr<IList> getList(const std::string& name) {
                return clientImpl->getDistributedObject<IList>(name);
            }

            /**
            * Returns the distributed topic instance with the specified name and entry type E.
            *
            * @param name name of the distributed topic
            * @return distributed topic instance with the specified name
            */
            std::shared_ptr<ITopic> getTopic(const std::string& name) {
                return clientImpl->getDistributedObject<ITopic>(name);
            };

            /**
            * Returns the distributed topic instance with the specified name and entry type E.
            *
            * @param name name of the distributed topic
            * @return distributed topic instance with the specified name
            */
            std::shared_ptr<ReliableTopic> getReliableTopic(const std::string& name) {
                return clientImpl->getDistributedObject<ReliableTopic>(name);
            }

            /**
             * Returns a generator that creates a cluster-wide unique IDs. Generated IDs are {@code long}
             * primitive values and are k-ordered (roughly ordered). IDs are in the range from {@code 0} to {@code
             * Long.MAX_VALUE}.
             * <p>
             * The IDs contain timestamp component and a node ID component, which is assigned when the member
             * joins the cluster. This allows the IDs to be ordered and unique without any coordination between
             * members, which makes the generator safe even in split-brain scenario (for caveats,
             * {@link com.hazelcast.internal.cluster.ClusterService#getMemberListJoinVersion() see here}).
             * <p>
             * For more details and caveats, see class documentation for {@link FlakeIdGenerator}.
             * <p>
             *
             * @param name name of the {@link FlakeIdGenerator}
             * @return FlakeIdGenerator for the given name
             */
            std::shared_ptr<FlakeIdGenerator> getFlakeIdGenerator(const std::string& name) {
                return clientImpl->getDistributedObject<FlakeIdGenerator>(name);
            }

            /**
             * Obtain a {@link com.hazelcast.crdt.pncounter.PNCounter} with the given
             * name.
             * <p>
             * The PN counter can be used as a counter with strong eventual consistency
             * guarantees - if operations to the counters stop, the counter values
             * of all replicas that can communicate with each other should eventually
             * converge to the same value.
             *
             * @param name the name of the PN counter
             * @return a {@link PNCounter}
             */
            std::shared_ptr<PNCounter> getPNCounter(const std::string& name) {
                return clientImpl->getDistributedObject<PNCounter>(name);
            }

            /**
             * Returns the distributed Ringbuffer instance with the specified name.
             *
             * @param name name of the distributed Ringbuffer
             * @return distributed RingBuffer instance with the specified name
             */
            std::shared_ptr<Ringbuffer> getRingbuffer(const std::string& name) {
                return clientImpl->getDistributedObject<Ringbuffer>(name);
            }

            /**
             * Creates or returns the distributed executor service for the given name.
             * Executor service enables you to run your <tt>Runnable</tt>s and <tt>Callable</tt>s
             * on the Hazelcast cluster.
             * <p>
             * <p><b>Note:</b> Note that it doesn't support {@code invokeAll/Any}
             * and doesn't have standard shutdown behavior</p>
             *
             * @param name name of the executor service
             * @return the distributed executor service for the given name
             */
            std::shared_ptr<IExecutorService> getExecutorService(const std::string &name) {
                return clientImpl->getDistributedObject<IExecutorService>(name);
            }

            /**
            *
            * @return configuration of this Hazelcast client.
            */
            ClientConfig& getClientConfig();

            /**
            * Creates a new TransactionContext associated with the current thread using default options.
            *
            * @return new TransactionContext
            */
            TransactionContext newTransactionContext();

            /**
            * Creates a new TransactionContext associated with the current thread with given options.
            *
            * @param options options for this transaction
            * @return new TransactionContext
            */
            TransactionContext newTransactionContext(const TransactionOptions& options);

            /**
            * Returns the Cluster that connected Hazelcast instance is a part of.
            * Cluster interface allows you to add listener for membership
            * events and learn more about the cluster.
            *
            * @return cluster
            */
            Cluster& getCluster();

            /**
             * Returns the local endpoint which this HazelcastInstance belongs to.
             * <p>
             *
             * @return the local enpoint which this client belongs to
             * @see Client
             */
            Client getLocalEndpoint() const;

            /**
            * Add listener to listen lifecycle events.
            *
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * @param lifecycleListener Listener object
            */
            boost::uuids::uuid addLifecycleListener(LifecycleListener &&lifecycleListener);

            /**
            * Remove lifecycle listener
            * @param lifecycleListener
            * @return true if removed successfully
            */
            bool removeLifecycleListener(const boost::uuids::uuid &registrationId);

            /**
            * Shuts down this HazelcastClient.
            */
            void shutdown();

            /**
             * Returns the lifecycle service for this instance.
             * <p>
             * LifecycleService allows you to shutdown this HazelcastInstance and listen for the lifecycle events.
             *
             * @return the lifecycle service for this instance
             */
            spi::LifecycleService &getLifecycleService();

            /**
             *
             * @return the CP subsystem that offers a set of in-memory linearizable data structures
             */
            cp::cp_subsystem &get_cp_subsystem();

        private:
            std::shared_ptr<impl::HazelcastClientInstanceImpl> clientImpl;
        };

        /**
         *
         * @return the version of the client
         */
        constexpr boost::string_view version() {
            return HAZELCAST_VERSION;
        }

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
