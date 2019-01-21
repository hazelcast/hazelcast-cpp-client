/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/IAtomicLong.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/spi/LifecycleService.h"


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        HazelcastClient::HazelcastClient(ClientConfig &config) : clientImpl(
                new impl::HazelcastClientInstanceImpl(config)) {
        }

        const std::string &HazelcastClient::getName() const {
            return clientImpl->getName();
        }

        IdGenerator HazelcastClient::getIdGenerator(const std::string &name) {
            return clientImpl->getIdGenerator(name);
        }

        FlakeIdGenerator HazelcastClient::getFlakeIdGenerator(const std::string &name) {
            return clientImpl->getFlakeIdGenerator(name);
        }

        IAtomicLong HazelcastClient::getIAtomicLong(const std::string &name) {
            return clientImpl->getIAtomicLong(name);
        }


        boost::shared_ptr<crdt::pncounter::PNCounter> HazelcastClient::getPNCounter(const std::string &name) {
            return clientImpl->getPNCounter(name);
        }

        ICountDownLatch HazelcastClient::getICountDownLatch(const std::string &name) {
            return clientImpl->getICountDownLatch(name);
        }

        ILock HazelcastClient::getILock(const std::string &name) {
            return clientImpl->getILock(name);
        }

        ISemaphore HazelcastClient::getISemaphore(const std::string &name) {
            return clientImpl->getISemaphore(name);
        }

        ClientConfig &HazelcastClient::getClientConfig() {
            return clientImpl->getClientConfig();
        }

        TransactionContext HazelcastClient::newTransactionContext() {
            return clientImpl->newTransactionContext();
        }

        TransactionContext HazelcastClient::newTransactionContext(const TransactionOptions &options) {
            return clientImpl->newTransactionContext(options);
        }

        Cluster &HazelcastClient::getCluster() {
            return clientImpl->getCluster();
        }

        void HazelcastClient::addLifecycleListener(LifecycleListener *lifecycleListener) {
            clientImpl->addLifecycleListener(lifecycleListener);
        }

        bool HazelcastClient::removeLifecycleListener(LifecycleListener *lifecycleListener) {
            return clientImpl->removeLifecycleListener(lifecycleListener);
        }

        void HazelcastClient::shutdown() {
            clientImpl->shutdown();
        }

        mixedtype::HazelcastClient &HazelcastClient::toMixedType() const {
            return clientImpl->toMixedType();
        }

        spi::LifecycleService &HazelcastClient::getLifecycleService() {
            return clientImpl->getLifecycleService();
        }

        boost::shared_ptr<IExecutorService> HazelcastClient::getExecutorService(const std::string &name) {
            return clientImpl->getExecutorService(name);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
