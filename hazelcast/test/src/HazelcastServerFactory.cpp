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
            std::string HazelcastServerFactory::serverAddress;
            PyObject *HazelcastServerFactory::rcObject;

            void HazelcastServerFactory::init(const std::string &server) {
                PyObject *pName = PyString_FromString("hzrc.client");
                PyObject *pModule = PyImport_Import(pName);
                Py_DECREF(pName);

                if (pModule != NULL) {
                    PyObject *rcAttribute = PyObject_GetAttrString(pModule, "HzRemoteController");
                    /* rcAttribute is a new reference */

                    serverAddress = server;
                    long portNum = 9701;
                    if (rcAttribute && PyCallable_Check(rcAttribute)) {
                        PyObject *pArgs = PyTuple_New(2);
                        PyObject *ip = PyString_FromString(serverAddress.c_str());
                        PyTuple_SetItem(pArgs, 0, ip);
                        PyObject *port = PyInt_FromLong(portNum);
                        PyTuple_SetItem(pArgs, 1, port);
                        rcObject = PyObject_CallObject(rcAttribute, pArgs);
                        Py_DECREF(ip);
                        Py_DECREF(port);
                        if (rcObject == NULL) {
                            std::ostringstream out;
                            out << "Failed to connect to remote controller at " << serverAddress << ":" << portNum;
                            throw exception::IllegalStateException("HazelcastServerFactory::init", out.str());
                        }
                    } else {
                        if (PyErr_Occurred())
                            PyErr_Print();
                        fprintf(stderr, "Cannot find function \"HzRemoteController\"\n");
                    }
                    Py_XDECREF(rcAttribute);
                    Py_DECREF(pModule);
                }
                else {
                    PyErr_Print();
                    fprintf(stderr, "Failed to load \"hzrc.client\"\n");
                }
            }

            HazelcastServerFactory::HazelcastServerFactory(const std::string &serverXmlConfigFilePath)
                    : logger(util::ILogger::getLogger()) {
                PyObject *clusterObject = PyObject_CallMethod(rcObject, const_cast<char *>("createCluster"),
                                                              const_cast<char *>("(ss)"), "3.9",
                                                              serverXmlConfigFilePath.c_str());
                if (clusterObject == NULL) {
                    std::ostringstream out;
                    out << "Failed to create cluster with xml file path: " << serverXmlConfigFilePath;
                    throw exception::IllegalStateException("HazelcastServerFactory::HazelcastServerFactory", out.str());
                }

                PyObject *clusterUUID = PyObject_GetAttrString(clusterObject, "id");
                if (clusterUUID == NULL) {
                    Py_DECREF(clusterObject);
                    throw exception::IllegalStateException("HazelcastServerFactory::init",
                                                           "Could not retrieve cluster id as string");
                }

                clusterId = PyString_AsString(clusterUUID);
                Py_DECREF(clusterUUID);
            }

            HazelcastServerFactory::~HazelcastServerFactory() {
                PyObject *resultObj = PyObject_CallMethod(rcObject, const_cast<char *>("shutdownCluster"),
                                                              const_cast<char *>("(s)"), clusterId.c_str());

                if (resultObj == NULL || resultObj == Py_False) {
                    std::ostringstream out;
                    out << "Failed to shutdown the cluster with id " << clusterId;
                    logger.severe(out.str());
                }

                Py_DECREF(resultObj);
                //Py_DECREF(rcObject);
            }

            HazelcastServerFactory::MemberInfo HazelcastServerFactory::startServer() {
                PyObject *memberObject = PyObject_CallMethod(rcObject, const_cast<char *>("startMember"),
                                                             const_cast<char *>("(s)"), clusterId.c_str());
                if (memberObject == NULL) {
                    std::ostringstream out;
                    out << "Failed to start member at cluster " << clusterId;
                    throw exception::IllegalStateException("HazelcastServerFactory::init", out.str());
                }

                PyObject *uuidObj = PyObject_GetAttrString(memberObject, "uuid");
                PyObject *hostObj = PyObject_GetAttrString(memberObject, "host");
                PyObject *portStringObj = PyString_FromString("port");
                PyObject *portObj = PyObject_GenericGetAttr(memberObject, portStringObj);

                HazelcastServerFactory::MemberInfo memberInfo(PyString_AsString(uuidObj), PyString_AsString(hostObj),
                                                              PyInt_AsLong(portObj));


                Py_DECREF(memberObject);
                Py_DECREF(uuidObj);
                Py_DECREF(hostObj);
                Py_DECREF(portStringObj);
                Py_DECREF(portObj);

                return memberInfo;
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

            bool HazelcastServerFactory::shutdownServer(const MemberInfo &member) {
                PyObject *resultObj = PyObject_CallMethod(rcObject, const_cast<char *>("shutdownMember"),
                                                             const_cast<char *>("(ss)"), clusterId.c_str(),
                                                          member.getUuid().c_str());

                if (resultObj == NULL || resultObj == Py_False) {
                    std::ostringstream out;
                    out << "Failed to shutdown the member " << member;
                    logger.severe(out.str());
                    Py_DECREF(resultObj);
                    return false;
                }

                Py_DECREF(resultObj);
                return true;

            }

            const std::string &HazelcastServerFactory::getServerAddress() {
                return serverAddress;
            }

            HazelcastServerFactory::MemberInfo::MemberInfo(const string &uuid, const string &ip, int port) : uuid(uuid),
                                                                                                             ip(ip),
                                                                                                             port(port) {}

            ostream &operator<<(ostream &os, const HazelcastServerFactory::MemberInfo &info) {
                os << "MemberInfo{uuid: " << info.uuid << " ip: " << info.ip << " port: " << info.port << "}";
                return os;
            }

            HazelcastServerFactory::MemberInfo::MemberInfo() : port(-1) {}

            const string &HazelcastServerFactory::MemberInfo::getUuid() const {
                return uuid;
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
