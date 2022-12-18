/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <iosfwd>
#include <unordered_set>
#include <memory>
#include <atomic>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>

#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/impl/vector_clock.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
class logger;

namespace client {
namespace proxy {
class HAZELCAST_API PNCounterImpl : public ProxyImpl
{
public:
    friend std::ostream& operator<<(std::ostream& os,
                                    const PNCounterImpl& proxy);

    /**
     * Returns the current value of the counter.
     *
     * @throws no_data_member_in_cluster if the cluster does not contain
     *                                        any data members
     * @throws unsupported_operation  if the cluster version is less
     *                                        than 3.10
     * @throws consistency_lost       if the session guarantees have
     *                                        been lost (see class level
     * javadoc)
     * @see ClusterService#getClusterVersion()
     */
    boost::future<int64_t> get();

    /**
     * Adds the given value to the current value.
     *
     * @param delta the value to add
     * @return the previous value
     * @throws no_data_member_in_cluster if the cluster does not contain
     *                                        any data members
     * @throws unsupported_operation  if the cluster version is less
     *                                        than 3.10
     * @throws consistency_lost       if the session guarantees have
     *                                        been lost (see class level
     * javadoc)
     * @see ClusterService#getClusterVersion()
     */
    boost::future<int64_t> get_and_add(int64_t delta);

    /**
     * Adds the given value to the current value.
     *
     * @param delta the value to add
     * @return the updated value
     * @throws no_data_member_in_cluster if the cluster does not contain
     *                                        any data members
     * @throws unsupported_operation  if the cluster version is less
     *                                        than 3.10
     * @throws consistency_lost       if the session guarantees have
     *                                        been lost (see class level
     * javadoc)
     * @see ClusterService#getClusterVersion()
     */
    boost::future<int64_t> add_and_get(int64_t delta);

    /**
     * Subtracts the given value from the current value.
     *
     * @param delta the value to add
     * @return the previous value
     * @throws no_data_member_in_cluster if the cluster does not contain
     *                                        any data members
     * @throws unsupported_operation  if the cluster version is less
     *                                        than 3.10
     * @throws consistency_lost       if the session guarantees have
     *                                        been lost (see class level
     * javadoc)
     * @see ClusterService#getClusterVersion()
     */
    boost::future<int64_t> get_and_subtract(int64_t delta);

    /**
     * Subtracts the given value from the current value.
     *
     * @param delta the value to subtract
     * @return the updated value
     * @throws no_data_member_in_cluster if the cluster does not contain
     *                                        any data members
     * @throws unsupported_operation  if the cluster version is less
     *                                        than 3.10
     * @throws consistency_lost       if the session guarantees have
     *                                        been lost (see class level
     * javadoc)
     * @see ClusterService#getClusterVersion()
     */
    boost::future<int64_t> subtract_and_get(int64_t delta);

    /**
     * Decrements by one the current value.
     *
     * @return the updated value
     * @throws no_data_member_in_cluster if the cluster does not contain
     *                                        any data members
     * @throws unsupported_operation  if the cluster version is less
     *                                        than 3.10
     * @throws consistency_lost       if the session guarantees have
     *                                        been lost (see class level
     * javadoc)
     * @see ClusterService#getClusterVersion()
     */
    boost::future<int64_t> decrement_and_get();

    /**
     * Increments by one the current value.
     *
     * @return the updated value
     * @throws no_data_member_in_cluster if the cluster does not contain
     *                                        any data members
     * @throws unsupported_operation  if the cluster version is less
     *                                        than 3.10
     * @throws consistency_lost       if the session guarantees have
     *                                        been lost (see class level
     * javadoc)
     * @see ClusterService#getClusterVersion()
     */
    boost::future<int64_t> increment_and_get();

    /**
     * Decrements by one the current value.
     *
     * @return the previous value
     * @throws no_data_member_in_cluster if the cluster does not contain
     *                                        any data members
     * @throws unsupported_operation  if the cluster version is less
     *                                        than 3.10
     * @throws consistency_lost       if the session guarantees have
     *                                        been lost (see class level
     * javadoc)
     * @see ClusterService#getClusterVersion()
     */
    boost::future<int64_t> get_and_decrement();

    /**
     * Increments by one the current value.
     *
     * @return the previous value
     * @throws no_data_member_in_cluster if the cluster does not contain
     *                                        any data members
     * @throws unsupported_operation  if the cluster version is less
     *                                        than 3.10
     * @throws consistency_lost       if the session guarantees have
     *                                        beenlost (see class level javadoc)
     * @see ClusterService#getClusterVersion()
     */
    boost::future<int64_t> get_and_increment();

    /**
     * Resets the observed state by this PN counter. This method may be used
     * after a method invocation has thrown a {@link consistency_lost}
     * to reset the proxy and to be able to start a new session.
     */
    boost::future<void> reset();

    /**
     * Returns the current target replica address to which this proxy is
     * sending invocations.
     */
    // public for testing purposes
    boost::shared_ptr<member> get_current_target_replica_address();

protected:
    PNCounterImpl(const std::string& service_name,
                  const std::string& object_name,
                  spi::ClientContext* context);

    static const std::shared_ptr<std::unordered_set<member>> EMPTY_ADDRESS_LIST;

