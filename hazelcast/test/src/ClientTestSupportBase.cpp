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
#include "HazelcastServerFactory.h"
#include "ClientTestSupportBase.h"

#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/exception/IllegalStateException.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/pimpl/SerializationService.h>
#include <hazelcast/util/UuidUtil.h>
#include <hazelcast/client/impl/Partition.h>

namespace hazelcast {
    namespace client {
        namespace test {
            std::string ClientTestSupportBase::getCAFilePath() {
                return "hazelcast/test/resources/cpp_client.crt";
            }

            hazelcast::client::ClientConfig ClientTestSupportBase::getConfig() {
                ClientConfig clientConfig;
                clientConfig.addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                return clientConfig;
            }

            HazelcastClient ClientTestSupportBase::getNewClient() {
                return HazelcastClient(getConfig());
            }

            const std::string ClientTestSupportBase::getSslFilePath() {
                return "hazelcast/test/resources/hazelcast-ssl.xml";
            }

            std::string ClientTestSupportBase::randomMapName() {
                return randomString();
            }

            std::string ClientTestSupportBase::randomString() {
                // TODO: Change with secure uuid generator as in Java
                return util::UuidUtil::newUnsecureUuidString();
            }

            void ClientTestSupportBase::sleepSeconds(int32_t seconds) {
                util::sleep(seconds);
            }

            ClientTestSupportBase::ClientTestSupportBase() {
            }

            std::string ClientTestSupportBase::generateKeyOwnedBy(spi::ClientContext &context, const Member &member) {
                spi::ClientPartitionService &partitionService = context.getPartitionService();
                serialization::pimpl::SerializationService &serializationService = context.getSerializationService();
                while (true) {
                    std::string id = randomString();
                    int partitionId = partitionService.getPartitionId(serializationService.toData<std::string>(&id));
                    std::shared_ptr<impl::Partition> partition = partitionService.getPartition(partitionId);
                    if (*partition->getOwner() == member) {
                        return id;
                    }
                }
            }
        }
    }

    namespace util {
        StartedThread::StartedThread(const std::string &name, void (*func)(ThreadArgs &),
                                     void *arg0, void *arg1, void *arg2, void *arg3)
                : name(name), logger(new util::ILogger("StartedThread", "StartedThread", "testversion",
                                                       client::config::LoggerConfig())) {
            init(func, arg0, arg1, arg2, arg3);
        }

        StartedThread::StartedThread(void (func)(ThreadArgs &),
                                     void *arg0,
                                     void *arg1,
                                     void *arg2,
                                     void *arg3)
                : name("hz.unnamed"), logger(new util::ILogger("StartedThread", "StartedThread", "testversion",
                                                               client::config::LoggerConfig())) {
            init(func, arg0, arg1, arg2, arg3);
        }

        void StartedThread::init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3) {
            threadArgs.arg0 = arg0;
            threadArgs.arg1 = arg1;
            threadArgs.arg2 = arg2;
            threadArgs.arg3 = arg3;
            threadArgs.func = func;
            if (!logger->start()) {
                throw (client::exception::ExceptionBuilder<client::exception::IllegalStateException>(
                        "StartedThread::init") << "Could not start logger " << logger->getInstanceName()).build();
            }

            thread.reset(
                    new util::Thread(std::shared_ptr<util::Runnable>(new util::RunnableDelegator(*this)), *logger));
            threadArgs.currentThread = thread.get();
            thread->start();
        }

        void StartedThread::run() {
            threadArgs.func(threadArgs);
        }

        const std::string StartedThread::getName() const {
            return name;
        }

        bool StartedThread::join() {
            return thread->join();
        }

        StartedThread::~StartedThread() {
        }

        void StartedThread::cancel() {
            thread->cancel();
        }

    }
}
