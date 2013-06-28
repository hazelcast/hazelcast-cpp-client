//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ENTRY_LISTENER
#define HAZELCAST_ENTRY_LISTENER

#include "../spi/EventListener.h"
#include "EntryEvent.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class EntryListener : public spi::EventListener {
            public:

                /**
                 * Invoked when an entry is added.
                 *
                 * @param event entry event
                 */
//                void entryAdded(EntryEvent<K, V> event);

                /**
                 * Invoked when an entry is removed.
                 *
                 * @param event entry event
                 */
                //void entryRemoved(EntryEvent<K, V> event);

                /**
                 * Invoked when an entry is updated.
                 *
                 * @param event entry event
                 */
                //void entryUpdated(EntryEvent<K, V> event);

                /**
                 * Invoked when an entry is evicted.
                 *
                 * @param event entry event
                 */
                //void entryEvicted(EntryEvent<K, V> event);
            };
        }
    }
}

#endif //__ItemListener_H_
