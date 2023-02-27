/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/address.h"
#include "hazelcast/client/client_config.h"
#include "hazelcast/client/cluster.h"
#include "hazelcast/client/distributed_object.h"
#include "hazelcast/client/entry_event.h"
#include "hazelcast/client/entry_view.h"
#include "hazelcast/client/hazelcast_client.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/ilist.h"
#include "hazelcast/client/imap.h"
#include "hazelcast/client/initial_membership_event.h"
#include "hazelcast/client/iqueue.h"
#include "hazelcast/client/iset.h"
#include "hazelcast/client/item_event.h"
#include "hazelcast/client/itopic.h"
#include "hazelcast/client/lifecycle_event.h"
#include "hazelcast/client/lifecycle_listener.h"
#include "hazelcast/client/load_balancer.h"
#include "hazelcast/client/member.h"
#include "hazelcast/client/membership_event.h"
#include "hazelcast/client/membership_listener.h"
#include "hazelcast/client/multi_map.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/socket.h"
#include "hazelcast/client/socket_interceptor.h"
#include "hazelcast/client/transactional_list.h"
#include "hazelcast/client/transactional_map.h"
#include "hazelcast/client/transactional_multi_map.h"
#include "hazelcast/client/transactional_queue.h"
#include "hazelcast/client/transactional_set.h"
#include "hazelcast/client/transaction_context.h"
#include "hazelcast/client/transaction_options.h"
#include "hazelcast/client/monitor/local_map_stats.h"
#include "hazelcast/client/monitor/near_cache_stats.h"
#include "hazelcast/client/partition_aware.h"
#include <hazelcast/client/member_selectors.h>
