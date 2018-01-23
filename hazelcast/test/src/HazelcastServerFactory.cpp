/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 8/26/13.

#include <iostream>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include <Python.h>

#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"

#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/exception/IllegalStateException.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            HazelcastServerFactory::HazelcastServerFactory(const std::string &serverXmlConfigFilePath)
                    : logger(util::ILogger::getLogger()) {
/*
                try {
                    rcClient->createCluster(cluster, "", serverXmlConfigFilePath);
                } catch (TException &tx) {
                    std::ostringstream out;
                    out << "The test environment failed to initialize. Could not connect create cluster with server xml file "
                            << serverXmlConfigFilePath << ". " << tx.what();
                    logger.severe(out.str());
                    throw;
                }
*/
            }

            HazelcastServerFactory::~HazelcastServerFactory() {
/*
                try {
                    rcClient->shutdownCluster(cluster.id);
                } catch (TException &tx) {
                    std::ostringstream out;
                    out << "Failed to shutdown the cluster with id " << cluster.id << tx.what();
                    logger.severe(out.str());
                }
*/
            }

            void HazelcastServerFactory::startServer(const std::string &member) {
/*
                rcClient->startMember(member, cluster.id);
*/
            }

            void HazelcastServerFactory::setAttributes(int memberStartOrder) {
/*
                Response response;
*/
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

/*
                rcClient->executeOnController(response, cluster.id, script.str(), Lang::JAVASCRIPT);
*/
            }

            void HazelcastServerFactory::shutdownServer(const std::string &member) {
/*
                rcClient->shutdownMember(cluster.id, member.uuid);
*/
            }

            const std::string &HazelcastServerFactory::getServerAddress() const {
                return serverAddress;
            }

            void HazelcastServerFactory::init(const std::string &serverAddress) {
                PyObject *pName, *pModule, *pFunc;


                Py_Initialize();

                pName = PyString_FromString("hzrc.client");
                pModule = PyImport_Import(pName);
                Py_DECREF(pName);

                if (pModule != NULL) {
                    pFunc = PyObject_GetAttrString(pModule, "HzRemoteController");
                    /* pFunc is a new reference */

                    if (pFunc && PyCallable_Check(pFunc)) {
                        PyObject *pArgs = PyTuple_New(2);
                        PyObject *ip = PyString_FromString("127.0.0.1");
                        PyTuple_SetItem(pArgs, 0, ip);
                        PyObject *port = PyInt_FromLong(9701);
                        PyTuple_SetItem(pArgs, 1, port);
                        PyObject *rcObject = PyObject_CallObject(pFunc, pArgs);
                        //Py_DECREF(pArgs);
                        if (rcObject != NULL) {
                            PyObject *clusterObject = PyObject_CallMethod(pFunc, const_cast<char *>("startMember"),
                                                         const_cast<char *>("ss"), "3.9",
                                                         "java/src/main/resources/hazelcast.xml");
                            if (clusterObject == NULL) {
                                throw exception::IllegalStateException("Failed to create cluster");
                            }

/*
                            PyObject *clusterId = PyObject_GetAttrString(clusterObject, "id");
*/

/*
                            PyObject *startServerMethod = PyObject_GetAttrString(pFunc, "startMember");
                            if (startServerMethod != NULL) {
                                PyObject *args = PyTuple_New(1);
                                PyTuple_SetItem(args, 0, clusterId);
                                pValue = PyObject_CallObject(startServerMethod, args);
                            }
*/

                            Py_DECREF(rcObject);
                        } else {
                            Py_DECREF(rcObject);
                            Py_DECREF(pModule);
                            PyErr_Print();
                            fprintf(stderr,"Call failed\n");
                        }

                        Py_DECREF(ip);
                        Py_DECREF(port);
                    }
                    else {
                        if (PyErr_Occurred())
                            PyErr_Print();
                        fprintf(stderr, "Cannot find function \"HzRemoteController\"\n");
                    }
                    Py_XDECREF(pFunc);
                    Py_DECREF(pModule);
                }
                else {
                    PyErr_Print();
                    fprintf(stderr, "Failed to load \"hzrc.client\"\n");
                }
                Py_Finalize();

/*
                PyRun_SimpleString("from hzrc.client import HzRemoteController\n"
                        "import logging\n"
                        "\n"
                        "logging.basicConfig(format='%(asctime)s%(msecs)03d [%(name)s] %(levelname)s: %(message)s', datefmt=\"%H:%M%:%S,\")\n"
                        "\n"
                        "logging.getLogger().setLevel(logging.INFO)\n"
                        "\n"
                        "rc = HzRemoteController('127.0.0.1', 9701)");
*/
                Py_Finalize();
/*
                boost::shared_ptr<TTransport> socket(new TSocket(serverAddress, 9701));
                boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
                boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
                rcClient.reset(new RemoteControllerClient(protocol));
                try {
                    transport->open();
                } catch (TException &tx) {
                    std::ostringstream out;
                    out << "The test environment failed to initialize. Could not connect to remote controller. " <<
                        tx.what();
                    util::ILogger::getLogger().severe(out.str());
                    throw;
                }
*/
            }

/*
            boost::shared_ptr<RemoteControllerClient> HazelcastServerFactory::rcClient;
*/

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
