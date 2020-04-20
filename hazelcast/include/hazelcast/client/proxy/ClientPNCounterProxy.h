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
#ifndef HAZELCAST_CLIENT_PROXY_CLIENTPNCOUNTERPROXY_H_
#define HAZELCAST_CLIENT_PROXY_CLIENTPNCOUNTERPROXY_H_

#include <ostream>
#include <set>
#include <memory>
#include <atomic>

#include "hazelcast/client/crdt/pncounter/PNCounter.h"
#include "hazelcast/client/proxy/ProxyImpl.h"

namespace hazelcast {
    namespace client {
        namespace cluster {
            namespace impl {
                class VectorClock;
            }
        }
        namespace proxy {
            /**
             * Client proxy implementation for a {@link PNCounter}.
             */
            class ClientPNCounterProxy : public crdt::pncounter::PNCounter, public ProxyImpl {
            public:
                static const std::string SERVICE_NAME;

                ClientPNCounterProxy(const std::string &serviceName, const std::string &objectName,
                                     spi::ClientContext *context);

                friend std::ostream &operator<<(std::ostream &os, const ClientPNCounterProxy &proxy);

                virtual int64_t get();

                virtual int64_t getAndAdd(int64_t delta);

                virtual int64_t addAndGet(int64_t delta);

                virtual int64_t getAndSubtract(int64_t delta);

                virtual int64_t subtractAndGet(int64_t delta);

                virtual int64_t decrementAndGet();

                virtual int64_t incrementAndGet();

                virtual int64_t getAndDecrement();

                virtual int64_t getAndIncrement();

                virtual void reset();

                /**
                 * Returns the current target replica address to which this proxy is
                 * sending invocations.
                 */
                // public for testing purposes
                std::shared_ptr<Address> HAZELCAST_API getCurrentTargetReplicaAddress();

            private:
                static const std::shared_ptr<std::set<Address> > EMPTY_ADDRESS_LIST;

                /**
                 * Returns the target on which this proxy should invoke a CRDT operation.
                 * On first invocation of this method, the method will choose a target
                 * address and return that address on future invocations. Replicas with
                 * addresses contained in the {@code excludedAddresses} list are excluded
                 * and if the chosen replica is in this list, a new replica is chosen and
                 * returned on future invocations.
                 * The method may return {@code null} if there are no viable target addresses.
                 *
                 * @param excludedAddresses the addresses to exclude when choosing a replica
                 *                          address
                 * @return a CRDT replica address or {@code null} if there are no viable
                 * addresses
                 */
                std::shared_ptr<Address> getCRDTOperationTarget(const std::set<Address> &excludedAddresses);

                /**
                 * Chooses and returns a CRDT replica address. Replicas with addresses
                 * contained in the {@code excludedAddresses} list are excluded and the
                 * method chooses randomly between the collection of viable target addresses.
                 * <p>
                 * The method may return {@code null} if there are no viable addresses.
                 *
                 * @param excludedAddresses the addresses to exclude when choosing a replica
                 *                          address
                 * @return a CRDT replica address or {@code null} if there are no viable addresses
                 */
                std::shared_ptr<Address> chooseTargetReplica(const std::set<Address> &excludedAddresses);

                /**
                 * Returns the addresses of the CRDT replicas from the current state of the
                 * local membership list. Addresses contained in the {@code excludedAddresses}
                 * collection are excluded.
                 *
                 * @param excludedAddresses the addresses to exclude when choosing a replica
                 *                          address
                 * @return list of possible CRDT replica addresses
                 */
                std::vector<Address> getReplicaAddresses(const std::set<Address> &excludedAddresses);

                /**
                 * Returns the max configured replica count.
                 * When invoked for the first time, this method will fetch the
                 * configuration from a cluster member.
                 *
                 * @return the maximum configured replica count
                 */
                int32_t getMaxConfiguredReplicaCount();

