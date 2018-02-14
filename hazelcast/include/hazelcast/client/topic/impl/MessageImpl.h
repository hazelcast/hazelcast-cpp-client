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
//
// Created by ihsan demir on 27 May 2016.

#ifndef HAZELCAST_CLIENT_TOPIC_IMPL_MESSAGEIMPL_H_
#define HAZELCAST_CLIENT_TOPIC_IMPL_MESSAGEIMPL_H_

#include <memory>
#include "hazelcast/client/topic/Message.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/TypedData.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                template <typename E>
                class MessageImpl : public Message<E> {
                public:
                    MessageImpl(std::string topicName, std::auto_ptr<E> message, int64_t publishTime,
                                std::auto_ptr<Member> member)
                            : messageObject(message)
                            , publishTime(publishTime)
                            , publishingMember(member)
                            , name(topicName) {
                    }

                    virtual ~MessageImpl() { }

                    const E *getMessageObject() const {
                        return messageObject.get();
                    }

                    virtual std::auto_ptr<E> releaseMessageObject() {
                        return messageObject;
                    }

                    int64_t getPublishTime() const {
                        return publishTime;
                    }

                    const Member *getPublishingMember() const {
                        return publishingMember.get();
                    }

                   const std::string &getSource() const {
                        return name;
                    }

                    std::string getName() {
                        return name;
                    }
                private:
                    std::auto_ptr<E> messageObject;
                    int64_t publishTime;
                    std::auto_ptr<Member> publishingMember;
                    std::string name;
                };
            }
        }

        namespace mixedtype {
            namespace topic {
                namespace impl {
                    class HAZELCAST_API MessageImpl : public client::topic::Message<TypedData> {
                    public:
                        MessageImpl(std::string topicName, std::auto_ptr<TypedData> message, int64_t publishTime,
                                    std::auto_ptr<Member> member)
                                : messageObject(message)
                                , publishTime(publishTime)
                                , publishingMember(member)
                                , name(topicName) {
                        }

                        virtual ~MessageImpl() { }

                        const TypedData *getMessageObject() const {
                            return messageObject.get();
                        }

                        virtual std::auto_ptr<TypedData> releaseMessageObject() {
                            return messageObject;
                        }

                        int64_t getPublishTime() const {
                            return publishTime;
                        }

                        const Member *getPublishingMember() const {
                            return publishingMember.get();
                        }

                        const std::string &getSource() const {
                            return name;
                        }

                        std::string getName() {
                            return name;
                        }
                    private:
                        std::auto_ptr<TypedData> messageObject;
                        int64_t publishTime;
                        std::auto_ptr<Member> publishingMember;
                        std::string name;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_TOPIC_IMPL_MESSAGEIMPL_H_

