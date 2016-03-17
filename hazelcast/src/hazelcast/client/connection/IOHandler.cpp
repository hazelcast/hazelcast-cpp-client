/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 30/12/13.
//

#include "hazelcast/util/Util.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            IOHandler::IOHandler(Connection& connection, IOSelector& ioSelector)
            : ioSelector(ioSelector)
            , connection(connection) {

            }

            void IOHandler::registerSocket() {
                ioSelector.addTask(this);
                ioSelector.wakeUp();
            }

            void IOHandler::registerHandler() {
                if (!connection.live)
                    return;
                Socket const& socket = connection.getSocket();
                ioSelector.addSocket(socket);
            }

            void IOHandler::deRegisterSocket() {
                ioSelector.cancelTask(this);
                ioSelector.removeSocket(connection.getSocket());
            }

            IOHandler::~IOHandler() {

            }

            void IOHandler::handleSocketException(const std::string& message) {
                Address const& address = connection.getRemoteEndpoint();

                size_t len = message.length() + 150;
                char *msg = new char[len];
                util::snprintf(msg, len, "[IOHandler::handleSocketException] Closing socket to endpoint %s:%d, Cause:%s\n",
                        address.getHost().c_str(), address.getPort(), message.c_str());
                util::ILogger::getLogger().getLogger().warning(msg);

                // TODO: This call shall resend pending requests and reregister events, hence it can be off-loaded
                // to another thread in order not to block the critical IO thread
                connection.close();
            }
        }
    }
}