                /**
                 * Returns the current value of the counter.
                 * It will invoke client messages recursively on viable replica addresses
                 * until successful or the list of viable replicas is exhausted.
                 * Replicas with addresses contained in the {@code excludedAddresses} are
                 * skipped. If there are no viable replicas, this method will throw the
                 * {@code lastException} if not {@code null} or a
                 * {@link NoDataMemberInClusterException} if the {@code lastException} is
                 * {@code null}.
                 *
                 * @param excludedAddresses the addresses to exclude when choosing a replica
                 *                          address, must not be {@code null}
                 * @param lastException     the exception thrown from the last invocation of
                 *                          the {@code request} on a replica, may be {@code null}
                 * @return the result of the request invocation on a replica
                 * @throws NoDataMemberInClusterException if there are no replicas and the
                 *                                        {@code lastException} is false
                 */
                protocol::ClientMessage
                invokeGetInternal(std::shared_ptr<std::set<Address> > excludedAddresses,
                                  std::exception_ptr lastException,
                                  const std::shared_ptr<Address> &target);


                /**
                 * Adds the {@code delta} and returns the value of the counter before the
                 * update if {@code getBeforeUpdate} is {@code true} or the value after
                 * the update if it is {@code false}.
                 * It will invoke client messages recursively on viable replica addresses
                 * until successful or the list of viable replicas is exhausted.
                 * Replicas with addresses contained in the {@code excludedAddresses} are
                 * skipped. If there are no viable replicas, this method will throw the
                 * {@code lastException} if not {@code null} or a
                 * {@link NoDataMemberInClusterException} if the {@code lastException} is
                 * {@code null}.
                 *
                 * @param delta             the delta to add to the counter value, can be negative
                 * @param getBeforeUpdate   {@code true} if the operation should return the
                 *                          counter value before the addition, {@code false}
                 *                          if it should return the value after the addition
                 * @param excludedAddresses the addresses to exclude when choosing a replica
                 *                          address, must not be {@code null}
                 * @param lastException     the exception thrown from the last invocation of
                 *                          the {@code request} on a replica, may be {@code null}
                 * @return the result of the request invocation on a replica
                 * @throws NoDataMemberInClusterException if there are no replicas and the
                 *                                        {@code lastException} is {@code null}
                 */
                protocol::ClientMessage
                invokeAddInternal(int64_t delta, bool getBeforeUpdate,
                                  std::shared_ptr<std::set<Address> > excludedAddresses,
                                  std::exception_ptr lastException, const std::shared_ptr<Address> &target);

                /**
                 * Updates the locally observed CRDT vector clock atomically. This method
                 * is thread safe and can be called concurrently. The method will only
                 * update the clock if the {@code receivedLogicalTimestamps} is higher than
                 * the currently observed vector clock.
                 *
                 * @param receivedLogicalTimestamps logical timestamps received from a replica state read
                 */
                void updateObservedReplicaTimestamps(
                        const std::vector<std::pair<std::string, int64_t> > &receivedLogicalTimestamps);

                /**
                 * Transforms the list of replica logical timestamps to a vector clock instance.
                 *
                 * @param replicaLogicalTimestamps the logical timestamps
                 * @return a vector clock instance
                 */
                std::shared_ptr<cluster::impl::VectorClock>
                toVectorClock(const std::vector<std::pair<std::string, int64_t> > &replicaLogicalTimestamps);

                util::Sync<std::shared_ptr<Address> > currentTargetReplicaAddress;
                std::mutex targetSelectionMutex;
                std::atomic<int32_t> maxConfiguredReplicaCount;
                /**
                 * The last vector clock observed by this proxy. It is used for maintaining
                 * session consistency guarantees when reading from different replicas.
                 */
                util::Sync<std::shared_ptr<cluster::impl::VectorClock> > observedClock;
                util::ILogger &logger;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_PROXY_CLIENTPNCOUNTERPROXY_H_