    /**
     * Returns the target on which this proxy should invoke a CRDT operation.
     * On first invocation of this method, the method will choose a target
     * address and return that address on future invocations. Replicas with
     * addresses contained in the {@code excludedAddresses} list are excluded
     * and if the chosen replica is in this list, a new replica is chosen and
     * returned on future invocations.
     * The method may return {@code null} if there are no viable target
     * addresses.
     *
     * @param excludedAddresses the addresses to exclude when choosing a replica
     *                          address
     * @return a CRDT replica address or {@code null} if there are no viable
     * addresses
     */
    boost::shared_ptr<member> get_crdt_operation_target(
      const std::unordered_set<member>& excluded_addresses);

    /**
     * Chooses and returns a CRDT replica address. Replicas with addresses
     * contained in the {@code excludedAddresses} list are excluded and the
     * method chooses randomly between the collection of viable target
     * addresses. <p> The method may return {@code null} if there are no viable
     * addresses.
     *
     * @param excludedAddresses the addresses to exclude when choosing a replica
     *                          address
     * @return a CRDT replica address or {@code null} if there are no viable
     * addresses
     */
    boost::shared_ptr<member> choose_target_replica(
      const std::unordered_set<member>& excluded_addresses);

    /**
     * Returns the addresses of the CRDT replicas from the current state of the
     * local membership list. Addresses contained in the {@code
     * excludedAddresses} collection are excluded.
     *
     * @param excludedMembers the addresses to exclude when choosing a replica
     *                          address
     * @return list of possible CRDT replica addresses
     */
    std::vector<member> get_replica_addresses(
      const std::unordered_set<member>& excluded_members);

    boost::shared_ptr<member> try_choose_a_new_target(
      std::shared_ptr<std::unordered_set<member>> excluded_addresses,
      boost::shared_ptr<member> last_target,
      const exception::hazelcast_& last_exception);

    /**
     * Returns the max configured replica count.
     * When invoked for the first time, this method will fetch the
     * configuration from a cluster member.
     *
     * @return the maximum configured replica count
     */
    int32_t get_max_configured_replica_count();

    /**
     * Returns the current value of the counter.
     * It will invoke client messages recursively on viable replica addresses
     * until successful or the list of viable replicas is exhausted.
     * Replicas with addresses contained in the {@code excludedAddresses} are
     * skipped. If there are no viable replicas, this method will throw the
     * {@code lastException} if not {@code null} or a
     * {@link no_data_member_in_cluster} if the {@code lastException} is
     * {@code null}.
     *
     * @param excludedAddresses the addresses to exclude when choosing a replica
     *                          address, must not be {@code null}
     * @param lastException     the exception thrown from the last invocation of
     *                          the {@code request} on a replica, may be {@code
     * null}
     * @return the result of the request invocation on a replica
     * @throws no_data_member_in_cluster if there are no replicas and the
     *                                        {@code lastException} is false
     */
    boost::future<int64_t> invoke_get_internal(
      std::shared_ptr<std::unordered_set<member>> excluded_addresses,
      std::exception_ptr last,
      const boost::shared_ptr<member>& target);

    /**
     * Adds the {@code delta} and returns the value of the counter before the
     * update if {@code getBeforeUpdate} is {@code true} or the value after
     * the update if it is {@code false}.
     * It will invoke client messages recursively on viable replica addresses
     * until successful or the list of viable replicas is exhausted.
     * Replicas with addresses contained in the {@code excludedAddresses} are
     * skipped. If there are no viable replicas, this method will throw the
     * {@code lastException} if not {@code null} or a
     * {@link no_data_member_in_cluster} if the {@code lastException} is
     * {@code null}.
     *
     * @param delta             the delta to add to the counter value, can be
     * negative
     * @param getBeforeUpdate   {@code true} if the operation should return the
     *                          counter value before the addition, {@code false}
     *                          if it should return the value after the addition
     * @param excludedAddresses the addresses to exclude when choosing a replica
     *                          address, must not be {@code null}
     * @param lastException     the exception thrown from the last invocation of
     *                          the {@code request} on a replica, may be {@code
     * null}
     * @return the result of the request invocation on a replica
     * @throws no_data_member_in_cluster if there are no replicas and the
     *                                        {@code lastException} is {@code
     * null}
     */
    boost::future<int64_t> invoke_add_internal(
      int64_t delta,
      bool get_before_update,
      std::shared_ptr<std::unordered_set<member>> excluded_addresses,
      std::exception_ptr last,
      const boost::shared_ptr<member>& target);

    /**
     * Updates the locally observed CRDT vector clock atomically. This method
     * is thread safe and can be called concurrently. The method will only
     * update the clock if the {@code receivedLogicalTimestamps} is higher than
     * the currently observed vector clock.
     *
     * @param receivedLogicalTimestamps logical timestamps received from a
     * replica state read
     */
    void update_observed_replica_timestamps(
      const impl::vector_clock::timestamp_vector& received_logical_timestamps);

    /**
     * Transforms the list of replica logical timestamps to a vector clock
     * instance.
     *
     * @param replicaLogicalTimestamps the logical timestamps
     * @return a vector clock instance
     */
    static std::shared_ptr<impl::vector_clock> to_vector_clock(
      const impl::vector_clock::timestamp_vector& replica_logical_timestamps);

    boost::atomic_shared_ptr<member> current_target_replica_address_;
    std::mutex target_selection_mutex_;
    std::atomic<int32_t> max_configured_replica_count_;
    /**
     * The last vector clock observed by this proxy. It is used for maintaining
     * session consistency guarantees when reading from different replicas.
     */
    util::Sync<std::shared_ptr<impl::vector_clock>> observed_clock_;
    logger& logger_;

    int64_t get_and_update_timestamps(boost::future<protocol::ClientMessage> f);
};
} // namespace proxy
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
