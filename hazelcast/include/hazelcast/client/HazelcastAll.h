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
// Created by sancar koyunlu on 29/01/14.
//

#ifndef HAZELCAST_HazelcastAll
#define HAZELCAST_HazelcastAll

#include "hazelcast/client/Address.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/Credentials.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/EntryView.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IAtomicLong.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/IList.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/ISet.h"
#include "hazelcast/client/ItemEvent.h"
#include "hazelcast/client/ITopic.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/TransactionalList.h"
#include "hazelcast/client/TransactionalMap.h"
#include "hazelcast/client/TransactionalMultiMap.h"
#include "hazelcast/client/TransactionalQueue.h"
#include "hazelcast/client/TransactionalSet.h"
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/client/monitor/LocalMapStats.h"
#include "hazelcast/client/monitor/NearCacheStats.h"
#include "hazelcast/client/PartitionAware.h"
#include "hazelcast/client/mixedtype/IList.h"
#include "hazelcast/client/mixedtype/IQueue.h"
#include "hazelcast/client/mixedtype/Ringbuffer.h"
#include "hazelcast/client/crdt/pncounter/PNCounter.h"
#include <hazelcast/client/cluster/memberselector/MemberSelectors.h>

#endif //HAZELCAST_HazelcastAll

