/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/protocol/ResponseMessageConst.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/protocol/codec/AddressCodec.h"
#include "hazelcast/client/protocol/codec/DataEntryViewCodec.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/protocol/codec/MemberCodec.h"
#include "hazelcast/client/protocol/codec/StackTraceElementCodec.h"
#include "hazelcast/client/protocol/codec/UUIDCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API ClientMessageType {

                    HZ_CLIENT_AUTHENTICATION = 0x0002,
                    HZ_CLIENT_AUTHENTICATIONCUSTOM = 0x0003,
                    HZ_CLIENT_ADDMEMBERSHIPLISTENER = 0x0004,
                    HZ_CLIENT_CREATEPROXY = 0x0005,
                    HZ_CLIENT_DESTROYPROXY = 0x0006,
                    HZ_CLIENT_GETPARTITIONS = 0x0008,
                    HZ_CLIENT_REMOVEALLLISTENERS = 0x0009,
                    HZ_CLIENT_ADDPARTITIONLOSTLISTENER = 0x000a,
                    HZ_CLIENT_REMOVEPARTITIONLOSTLISTENER = 0x000b,
                    HZ_CLIENT_GETDISTRIBUTEDOBJECTS = 0x000c,
                    HZ_CLIENT_ADDDISTRIBUTEDOBJECTLISTENER = 0x000d,
                    HZ_CLIENT_REMOVEDISTRIBUTEDOBJECTLISTENER = 0x000e,
                    HZ_CLIENT_PING = 0x000f,
                    HZ_CLIENT_STATISTICS = 0x0010,
                    HZ_CLIENT_DEPLOYCLASSES = 0x0011,
                    HZ_CLIENT_ADDPARTITIONLISTENER = 0x0012,
                    HZ_CLIENT_CREATEPROXIES = 0x0013
                };
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API ExecutorServiceMessageType {

                    HZ_EXECUTORSERVICE_SHUTDOWN = 0x0901,
                    HZ_EXECUTORSERVICE_ISSHUTDOWN = 0x0902,
                    HZ_EXECUTORSERVICE_CANCELONPARTITION = 0x0903,
                    HZ_EXECUTORSERVICE_CANCELONADDRESS = 0x0904,
                    HZ_EXECUTORSERVICE_SUBMITTOPARTITION = 0x0905,
                    HZ_EXECUTORSERVICE_SUBMITTOADDRESS = 0x0906
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API FlakeIdGeneratorMessageType {

                    HZ_FLAKEIDGENERATOR_NEWIDBATCH = 0x1f01
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API ListMessageType {

                    HZ_LIST_SIZE = 0x0501,
                    HZ_LIST_CONTAINS = 0x0502,
                    HZ_LIST_CONTAINSALL = 0x0503,
                    HZ_LIST_ADD = 0x0504,
                    HZ_LIST_REMOVE = 0x0505,
                    HZ_LIST_ADDALL = 0x0506,
                    HZ_LIST_COMPAREANDREMOVEALL = 0x0507,
                    HZ_LIST_COMPAREANDRETAINALL = 0x0508,
                    HZ_LIST_CLEAR = 0x0509,
                    HZ_LIST_GETALL = 0x050a,
                    HZ_LIST_ADDLISTENER = 0x050b,
                    HZ_LIST_REMOVELISTENER = 0x050c,
                    HZ_LIST_ISEMPTY = 0x050d,
                    HZ_LIST_ADDALLWITHINDEX = 0x050e,
                    HZ_LIST_GET = 0x050f,
                    HZ_LIST_SET = 0x0510,
                    HZ_LIST_ADDWITHINDEX = 0x0511,
                    HZ_LIST_REMOVEWITHINDEX = 0x0512,
                    HZ_LIST_LASTINDEXOF = 0x0513,
                    HZ_LIST_INDEXOF = 0x0514,
                    HZ_LIST_SUB = 0x0515,
                    HZ_LIST_ITERATOR = 0x0516,
                    HZ_LIST_LISTITERATOR = 0x0517
                };
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API MapMessageType {

                    HZ_MAP_PUT = 0x0101,
                    HZ_MAP_GET = 0x0102,
                    HZ_MAP_REMOVE = 0x0103,
                    HZ_MAP_REPLACE = 0x0104,
                    HZ_MAP_REPLACEIFSAME = 0x0105,
                    HZ_MAP_CONTAINSKEY = 0x0109,
                    HZ_MAP_CONTAINSVALUE = 0x010a,
                    HZ_MAP_REMOVEIFSAME = 0x010b,
                    HZ_MAP_DELETE = 0x010c,
                    HZ_MAP_FLUSH = 0x010d,
                    HZ_MAP_TRYREMOVE = 0x010e,
                    HZ_MAP_TRYPUT = 0x010f,
                    HZ_MAP_PUTTRANSIENT = 0x0110,
                    HZ_MAP_PUTIFABSENT = 0x0111,
                    HZ_MAP_SET = 0x0112,
                    HZ_MAP_LOCK = 0x0113,
                    HZ_MAP_TRYLOCK = 0x0114,
                    HZ_MAP_ISLOCKED = 0x0115,
                    HZ_MAP_UNLOCK = 0x0116,
                    HZ_MAP_ADDINTERCEPTOR = 0x0117,
                    HZ_MAP_REMOVEINTERCEPTOR = 0x0118,
                    HZ_MAP_ADDENTRYLISTENERTOKEYWITHPREDICATE = 0x0119,
                    HZ_MAP_ADDENTRYLISTENERWITHPREDICATE = 0x011a,
                    HZ_MAP_ADDENTRYLISTENERTOKEY = 0x011b,
                    HZ_MAP_ADDENTRYLISTENER = 0x011c,
                    HZ_MAP_ADDNEARCACHEENTRYLISTENER = 0x011d,
                    HZ_MAP_REMOVEENTRYLISTENER = 0x011e,
                    HZ_MAP_ADDPARTITIONLOSTLISTENER = 0x011f,
                    HZ_MAP_REMOVEPARTITIONLOSTLISTENER = 0x0120,
                    HZ_MAP_GETENTRYVIEW = 0x0121,
                    HZ_MAP_EVICT = 0x0122,
                    HZ_MAP_EVICTALL = 0x0123,
                    HZ_MAP_LOADALL = 0x0124,
                    HZ_MAP_LOADGIVENKEYS = 0x0125,
                    HZ_MAP_KEYSET = 0x0126,
                    HZ_MAP_GETALL = 0x0127,
                    HZ_MAP_VALUES = 0x0128,
                    HZ_MAP_ENTRYSET = 0x0129,
                    HZ_MAP_KEYSETWITHPREDICATE = 0x012a,
                    HZ_MAP_VALUESWITHPREDICATE = 0x012b,
                    HZ_MAP_ENTRIESWITHPREDICATE = 0x012c,
                    HZ_MAP_ADDINDEX = 0x012d,
                    HZ_MAP_SIZE = 0x012e,
                    HZ_MAP_ISEMPTY = 0x012f,
                    HZ_MAP_PUTALL = 0x0130,
                    HZ_MAP_CLEAR = 0x0131,
                    HZ_MAP_EXECUTEONKEY = 0x0132,
                    HZ_MAP_SUBMITTOKEY = 0x0133,
                    HZ_MAP_EXECUTEONALLKEYS = 0x0134,
                    HZ_MAP_EXECUTEWITHPREDICATE = 0x0135,
                    HZ_MAP_EXECUTEONKEYS = 0x0136,
                    HZ_MAP_FORCEUNLOCK = 0x0137,
                    HZ_MAP_KEYSETWITHPAGINGPREDICATE = 0x0138,
                    HZ_MAP_VALUESWITHPAGINGPREDICATE = 0x0139,
                    HZ_MAP_ENTRIESWITHPAGINGPREDICATE = 0x013a,
                    HZ_MAP_CLEARNEARCACHE = 0x013b,
                    HZ_MAP_FETCHKEYS = 0x013c,
                    HZ_MAP_FETCHENTRIES = 0x013d,
                    HZ_MAP_AGGREGATE = 0x013e,
                    HZ_MAP_AGGREGATEWITHPREDICATE = 0x013f,
                    HZ_MAP_PROJECT = 0x0140,
                    HZ_MAP_PROJECTWITHPREDICATE = 0x0141,
                    HZ_MAP_FETCHNEARCACHEINVALIDATIONMETADATA = 0x0142,
                    HZ_MAP_ASSIGNANDGETUUIDS = 0x0143,
                    HZ_MAP_REMOVEALL = 0x0144,
                    HZ_MAP_ADDNEARCACHEINVALIDATIONLISTENER = 0x0145,
                    HZ_MAP_FETCHWITHQUERY = 0x0146,
                    HZ_MAP_EVENTJOURNALSUBSCRIBE = 0x0147,
                    HZ_MAP_EVENTJOURNALREAD = 0x0148,
                    HZ_MAP_SETTTL = 0x0149,
                    HZ_MAP_PUTWITHMAXIDLE = 0x014a,
                    HZ_MAP_PUTTRANSIENTWITHMAXIDLE = 0x014b,
                    HZ_MAP_PUTIFABSENTWITHMAXIDLE = 0x014c,
                    HZ_MAP_SETWITHMAXIDLE = 0x014d
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API MultiMapMessageType {

                    HZ_MULTIMAP_PUT = 0x0201,
                    HZ_MULTIMAP_GET = 0x0202,
                    HZ_MULTIMAP_REMOVE = 0x0203,
                    HZ_MULTIMAP_KEYSET = 0x0204,
                    HZ_MULTIMAP_VALUES = 0x0205,
                    HZ_MULTIMAP_ENTRYSET = 0x0206,
                    HZ_MULTIMAP_CONTAINSKEY = 0x0207,
                    HZ_MULTIMAP_CONTAINSVALUE = 0x0208,
                    HZ_MULTIMAP_CONTAINSENTRY = 0x0209,
                    HZ_MULTIMAP_SIZE = 0x020a,
                    HZ_MULTIMAP_CLEAR = 0x020b,
                    HZ_MULTIMAP_VALUECOUNT = 0x020c,
                    HZ_MULTIMAP_ADDENTRYLISTENERTOKEY = 0x020d,
                    HZ_MULTIMAP_ADDENTRYLISTENER = 0x020e,
                    HZ_MULTIMAP_REMOVEENTRYLISTENER = 0x020f,
                    HZ_MULTIMAP_LOCK = 0x0210,
                    HZ_MULTIMAP_TRYLOCK = 0x0211,
                    HZ_MULTIMAP_ISLOCKED = 0x0212,
                    HZ_MULTIMAP_UNLOCK = 0x0213,
                    HZ_MULTIMAP_FORCEUNLOCK = 0x0214,
                    HZ_MULTIMAP_REMOVEENTRY = 0x0215,
                    HZ_MULTIMAP_DELETE = 0x0216
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API PNCounterMessageType {

                    HZ_PNCOUNTER_GET = 0x2001,
                    HZ_PNCOUNTER_ADD = 0x2002,
                    HZ_PNCOUNTER_GETCONFIGUREDREPLICACOUNT = 0x2003
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API QueueMessageType {

                    HZ_QUEUE_OFFER = 0x0301,
                    HZ_QUEUE_PUT = 0x0302,
                    HZ_QUEUE_SIZE = 0x0303,
                    HZ_QUEUE_REMOVE = 0x0304,
                    HZ_QUEUE_POLL = 0x0305,
                    HZ_QUEUE_TAKE = 0x0306,
                    HZ_QUEUE_PEEK = 0x0307,
                    HZ_QUEUE_ITERATOR = 0x0308,
                    HZ_QUEUE_DRAINTO = 0x0309,
                    HZ_QUEUE_DRAINTOMAXSIZE = 0x030a,
                    HZ_QUEUE_CONTAINS = 0x030b,
                    HZ_QUEUE_CONTAINSALL = 0x030c,
                    HZ_QUEUE_COMPAREANDREMOVEALL = 0x030d,
                    HZ_QUEUE_COMPAREANDRETAINALL = 0x030e,
                    HZ_QUEUE_CLEAR = 0x030f,
                    HZ_QUEUE_ADDALL = 0x0310,
                    HZ_QUEUE_ADDLISTENER = 0x0311,
                    HZ_QUEUE_REMOVELISTENER = 0x0312,
                    HZ_QUEUE_REMAININGCAPACITY = 0x0313,
                    HZ_QUEUE_ISEMPTY = 0x0314
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API ReplicatedMapMessageType {

                    HZ_REPLICATEDMAP_PUT = 0x0e01,
                    HZ_REPLICATEDMAP_SIZE = 0x0e02,
                    HZ_REPLICATEDMAP_ISEMPTY = 0x0e03,
                    HZ_REPLICATEDMAP_CONTAINSKEY = 0x0e04,
                    HZ_REPLICATEDMAP_CONTAINSVALUE = 0x0e05,
                    HZ_REPLICATEDMAP_GET = 0x0e06,
                    HZ_REPLICATEDMAP_REMOVE = 0x0e07,
                    HZ_REPLICATEDMAP_PUTALL = 0x0e08,
                    HZ_REPLICATEDMAP_CLEAR = 0x0e09,
                    HZ_REPLICATEDMAP_ADDENTRYLISTENERTOKEYWITHPREDICATE = 0x0e0a,
                    HZ_REPLICATEDMAP_ADDENTRYLISTENERWITHPREDICATE = 0x0e0b,
                    HZ_REPLICATEDMAP_ADDENTRYLISTENERTOKEY = 0x0e0c,
                    HZ_REPLICATEDMAP_ADDENTRYLISTENER = 0x0e0d,
                    HZ_REPLICATEDMAP_REMOVEENTRYLISTENER = 0x0e0e,
                    HZ_REPLICATEDMAP_KEYSET = 0x0e0f,
                    HZ_REPLICATEDMAP_VALUES = 0x0e10,
                    HZ_REPLICATEDMAP_ENTRYSET = 0x0e11,
                    HZ_REPLICATEDMAP_ADDNEARCACHEENTRYLISTENER = 0x0e12
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API RingbufferMessageType {

                    HZ_RINGBUFFER_SIZE = 0x1901,
                    HZ_RINGBUFFER_TAILSEQUENCE = 0x1902,
                    HZ_RINGBUFFER_HEADSEQUENCE = 0x1903,
                    HZ_RINGBUFFER_CAPACITY = 0x1904,
                    HZ_RINGBUFFER_REMAININGCAPACITY = 0x1905,
                    HZ_RINGBUFFER_ADD = 0x1906,
                    HZ_RINGBUFFER_READONE = 0x1908,
                    HZ_RINGBUFFER_ADDALL = 0x1909,
                    HZ_RINGBUFFER_READMANY = 0x190a
                };
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API SetMessageType {

                    HZ_SET_SIZE = 0x0601,
                    HZ_SET_CONTAINS = 0x0602,
                    HZ_SET_CONTAINSALL = 0x0603,
                    HZ_SET_ADD = 0x0604,
                    HZ_SET_REMOVE = 0x0605,
                    HZ_SET_ADDALL = 0x0606,
                    HZ_SET_COMPAREANDREMOVEALL = 0x0607,
                    HZ_SET_COMPAREANDRETAINALL = 0x0608,
                    HZ_SET_CLEAR = 0x0609,
                    HZ_SET_GETALL = 0x060a,
                    HZ_SET_ADDLISTENER = 0x060b,
                    HZ_SET_REMOVELISTENER = 0x060c,
                    HZ_SET_ISEMPTY = 0x060d
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API TopicMessageType {

                    HZ_TOPIC_PUBLISH = 0x0401,
                    HZ_TOPIC_ADDMESSAGELISTENER = 0x0402,
                    HZ_TOPIC_REMOVEMESSAGELISTENER = 0x0403
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API TransactionMessageType {

                    HZ_TRANSACTION_COMMIT = 0x1701,
                    HZ_TRANSACTION_CREATE = 0x1702,
                    HZ_TRANSACTION_ROLLBACK = 0x1703
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API TransactionalListMessageType {

                    HZ_TRANSACTIONALLIST_ADD = 0x1301,
                    HZ_TRANSACTIONALLIST_REMOVE = 0x1302,
                    HZ_TRANSACTIONALLIST_SIZE = 0x1303
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API TransactionalMapMessageType {

                    HZ_TRANSACTIONALMAP_CONTAINSKEY = 0x1001,
                    HZ_TRANSACTIONALMAP_GET = 0x1002,
                    HZ_TRANSACTIONALMAP_GETFORUPDATE = 0x1003,
                    HZ_TRANSACTIONALMAP_SIZE = 0x1004,
                    HZ_TRANSACTIONALMAP_ISEMPTY = 0x1005,
                    HZ_TRANSACTIONALMAP_PUT = 0x1006,
                    HZ_TRANSACTIONALMAP_SET = 0x1007,
                    HZ_TRANSACTIONALMAP_PUTIFABSENT = 0x1008,
                    HZ_TRANSACTIONALMAP_REPLACE = 0x1009,
                    HZ_TRANSACTIONALMAP_REPLACEIFSAME = 0x100a,
                    HZ_TRANSACTIONALMAP_REMOVE = 0x100b,
                    HZ_TRANSACTIONALMAP_DELETE = 0x100c,
                    HZ_TRANSACTIONALMAP_REMOVEIFSAME = 0x100d,
                    HZ_TRANSACTIONALMAP_KEYSET = 0x100e,
                    HZ_TRANSACTIONALMAP_KEYSETWITHPREDICATE = 0x100f,
                    HZ_TRANSACTIONALMAP_VALUES = 0x1010,
                    HZ_TRANSACTIONALMAP_VALUESWITHPREDICATE = 0x1011,
                    HZ_TRANSACTIONALMAP_CONTAINSVALUE = 0x1012
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API TransactionalMultiMapMessageType {

                    HZ_TRANSACTIONALMULTIMAP_PUT = 0x1101,
                    HZ_TRANSACTIONALMULTIMAP_GET = 0x1102,
                    HZ_TRANSACTIONALMULTIMAP_REMOVE = 0x1103,
                    HZ_TRANSACTIONALMULTIMAP_REMOVEENTRY = 0x1104,
                    HZ_TRANSACTIONALMULTIMAP_VALUECOUNT = 0x1105,
                    HZ_TRANSACTIONALMULTIMAP_SIZE = 0x1106
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API TransactionalQueueMessageType {

                    HZ_TRANSACTIONALQUEUE_OFFER = 0x1401,
                    HZ_TRANSACTIONALQUEUE_TAKE = 0x1402,
                    HZ_TRANSACTIONALQUEUE_POLL = 0x1403,
                    HZ_TRANSACTIONALQUEUE_PEEK = 0x1404,
                    HZ_TRANSACTIONALQUEUE_SIZE = 0x1405
                };
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API TransactionalSetMessageType {

                    HZ_TRANSACTIONALSET_ADD = 0x1201,
                    HZ_TRANSACTIONALSET_REMOVE = 0x1202,
                    HZ_TRANSACTIONALSET_SIZE = 0x1203
                };
            }
        }
    }
}


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class Member;

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ClientAddMembershipListenerCodec {
                public:
                    static const ClientMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Client.AddMembershipListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            bool localOnly);

                    static int32_t calculateDataSize(
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleMemberEventV10(const Member &member, const int32_t &eventType) = 0;


                        virtual void handleMemberListEventV10(const std::vector<Member> &members) = 0;


                        virtual void
                        handleMemberAttributeChangeEventV10(const std::string &uuid, const std::string &key,
                                                            const int32_t &operationType,
                                                            std::unique_ptr<std::string> &value) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    ClientAddMembershipListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        class Address;

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ClientAddPartitionListenerCodec {
                public:
                    static const ClientMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Client.AddPartitionListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest();

                    static int32_t calculateDataSize();
                    //************************ REQUEST ENDS ********************************************************************//



                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handlePartitionsEventV15(
                                const std::vector<std::pair<Address, std::vector<int32_t> > > &partitions,
                                const int32_t &partitionStateVersion) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    ClientAddPartitionListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        class Address;

        class Member;

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ClientAuthenticationCodec {
                public:
                    static const ClientMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Client.Authentication";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &username,
                            const std::string &password,
                            const std::string *uuid,
                            const std::string *ownerUuid,
                            bool isOwnerConnection,
                            const std::string &clientType,
                            uint8_t serializationVersion,
                            const std::string &clientHazelcastVersion);

                    static int32_t calculateDataSize(
                            const std::string &username,
                            const std::string &password,
                            const std::string *uuid,
                            const std::string *ownerUuid,
                            bool isOwnerConnection,
                            const std::string &clientType,
                            uint8_t serializationVersion,
                            const std::string &clientHazelcastVersion);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        uint8_t status;

                        std::unique_ptr<Address> address;

                        std::unique_ptr<std::string> uuid;

                        std::unique_ptr<std::string> ownerUuid;

                        uint8_t serializationVersion;

                        std::string serverHazelcastVersion;
                        bool serverHazelcastVersionExist;
                        std::unique_ptr<std::vector<Member> > clientUnregisteredMembers;
                        bool clientUnregisteredMembersExist;

                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ClientAuthenticationCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        class Address;

        class Member;

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ClientAuthenticationCustomCodec {
                public:
                    static const ClientMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Client.AuthenticationCustom";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const serialization::pimpl::Data &credentials,
                            const std::string *uuid,
                            const std::string *ownerUuid,
                            bool isOwnerConnection,
                            const std::string &clientType,
                            uint8_t serializationVersion,
                            const std::string &clientHazelcastVersion);

                    static int32_t calculateDataSize(
                            const serialization::pimpl::Data &credentials,
                            const std::string *uuid,
                            const std::string *ownerUuid,
                            bool isOwnerConnection,
                            const std::string &clientType,
                            uint8_t serializationVersion,
                            const std::string &clientHazelcastVersion);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        uint8_t status;

                        std::unique_ptr<Address> address;

                        std::unique_ptr<std::string> uuid;

                        std::unique_ptr<std::string> ownerUuid;

                        uint8_t serializationVersion;

                        std::string serverHazelcastVersion;
                        bool serverHazelcastVersionExist;
                        std::unique_ptr<std::vector<Member> > clientUnregisteredMembers;
                        bool clientUnregisteredMembersExist;

                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ClientAuthenticationCustomCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        class Address;

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ClientCreateProxyCodec {
                public:
                    static const ClientMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Client.CreateProxy";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &serviceName,
                            const Address &target);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &serviceName,
                            const Address &target);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    ClientCreateProxyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ClientDestroyProxyCodec {
                public:
                    static const ClientMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Client.DestroyProxy";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &serviceName);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &serviceName);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    ClientDestroyProxyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ClientGetPartitionsCodec {
                public:
                    static const ClientMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Client.GetPartitions";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest();

                    static int32_t calculateDataSize();
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<Address, std::vector<int32_t> > > partitions;

                        int32_t partitionStateVersion;
                        bool partitionStateVersionExist;

                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ClientGetPartitionsCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ClientPingCodec {
                public:
                    static const ClientMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Client.Ping";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest();

                    static int32_t calculateDataSize();
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    ClientPingCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ClientStatisticsCodec {
                public:
                    static const ClientMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Client.Statistics";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &stats);

                    static int32_t calculateDataSize(
                            const std::string &stats);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    ClientStatisticsCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

namespace hazelcast {
    namespace client {
        class Address;

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ExecutorServiceCancelOnAddressCodec {
                public:
                    static const ExecutorServiceMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ExecutorService.CancelOnAddress";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &uuid,
                            const Address &address,
                            bool interrupt);

                    static int32_t calculateDataSize(
                            const std::string &uuid,
                            const Address &address,
                            bool interrupt);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ExecutorServiceCancelOnAddressCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ExecutorServiceCancelOnPartitionCodec {
                public:
                    static const ExecutorServiceMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ExecutorService.CancelOnPartition";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &uuid,
                            int32_t partitionId,
                            bool interrupt);

                    static int32_t calculateDataSize(
                            const std::string &uuid,
                            int32_t partitionId,
                            bool interrupt);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ExecutorServiceCancelOnPartitionCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ExecutorServiceIsShutdownCodec {
                public:
                    static const ExecutorServiceMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ExecutorService.IsShutdown";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ExecutorServiceIsShutdownCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ExecutorServiceShutdownCodec {
                public:
                    static const ExecutorServiceMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ExecutorService.Shutdown";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    ExecutorServiceShutdownCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        class Address;

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ExecutorServiceSubmitToAddressCodec {
                public:
                    static const ExecutorServiceMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ExecutorService.SubmitToAddress";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &uuid,
                            const serialization::pimpl::Data &callable,
                            const Address &address);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &uuid,
                            const serialization::pimpl::Data &callable,
                            const Address &address);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ExecutorServiceSubmitToAddressCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ExecutorServiceSubmitToPartitionCodec {
                public:
                    static const ExecutorServiceMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ExecutorService.SubmitToPartition";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &uuid,
                            const serialization::pimpl::Data &callable,
                            int32_t partitionId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &uuid,
                            const serialization::pimpl::Data &callable,
                            int32_t partitionId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ExecutorServiceSubmitToPartitionCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API FlakeIdGeneratorNewIdBatchCodec {
                public:
                    static const FlakeIdGeneratorMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "FlakeIdGenerator.NewIdBatch";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int32_t batchSize);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int32_t batchSize);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int64_t base;

                        int64_t increment;

                        int32_t batchSize;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    FlakeIdGeneratorNewIdBatchCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListAddAllCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.AddAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &valueList);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &valueList);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListAddAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListAddAllWithIndexCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.AddAllWithIndex";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int32_t index,
                            const std::vector<serialization::pimpl::Data> &valueList);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int32_t index,
                            const std::vector<serialization::pimpl::Data> &valueList);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListAddAllWithIndexCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListAddCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.Add";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListAddCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListAddListenerCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.AddListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            bool includeValue,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            bool includeValue,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void
                        handleItemEventV10(std::unique_ptr<serialization::pimpl::Data> &item, const std::string &uuid,
                                           const int32_t &eventType) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    ListAddListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListAddWithIndexCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.AddWithIndex";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int32_t index,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int32_t index,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    ListAddWithIndexCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListClearCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.Clear";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    ListClearCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListCompareAndRemoveAllCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.CompareAndRemoveAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &values);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &values);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListCompareAndRemoveAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListCompareAndRetainAllCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.CompareAndRetainAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &values);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &values);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListCompareAndRetainAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListContainsAllCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.ContainsAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &values);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &values);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListContainsAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListContainsCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.Contains";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListContainsCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListGetAllCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.GetAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListGetAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListGetCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.Get";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int32_t index);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int32_t index);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListGetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListIndexOfCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.IndexOf";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListIndexOfCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListIsEmptyCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.IsEmpty";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListIsEmptyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListLastIndexOfCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.LastIndexOf";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListLastIndexOfCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListRemoveCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.Remove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListRemoveListenerCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.RemoveListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &registrationId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &registrationId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListRemoveListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListRemoveWithIndexCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.RemoveWithIndex";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int32_t index);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int32_t index);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListRemoveWithIndexCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListSetCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.Set";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int32_t index,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int32_t index,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListSetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListSizeCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListSubCodec {
                public:
                    static const ListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "List.Sub";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int32_t from,
                            int32_t to);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int32_t from,
                            int32_t to);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ListSubCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapAddEntryListenerCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.AddEntryListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            bool includeValue,
                            int32_t listenerFlags,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            bool includeValue,
                            int32_t listenerFlags,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                         const int32_t &eventType, const std::string &uuid,
                                                         const int32_t &numberOfAffectedEntries) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    MapAddEntryListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapAddEntryListenerToKeyCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.AddEntryListenerToKey";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            bool includeValue,
                            int32_t listenerFlags,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            bool includeValue,
                            int32_t listenerFlags,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                         const int32_t &eventType, const std::string &uuid,
                                                         const int32_t &numberOfAffectedEntries) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    MapAddEntryListenerToKeyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapAddEntryListenerWithPredicateCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.AddEntryListenerWithPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate,
                            bool includeValue,
                            int32_t listenerFlags,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate,
                            bool includeValue,
                            int32_t listenerFlags,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                         const int32_t &eventType, const std::string &uuid,
                                                         const int32_t &numberOfAffectedEntries) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    MapAddEntryListenerWithPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapAddIndexCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.AddIndex";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &attribute,
                            bool ordered);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &attribute,
                            bool ordered);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapAddIndexCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapAddInterceptorCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.AddInterceptor";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &interceptor);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &interceptor);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapAddInterceptorCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapAddNearCacheEntryListenerCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.AddNearCacheEntryListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int32_t listenerFlags,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int32_t listenerFlags,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void
                        handleIMapInvalidationEventV10(std::unique_ptr<serialization::pimpl::Data> &key) = 0;


                        virtual void handleIMapInvalidationEventV14(std::unique_ptr<serialization::pimpl::Data> &key,
                                                                    const std::string &sourceUuid,
                                                                    const util::UUID &partitionUuid,
                                                                    const int64_t &sequence) = 0;


                        virtual void
                        handleIMapBatchInvalidationEventV10(const std::vector<serialization::pimpl::Data> &keys) = 0;


                        virtual void
                        handleIMapBatchInvalidationEventV14(const std::vector<serialization::pimpl::Data> &keys,
                                                            const std::vector<std::string> &sourceUuids,
                                                            const std::vector<util::UUID> &partitionUuids,
                                                            const std::vector<int64_t> &sequences) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    MapAddNearCacheEntryListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapAddPartitionLostListenerCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.AddPartitionLostListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void
                        handleMapPartitionLostEventV10(const int32_t &partitionId, const std::string &uuid) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    MapAddPartitionLostListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapClearCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Clear";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapClearCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapContainsKeyCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.ContainsKey";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapContainsKeyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapContainsValueCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.ContainsValue";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapContainsValueCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapDeleteCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Delete";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapDeleteCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapEntriesWithPagingPredicateCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.EntriesWithPagingPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapEntriesWithPagingPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapEntriesWithPredicateCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.EntriesWithPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapEntriesWithPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapEntrySetCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.EntrySet";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapEntrySetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapEvictAllCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.EvictAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapEvictAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapEvictCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Evict";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapEvictCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapExecuteOnAllKeysCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.ExecuteOnAllKeys";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &entryProcessor);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &entryProcessor);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapExecuteOnAllKeysCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapExecuteOnKeyCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.ExecuteOnKey";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &entryProcessor,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &entryProcessor,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapExecuteOnKeyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapExecuteOnKeysCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.ExecuteOnKeys";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &entryProcessor,
                            const std::vector<serialization::pimpl::Data> &keys);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &entryProcessor,
                            const std::vector<serialization::pimpl::Data> &keys);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapExecuteOnKeysCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapExecuteWithPredicateCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.ExecuteWithPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &entryProcessor,
                            const serialization::pimpl::Data &predicate);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &entryProcessor,
                            const serialization::pimpl::Data &predicate);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapExecuteWithPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapFlushCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Flush";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapFlushCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapForceUnlockCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.ForceUnlock";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t referenceId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t referenceId);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapForceUnlockCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapGetAllCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.GetAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &keys);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &keys);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapGetAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapGetCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Get";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapGetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        namespace map {
            class DataEntryView;
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapGetEntryViewCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.GetEntryView";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<map::DataEntryView> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapGetEntryViewCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapIsEmptyCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.IsEmpty";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapIsEmptyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapIsLockedCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.IsLocked";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapIsLockedCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapKeySetCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.KeySet";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapKeySetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapKeySetWithPagingPredicateCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.KeySetWithPagingPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapKeySetWithPagingPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapKeySetWithPredicateCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.KeySetWithPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapKeySetWithPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapLockCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Lock";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t ttl,
                            int64_t referenceId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t ttl,
                            int64_t referenceId);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapLockCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapPutAllCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.PutAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapPutAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapPutCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Put";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapPutCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapPutIfAbsentCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.PutIfAbsent";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapPutIfAbsentCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapPutTransientCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.PutTransient";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapPutTransientCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapPutWithMaxIdleCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.PutWithMaxIdle";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl,
                            int64_t maxIdle);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl,
                            int64_t maxIdle);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapPutWithMaxIdleCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapRemoveAllCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.RemoveAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapRemoveAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapRemoveCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Remove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapRemoveEntryListenerCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.RemoveEntryListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &registrationId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &registrationId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapRemoveEntryListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapRemoveIfSameCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.RemoveIfSame";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapRemoveIfSameCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapRemoveInterceptorCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.RemoveInterceptor";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &id);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &id);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapRemoveInterceptorCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapReplaceCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Replace";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapReplaceCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapReplaceIfSameCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.ReplaceIfSame";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &testValue,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &testValue,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapReplaceIfSameCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapSetCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Set";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapSetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapSetWithMaxIdleCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.SetWithMaxIdle";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl,
                            int64_t maxIdle);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t ttl,
                            int64_t maxIdle);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapSetWithMaxIdleCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapSizeCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapSubmitToKeyCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.SubmitToKey";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &entryProcessor,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &entryProcessor,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapSubmitToKeyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapTryLockCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.TryLock";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t lease,
                            int64_t timeout,
                            int64_t referenceId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t lease,
                            int64_t timeout,
                            int64_t referenceId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapTryLockCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapTryPutCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.TryPut";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t timeout);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId,
                            int64_t timeout);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapTryPutCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapTryRemoveCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.TryRemove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t timeout);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t timeout);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapTryRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapUnlockCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Unlock";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t referenceId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t referenceId);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MapUnlockCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapValuesCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.Values";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapValuesCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapValuesWithPagingPredicateCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.ValuesWithPagingPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapValuesWithPagingPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapValuesWithPredicateCodec {
                public:
                    static const MapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Map.ValuesWithPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MapValuesWithPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapAddEntryListenerCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.AddEntryListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            bool includeValue,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            bool includeValue,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                         const int32_t &eventType, const std::string &uuid,
                                                         const int32_t &numberOfAffectedEntries) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    MultiMapAddEntryListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapAddEntryListenerToKeyCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.AddEntryListenerToKey";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            bool includeValue,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            bool includeValue,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                         const int32_t &eventType, const std::string &uuid,
                                                         const int32_t &numberOfAffectedEntries) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    MultiMapAddEntryListenerToKeyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapClearCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.Clear";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MultiMapClearCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapContainsEntryCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.ContainsEntry";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapContainsEntryCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapContainsKeyCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.ContainsKey";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapContainsKeyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapContainsValueCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.ContainsValue";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapContainsValueCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapEntrySetCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.EntrySet";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapEntrySetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapForceUnlockCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.ForceUnlock";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t referenceId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t referenceId);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MultiMapForceUnlockCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapGetCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.Get";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapGetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapIsLockedCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.IsLocked";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapIsLockedCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapKeySetCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.KeySet";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapKeySetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapLockCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.Lock";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t ttl,
                            int64_t referenceId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t ttl,
                            int64_t referenceId);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MultiMapLockCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapPutCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.Put";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapPutCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapRemoveCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.Remove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapRemoveEntryCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.RemoveEntry";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapRemoveEntryCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapRemoveEntryListenerCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.RemoveEntryListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &registrationId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &registrationId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapRemoveEntryListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapSizeCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapTryLockCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.TryLock";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t lease,
                            int64_t timeout,
                            int64_t referenceId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t lease,
                            int64_t timeout,
                            int64_t referenceId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapTryLockCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapUnlockCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.Unlock";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t referenceId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId,
                            int64_t referenceId);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    MultiMapUnlockCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapValueCountCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.ValueCount";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapValueCountCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MultiMapValuesCodec {
                public:
                    static const MultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "MultiMap.Values";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    MultiMapValuesCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        class Address;

        namespace protocol {
            namespace codec {
                class HAZELCAST_API PNCounterAddCodec {
                public:
                    static const PNCounterMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "PNCounter.Add";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int64_t delta,
                            bool getBeforeUpdate,
                            const std::vector<std::pair<std::string, int64_t> > &replicaTimestamps,
                            const Address &targetReplica);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int64_t delta,
                            bool getBeforeUpdate,
                            const std::vector<std::pair<std::string, int64_t> > &replicaTimestamps,
                            const Address &targetReplica);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int64_t value;

                        std::vector<std::pair<std::string, int64_t> > replicaTimestamps;

                        int32_t replicaCount;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    PNCounterAddCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        class Address;

        namespace protocol {
            namespace codec {
                class HAZELCAST_API PNCounterGetCodec {
                public:
                    static const PNCounterMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "PNCounter.Get";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<std::pair<std::string, int64_t> > &replicaTimestamps,
                            const Address &targetReplica);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<std::pair<std::string, int64_t> > &replicaTimestamps,
                            const Address &targetReplica);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int64_t value;

                        std::vector<std::pair<std::string, int64_t> > replicaTimestamps;

                        int32_t replicaCount;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    PNCounterGetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API PNCounterGetConfiguredReplicaCountCodec {
                public:
                    static const PNCounterMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "PNCounter.GetConfiguredReplicaCount";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    PNCounterGetConfiguredReplicaCountCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueAddAllCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.AddAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &dataList);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &dataList);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueAddAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueAddListenerCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.AddListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            bool includeValue,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            bool includeValue,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void
                        handleItemEventV10(std::unique_ptr<serialization::pimpl::Data> &item, const std::string &uuid,
                                           const int32_t &eventType) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    QueueAddListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueClearCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.Clear";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    QueueClearCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueCompareAndRemoveAllCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.CompareAndRemoveAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &dataList);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &dataList);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueCompareAndRemoveAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueCompareAndRetainAllCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.CompareAndRetainAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &dataList);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &dataList);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueCompareAndRetainAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueContainsAllCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.ContainsAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &dataList);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &dataList);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueContainsAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueContainsCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.Contains";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueContainsCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueDrainToCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.DrainTo";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueDrainToCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueDrainToMaxSizeCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.DrainToMaxSize";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int32_t maxSize);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int32_t maxSize);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueDrainToMaxSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueIsEmptyCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.IsEmpty";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueIsEmptyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueIteratorCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.Iterator";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueIteratorCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueOfferCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.Offer";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value,
                            int64_t timeoutMillis);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value,
                            int64_t timeoutMillis);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueOfferCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueuePeekCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.Peek";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueuePeekCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueuePollCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.Poll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int64_t timeoutMillis);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int64_t timeoutMillis);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueuePollCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueuePutCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.Put";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    QueuePutCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueRemainingCapacityCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.RemainingCapacity";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueRemainingCapacityCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueRemoveCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.Remove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueRemoveListenerCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.RemoveListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &registrationId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &registrationId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueRemoveListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API QueueSizeCodec {
                public:
                    static const QueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Queue.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    QueueSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapAddEntryListenerCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.AddEntryListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                         const int32_t &eventType, const std::string &uuid,
                                                         const int32_t &numberOfAffectedEntries) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    ReplicatedMapAddEntryListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapAddEntryListenerToKeyCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.AddEntryListenerToKey";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                         const int32_t &eventType, const std::string &uuid,
                                                         const int32_t &numberOfAffectedEntries) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    ReplicatedMapAddEntryListenerToKeyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapAddEntryListenerToKeyWithPredicateCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.AddEntryListenerToKeyWithPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &predicate,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &predicate,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                         const int32_t &eventType, const std::string &uuid,
                                                         const int32_t &numberOfAffectedEntries) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    ReplicatedMapAddEntryListenerToKeyWithPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapAddEntryListenerWithPredicateCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.AddEntryListenerWithPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &predicate,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                         const int32_t &eventType, const std::string &uuid,
                                                         const int32_t &numberOfAffectedEntries) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    ReplicatedMapAddEntryListenerWithPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapAddNearCacheEntryListenerCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.AddNearCacheEntryListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            bool includeValue,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            bool includeValue,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                         const int32_t &eventType, const std::string &uuid,
                                                         const int32_t &numberOfAffectedEntries) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    ReplicatedMapAddNearCacheEntryListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapClearCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.Clear";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    ReplicatedMapClearCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapContainsKeyCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.ContainsKey";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapContainsKeyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapContainsValueCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.ContainsValue";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapContainsValueCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapEntrySetCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.EntrySet";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapEntrySetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapGetCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.Get";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapGetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapIsEmptyCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.IsEmpty";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapIsEmptyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapKeySetCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.KeySet";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapKeySetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapPutAllCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.PutAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    ReplicatedMapPutAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapPutCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.Put";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t ttl);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t ttl);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapPutCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapRemoveCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.Remove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapRemoveEntryListenerCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.RemoveEntryListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &registrationId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &registrationId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapRemoveEntryListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapSizeCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ReplicatedMapValuesCodec {
                public:
                    static const ReplicatedMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "ReplicatedMap.Values";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ReplicatedMapValuesCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API RingbufferAddAllCodec {
                public:
                    static const RingbufferMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Ringbuffer.AddAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &valueList,
                            int32_t overflowPolicy);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &valueList,
                            int32_t overflowPolicy);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int64_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    RingbufferAddAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API RingbufferAddCodec {
                public:
                    static const RingbufferMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Ringbuffer.Add";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int32_t overflowPolicy,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int32_t overflowPolicy,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int64_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    RingbufferAddCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API RingbufferCapacityCodec {
                public:
                    static const RingbufferMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Ringbuffer.Capacity";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int64_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    RingbufferCapacityCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API RingbufferHeadSequenceCodec {
                public:
                    static const RingbufferMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Ringbuffer.HeadSequence";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int64_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    RingbufferHeadSequenceCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API RingbufferReadManyCodec {
                public:
                    static const RingbufferMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Ringbuffer.ReadMany";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int64_t startSequence,
                            int32_t minCount,
                            int32_t maxCount,
                            const serialization::pimpl::Data *filter);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int64_t startSequence,
                            int32_t minCount,
                            int32_t maxCount,
                            const serialization::pimpl::Data *filter);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t readCount;

                        std::vector<serialization::pimpl::Data> items;

                        std::unique_ptr<std::vector<int64_t> > itemSeqs;
                        bool itemSeqsExist;
                        int64_t nextSeq;
                        bool nextSeqExist;

                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    RingbufferReadManyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API RingbufferReadOneCodec {
                public:
                    static const RingbufferMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Ringbuffer.ReadOne";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            int64_t sequence);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            int64_t sequence);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    RingbufferReadOneCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API RingbufferRemainingCapacityCodec {
                public:
                    static const RingbufferMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Ringbuffer.RemainingCapacity";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int64_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    RingbufferRemainingCapacityCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API RingbufferSizeCodec {
                public:
                    static const RingbufferMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Ringbuffer.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int64_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    RingbufferSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API RingbufferTailSequenceCodec {
                public:
                    static const RingbufferMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Ringbuffer.TailSequence";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int64_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    RingbufferTailSequenceCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetAddAllCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.AddAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &valueList);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &valueList);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetAddAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetAddCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.Add";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetAddCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetAddListenerCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.AddListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            bool includeValue,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            bool includeValue,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void
                        handleItemEventV10(std::unique_ptr<serialization::pimpl::Data> &item, const std::string &uuid,
                                           const int32_t &eventType) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    SetAddListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetClearCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.Clear";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    SetClearCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetCompareAndRemoveAllCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.CompareAndRemoveAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &values);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &values);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetCompareAndRemoveAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetCompareAndRetainAllCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.CompareAndRetainAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &values);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &values);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetCompareAndRetainAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetContainsAllCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.ContainsAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &items);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::vector<serialization::pimpl::Data> &items);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetContainsAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetContainsCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.Contains";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetContainsCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetGetAllCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.GetAll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetGetAllCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetIsEmptyCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.IsEmpty";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetIsEmptyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetRemoveCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.Remove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetRemoveListenerCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.RemoveListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &registrationId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &registrationId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetRemoveListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API SetSizeCodec {
                public:
                    static const SetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Set.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name);

                    static int32_t calculateDataSize(
                            const std::string &name);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    SetSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TopicAddMessageListenerCodec {
                public:
                    static const TopicMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Topic.AddMessageListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            bool localOnly);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            bool localOnly);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//


                    //************************ EVENTS START*********************************************************************//
                    class HAZELCAST_API AbstractEventHandler : public impl::BaseEventHandler {
                    public:
                        virtual ~AbstractEventHandler();

                        void handle(std::unique_ptr<protocol::ClientMessage> message);


                        virtual void
                        handleTopicEventV10(serialization::pimpl::Data &&item, const int64_t &publishTime,
                                            const std::string &uuid) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//
                private:
                    // Preventing public access to constructors
                    TopicAddMessageListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TopicPublishCodec {
                public:
                    static const TopicMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Topic.Publish";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const serialization::pimpl::Data &message);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const serialization::pimpl::Data &message);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    TopicPublishCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TopicRemoveMessageListenerCodec {
                public:
                    static const TopicMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Topic.RemoveMessageListener";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &registrationId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &registrationId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TopicRemoveMessageListenerCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionCommitCodec {
                public:
                    static const TransactionMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Transaction.Commit";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &transactionId,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &transactionId,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    TransactionCommitCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionCreateCodec {
                public:
                    static const TransactionMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Transaction.Create";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            int64_t timeout,
                            int32_t durability,
                            int32_t transactionType,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            int64_t timeout,
                            int32_t durability,
                            int32_t transactionType,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::string response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionCreateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionRollbackCodec {
                public:
                    static const TransactionMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "Transaction.Rollback";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &transactionId,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &transactionId,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    TransactionRollbackCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalListAddCodec {
                public:
                    static const TransactionalListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalList.Add";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &item);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &item);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalListAddCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalListRemoveCodec {
                public:
                    static const TransactionalListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalList.Remove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &item);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &item);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalListRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalListSizeCodec {
                public:
                    static const TransactionalListMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalList.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalListSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapContainsKeyCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.ContainsKey";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapContainsKeyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapDeleteCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.Delete";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    TransactionalMapDeleteCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapGetCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.Get";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapGetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapIsEmptyCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.IsEmpty";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapIsEmptyCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapKeySetCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.KeySet";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapKeySetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapKeySetWithPredicateCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.KeySetWithPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &predicate);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &predicate);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapKeySetWithPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapPutCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.Put";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t ttl);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value,
                            int64_t ttl);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapPutCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapPutIfAbsentCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.PutIfAbsent";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapPutIfAbsentCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapRemoveCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.Remove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapRemoveIfSameCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.RemoveIfSame";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapRemoveIfSameCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapReplaceCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.Replace";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapReplaceCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapReplaceIfSameCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.ReplaceIfSame";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &oldValue,
                            const serialization::pimpl::Data &newValue);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &oldValue,
                            const serialization::pimpl::Data &newValue);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapReplaceIfSameCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapSetCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.Set";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//


                private:
                    // Preventing public access to constructors
                    TransactionalMapSetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapSizeCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapValuesCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.Values";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapValuesCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMapValuesWithPredicateCodec {
                public:
                    static const TransactionalMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMap.ValuesWithPredicate";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &predicate);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &predicate);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMapValuesWithPredicateCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMultiMapGetCodec {
                public:
                    static const TransactionalMultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMultiMap.Get";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMultiMapGetCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMultiMapPutCodec {
                public:
                    static const TransactionalMultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMultiMap.Put";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMultiMapPutCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMultiMapRemoveCodec {
                public:
                    static const TransactionalMultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMultiMap.Remove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::vector<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMultiMapRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMultiMapRemoveEntryCodec {
                public:
                    static const TransactionalMultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMultiMap.RemoveEntry";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key,
                            const serialization::pimpl::Data &value);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMultiMapRemoveEntryCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMultiMapSizeCodec {
                public:
                    static const TransactionalMultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMultiMap.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMultiMapSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalMultiMapValueCountCodec {
                public:
                    static const TransactionalMultiMapMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalMultiMap.ValueCount";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &key);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalMultiMapValueCountCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalQueueOfferCodec {
                public:
                    static const TransactionalQueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalQueue.Offer";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &item,
                            int64_t timeout);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &item,
                            int64_t timeout);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalQueueOfferCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalQueuePollCodec {
                public:
                    static const TransactionalQueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalQueue.Poll";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            int64_t timeout);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            int64_t timeout);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        std::unique_ptr<serialization::pimpl::Data> response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                        // define copy constructor (needed for unique_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalQueuePollCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalQueueSizeCodec {
                public:
                    static const TransactionalQueueMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalQueue.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalQueueSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalSetAddCodec {
                public:
                    static const TransactionalSetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalSet.Add";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &item);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &item);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalSetAddCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalSetRemoveCodec {
                public:
                    static const TransactionalSetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalSet.Remove";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &item);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId,
                            const serialization::pimpl::Data &item);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        bool response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalSetRemoveCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API TransactionalSetSizeCodec {
                public:
                    static const TransactionalSetMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static constexpr const char* OPERATION_NAME = "TransactionalSet.Size";
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::unique_ptr<ClientMessage> encodeRequest(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);

                    static int32_t calculateDataSize(
                            const std::string &name,
                            const std::string &txnId,
                            int64_t threadId);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        int32_t response;


                        static ResponseParameters decode(ClientMessage clientMessage);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    TransactionalSetSizeCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



