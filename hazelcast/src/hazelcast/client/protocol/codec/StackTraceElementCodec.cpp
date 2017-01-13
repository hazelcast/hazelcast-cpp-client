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
// Created by ihsan demir on 5/11/15.
//

#include "hazelcast/client/protocol/codec/StackTraceElementCodec.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/protocol/codec/StackTraceElement.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                StackTraceElement StackTraceElementCodec::decode(ClientMessage &clientMessage) {
                    std::string className = clientMessage.getStringUtf8();
                    std::string methodName = clientMessage.getStringUtf8();
                    std::auto_ptr<std::string> fileName = clientMessage.getNullable<std::string>();
                    int32_t lineNumber = clientMessage.getInt32();
                    
                    return StackTraceElement(className, methodName, fileName, lineNumber);
                }
            }
        }
    }
}
