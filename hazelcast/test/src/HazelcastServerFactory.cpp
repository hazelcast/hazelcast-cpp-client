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

#include <iostream>

#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/exception/IllegalStateException.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            extern HazelcastServerFactory *g_srvFactory;
            extern std::shared_ptr<RemoteControllerClient> remoteController;

            HazelcastServerFactory::HazelcastServerFactory(const std::string &serverXmlConfigFilePath)
                    : HazelcastServerFactory::HazelcastServerFactory(g_srvFactory->getServerAddress(),
                                                                     serverXmlConfigFilePath) {
            }

            HazelcastServerFactory::HazelcastServerFactory(const std::string &serverAddress,
                                                           const std::string &serverXmlConfigFilePath)
                    : logger("HazelcastServerFactory", "HazelcastServerFactory", "testversion", config::LoggerConfig()),
                      serverAddress(serverAddress) {

                if (!logger.start()) {
                    throw (client::exception::ExceptionBuilder<client::exception::IllegalStateException>(
                            "HazelcastServerFactory::HazelcastServerFactory") << "Could not start logger "
                                                                              << logger.getInstanceName()).build();
                }

                std::string xmlConfig = readFromXmlFile(serverXmlConfigFilePath);

                Cluster cluster;
                remoteController->createCluster(cluster, HAZELCAST_VERSION, xmlConfig);

                this->clusterId = cluster.id;
            }

            HazelcastServerFactory::~HazelcastServerFactory() {
                remoteController->shutdownCluster(clusterId);
            }

            remote::Member HazelcastServerFactory::startServer() {
                remote::Member member;
                remoteController->startMember(member, clusterId);
                return member;
            }

            bool HazelcastServerFactory::setAttributes(int memberStartOrder) {
                std::ostringstream script;
                script << "function attrs() { "
                          "var member = instance_" << memberStartOrder << ".getCluster().getLocalMember(); "
                                                                          "member.setIntAttribute(\"intAttr\", 211); "
                                                                          "member.setBooleanAttribute(\"boolAttr\", true); "
                                                                          "member.setByteAttribute(\"byteAttr\", 7); "
                                                                          "member.setDoubleAttribute(\"doubleAttr\", 2.0); "
                                                                          "member.setFloatAttribute(\"floatAttr\", 1.2); "
                                                                          "member.setShortAttribute(\"shortAttr\", 3); "
                                                                          "return member.setStringAttribute(\"strAttr\", \"strAttr\");} "
                                                                          " result=attrs(); ";


                Response response;
                remoteController->executeOnController(response, clusterId, script.str().c_str(), Lang::JAVASCRIPT);
                return response.success;
            }

            bool HazelcastServerFactory::shutdownServer(const remote::Member &member) {
                return remoteController->shutdownMember(clusterId, member.uuid);
            }

            bool HazelcastServerFactory::terminateServer(const remote::Member &member) {
                return remoteController->terminateMember(clusterId, member.uuid);
            }

            const std::string &HazelcastServerFactory::getServerAddress() {
                return serverAddress;
            }

            std::string HazelcastServerFactory::readFromXmlFile(const std::string &xmlFilePath) {
                std::ifstream xmlFile(xmlFilePath.c_str());
                if (!xmlFile) {
                    std::ostringstream out;
                    out << "Failed to read from xml file to at " << xmlFilePath;
                    throw exception::IllegalStateException("HazelcastServerFactory::readFromXmlFile", out.str());
                }

                std::ostringstream buffer;

                buffer << xmlFile.rdbuf();

                xmlFile.close();

                return buffer.str();
            }

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
