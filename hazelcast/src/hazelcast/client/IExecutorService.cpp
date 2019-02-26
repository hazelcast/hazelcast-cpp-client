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

#include <boost/foreach.hpp>
#include <hazelcast/client/IExecutorService.h>


#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/client/protocol/codec/ExecutorServiceShutdownCodec.h"
#include "hazelcast/client/protocol/codec/ExecutorServiceIsShutdownCodec.h"

namespace hazelcast {
    namespace client {
        const std::string IExecutorService::SERVICE_NAME = "hz:impl:executorService";

        IExecutorService::IExecutorService(const std::string &name, spi::ClientContext *context) : ProxyImpl(
                SERVICE_NAME, name, context), consecutiveSubmits(0), lastSubmitTime(0) {
        }

        std::vector<Member>
        IExecutorService::selectMembers(const cluster::memberselector::MemberSelector &memberSelector) {
            std::vector<Member> selected;
            std::vector<Member> members = getContext().getClientClusterService().getMemberList();
            BOOST_FOREACH (const Member &member, members) {
                            if (memberSelector.select(member)) {
                                selected.push_back(member);
                            }
                        }
            if (selected.empty()) {
                throw (exception::ExceptionBuilder<exception::RejectedExecutionException>(
                        "IExecutorService::selectMembers") << "No member selected with memberSelector["
                                                           << memberSelector << "]").build();
            }
            return selected;
        }

        boost::shared_ptr<spi::impl::ClientInvocationFuture>
        IExecutorService::invokeOnTarget(std::auto_ptr<protocol::ClientMessage> &request, const Address &target) {
            try {
                boost::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), target);
                return clientInvocation->invoke();
            } catch (exception::IException &e) {
                util::ExceptionUtil::rethrow(e);
            }
            return boost::shared_ptr<spi::impl::ClientInvocationFuture>();
        }

        boost::shared_ptr<spi::impl::ClientInvocationFuture>
        IExecutorService::invokeOnPartitionOwner(std::auto_ptr<protocol::ClientMessage> &request, int partitionId) {
            try {
                boost::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), partitionId);
                return clientInvocation->invoke();
            } catch (exception::IException &e) {
                util::ExceptionUtil::rethrow(e);
            }
            return boost::shared_ptr<spi::impl::ClientInvocationFuture>();
        }

        bool IExecutorService::isSyncComputation(bool preventSync) {
            int64_t now = util::currentTimeMillis();

            int64_t last = lastSubmitTime;
            lastSubmitTime = now;

            if (last + MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS < now) {
                consecutiveSubmits.set(0);
                return false;
            }

            return !preventSync && (consecutiveSubmits++ % MAX_CONSECUTIVE_SUBMITS == 0);
        }

        Address IExecutorService::getMemberAddress(const Member &member) {
            boost::shared_ptr<Member> m = getContext().getClientClusterService().getMember(member.getUuid());
            if (m.get() == NULL) {
                throw (exception::ExceptionBuilder<exception::HazelcastException>(
                        "IExecutorService::getMemberAddress(Member)") << member << " is not available!").build();
            }
            return m->getAddress();
        }

        int IExecutorService::randomPartitionId() {
            spi::ClientPartitionService &partitionService = getContext().getPartitionService();
            return rand() % partitionService.getPartitionCount();
        }

        void IExecutorService::shutdown() {
            std::auto_ptr<protocol::ClientMessage> request = protocol::codec::ExecutorServiceShutdownCodec::encodeRequest(
                    getName());
            invoke(request);
        }

        bool IExecutorService::isShutdown() {
            std::auto_ptr<protocol::ClientMessage> request = protocol::codec::ExecutorServiceIsShutdownCodec::encodeRequest(
                    getName());
            return invokeAndGetResult<bool, protocol::codec::ExecutorServiceIsShutdownCodec::ResponseParameters>(
                    request);
        }

        bool IExecutorService::isTerminated() {
            return isShutdown();
        }
    }
}
