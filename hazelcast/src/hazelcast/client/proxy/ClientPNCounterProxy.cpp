/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/client/proxy/ClientPNCounterProxy.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/cluster/impl/VectorClock.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientPNCounterProxy::SERVICE_NAME = "hz:impl:PNCounterService";
            const std::shared_ptr<std::set<Address> > ClientPNCounterProxy::EMPTY_ADDRESS_LIST(
                    new std::set<Address>());

            ClientPNCounterProxy::ClientPNCounterProxy(const std::string &serviceName, const std::string &objectName,
                                                       spi::ClientContext *context)
                    : ProxyImpl(serviceName, objectName, context), maxConfiguredReplicaCount(0),
                      observedClock(std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock())),
                      logger(context->getLogger()) {
            }

            std::ostream &operator<<(std::ostream &os, const ClientPNCounterProxy &proxy) {
                os << "PNCounter{name='" << proxy.getName() << "\'}";
                return os;
            }

            int64_t ClientPNCounterProxy::get() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::get",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeGetInternal(EMPTY_ADDRESS_LIST,
                                                                                        std::unique_ptr<exception::HazelcastException>(),
                                                                                        target);
                protocol::codec::PNCounterGetCodec::ResponseParameters resultParameters = protocol::codec::PNCounterGetCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndAdd(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndAdd",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(delta, true, EMPTY_ADDRESS_LIST,
                                                                                        std::unique_ptr<exception::HazelcastException>(),
                                                                                        target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::addAndGet(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::addAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(delta, false,
                                                                                        EMPTY_ADDRESS_LIST,
                                                                                        std::unique_ptr<exception::HazelcastException>(),
                                                                                        target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndSubtract(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndSubtract",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-delta, true,
                                                                                        EMPTY_ADDRESS_LIST,
                                                                                        std::unique_ptr<exception::HazelcastException>(),
                                                                                        target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::subtractAndGet(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::subtractAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-delta, false,
                                                                                        EMPTY_ADDRESS_LIST,
                                                                                        std::unique_ptr<exception::HazelcastException>(),
                                                                                        target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::decrementAndGet() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::decrementAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-1, false, EMPTY_ADDRESS_LIST,
                                                                                        std::unique_ptr<exception::HazelcastException>(),
                                                                                        target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::incrementAndGet() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::incrementAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(1, false, EMPTY_ADDRESS_LIST,
                                                                                        std::unique_ptr<exception::HazelcastException>(),
                                                                                        target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndDecrement() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndDecrement",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-1, true, EMPTY_ADDRESS_LIST,
                                                                                        std::unique_ptr<exception::HazelcastException>(),
                                                                                        target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndIncrement() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndIncrement",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(1, true, EMPTY_ADDRESS_LIST,
                                                                                        std::unique_ptr<exception::HazelcastException>(),
                                                                                        target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            void ClientPNCounterProxy::reset() {
                observedClock = std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock());
            }

            std::shared_ptr<Address>
            ClientPNCounterProxy::getCRDTOperationTarget(const std::set<Address> &excludedAddresses) {
                if (currentTargetReplicaAddress.get().get() != NULL &&
                    excludedAddresses.find(*currentTargetReplicaAddress.get()) == excludedAddresses.end()) {
                    return currentTargetReplicaAddress;
                }

                {
                    util::LockGuard guard(targetSelectionMutex);
                    if (currentTargetReplicaAddress.get() == NULL ||
                        excludedAddresses.find(*currentTargetReplicaAddress.get()) != excludedAddresses.end()) {
                        currentTargetReplicaAddress = chooseTargetReplica(excludedAddresses);
                    }
                }
                return currentTargetReplicaAddress;
            }

            std::shared_ptr<Address>
            ClientPNCounterProxy::chooseTargetReplica(const std::set<Address> &excludedAddresses) {
                std::vector<Address> replicaAddresses = getReplicaAddresses(excludedAddresses);
                if (replicaAddresses.empty()) {
                    return std::shared_ptr<Address>();
                }
                // TODO: Use a random generator as used in Java (ThreadLocalRandomProvider) which is per thread
                int randomReplicaIndex = std::abs(rand()) % (int) replicaAddresses.size();
                return std::shared_ptr<Address>(new Address(replicaAddresses[randomReplicaIndex]));
            }

            std::vector<Address> ClientPNCounterProxy::getReplicaAddresses(const std::set<Address> &excludedAddresses) {
                std::vector<Member> dataMembers = getContext().getClientClusterService().getMembers(
                        *cluster::memberselector::MemberSelectors::DATA_MEMBER_SELECTOR);
                int32_t maxConfiguredReplicaCount = getMaxConfiguredReplicaCount();
                int currentReplicaCount = util::min<int>(maxConfiguredReplicaCount, (int) dataMembers.size());

                std::vector<Address> replicaAddresses;
                for (int i = 0; i < currentReplicaCount; i++) {
                    const Address &dataMemberAddress = dataMembers[i].getAddress();
                    if (excludedAddresses.find(dataMemberAddress) == excludedAddresses.end()) {
                        replicaAddresses.push_back(dataMemberAddress);
                    }
                }
                return replicaAddresses;
            }

            int32_t ClientPNCounterProxy::getMaxConfiguredReplicaCount() {
                if (maxConfiguredReplicaCount > 0) {
                    return maxConfiguredReplicaCount;
                } else {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::PNCounterGetConfiguredReplicaCountCodec::encodeRequest(
                            getName());
                    maxConfiguredReplicaCount = invokeAndGetResult<int32_t, protocol::codec::PNCounterGetConfiguredReplicaCountCodec::ResponseParameters>(
                            request);
                }
                return maxConfiguredReplicaCount;
            }

            std::shared_ptr<protocol::ClientMessage>
            ClientPNCounterProxy::invokeGetInternal(std::shared_ptr<std::set<Address> > excludedAddresses,
                                                    const std::unique_ptr<exception::IException> &lastException,
                                                    const std::shared_ptr<Address> &target) {
                if (target.get() == NULL) {
                    if (lastException.get()) {
                        throw *lastException;
                    } else {
                        throw (exception::ExceptionBuilder<exception::NoDataMemberInClusterException>(
                                "ClientPNCounterProxy::invokeGetInternal") <<
                                                                           "Cannot invoke operations on a CRDT because the cluster does not contain any data members").build();
                    }
                }
                try {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::PNCounterGetCodec::encodeRequest(
                            getName(), observedClock.get()->entrySet(), *target);
                    return invokeOnAddress(request, *target);
                } catch (exception::HazelcastException &e) {
                    logger.finest("Exception occurred while invoking operation on target " , *target , ", choosing different target. Cause: " , e);
                    if (excludedAddresses == EMPTY_ADDRESS_LIST) {
                        // TODO: Make sure that this only affects the local variable of the method
                        excludedAddresses = std::shared_ptr<std::set<Address> >(new std::set<Address>());
                    }
                    excludedAddresses->insert(*target);
                    std::shared_ptr<Address> newTarget = getCRDTOperationTarget(*excludedAddresses);
                    std::unique_ptr<exception::IException> exception = e.clone();
                    return invokeGetInternal(excludedAddresses, exception, newTarget);
                }
            }


            std::shared_ptr<protocol::ClientMessage>
            ClientPNCounterProxy::invokeAddInternal(int64_t delta, bool getBeforeUpdate,
                                                    std::shared_ptr<std::set<Address> > excludedAddresses,
                                                    const std::unique_ptr<exception::IException> &lastException,
                                                    const std::shared_ptr<Address> &target) {
                if (target.get() == NULL) {
                    if (lastException.get()) {
                        throw *lastException;
                    } else {
                        throw (exception::ExceptionBuilder<exception::NoDataMemberInClusterException>(
                                "ClientPNCounterProxy::invokeAddInternal") <<
                                                                           "Cannot invoke operations on a CRDT because the cluster does not contain any data members").build();
                    }
                }

                try {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::PNCounterAddCodec::encodeRequest(
                            getName(), delta, getBeforeUpdate, observedClock.get()->entrySet(), *target);
                    return invokeOnAddress(request, *target);
                } catch (exception::HazelcastException &e) {
                    logger.finest("Unable to provide session guarantees when sending operations to " , *target , ", choosing different target. Cause: " , e);
                    if (excludedAddresses == EMPTY_ADDRESS_LIST) {
                        // TODO: Make sure that this only affects the local variable of the method
                        excludedAddresses = std::shared_ptr<std::set<Address> >(new std::set<Address>());
                    }
                    excludedAddresses->insert(*target);
                    std::shared_ptr<Address> newTarget = getCRDTOperationTarget(*excludedAddresses);
                    std::unique_ptr<exception::IException> exception = e.clone();
                    return invokeAddInternal(delta, getBeforeUpdate, excludedAddresses, exception, newTarget);
                }
            }

            void ClientPNCounterProxy::updateObservedReplicaTimestamps(
                    const std::vector<std::pair<std::string, int64_t> > &receivedLogicalTimestamps) {
                std::shared_ptr<cluster::impl::VectorClock> received = toVectorClock(receivedLogicalTimestamps);
                for (;;) {
                    std::shared_ptr<cluster::impl::VectorClock> currentClock = this->observedClock;
                    if (currentClock->isAfter(*received)) {
                        break;
                    }
                    if (observedClock.compareAndSet(currentClock, received)) {
                        break;
                    }
                }
            }

            std::shared_ptr<cluster::impl::VectorClock> ClientPNCounterProxy::toVectorClock(
                    const std::vector<std::pair<std::string, int64_t> > &replicaLogicalTimestamps) {
                return std::shared_ptr<cluster::impl::VectorClock>(
                        new cluster::impl::VectorClock(replicaLogicalTimestamps));
            }

            std::shared_ptr<Address> ClientPNCounterProxy::getCurrentTargetReplicaAddress() {
                return currentTargetReplicaAddress.get();
            }
        }
    }
}
