//
// Created by sancar koyunlu on 04/09/14.
//


#ifndef HAZELCAST_EntryListener
#define HAZELCAST_EntryListener

#include "hazelcast/client/EntryEvent.h"

namespace hazelcast {
    namespace client {
        class IMapEvent;

        /**
        * Map Entry listener to get notified when a map entry
        * is added, removed, updated or evicted.  Events will fire as a result
        * of operations carried out via the IMap.
        *
        * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
        * otherwise it will slow down the system.
        *
        * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
        *
        * @param <K> key of the map entry
        * @param <V> value of the map entry.
        * @see IMap#addEntryListener(EntryListener, boolean)
        */
        template<typename K, typename V>
        class HAZELCAST_API EntryListener {
        public:
            virtual ~EntryListener() {

            }

            /**
            * Invoked when an entry is added.
            *
            * @param event entry event
            */
            virtual void entryAdded(const EntryEvent<K, V>& event) = 0;

            /**
            * Invoked when an entry is removed.
            *
            * @param event entry event
            */
            virtual void entryRemoved(const EntryEvent<K, V>& event) = 0;

            /**
            * Invoked when an entry is removed.
            *
            * @param event entry event
            */
            virtual void entryUpdated(const EntryEvent<K, V>& event) = 0;

            /**
            * Invoked when an entry is evicted.
            *
            * @param event entry event
            */
            virtual void entryEvicted(const EntryEvent<K, V>& event) = 0;

            /**
            * Invoked when all entries evicted by {@link IMap#evictAll()}.
            *
            * @param event map event
            */
            virtual void mapEvicted(const IMapEvent& event) = 0;

            /**
            * Invoked when all entries are removed by {@link IMap#clear()}.}
            */
            virtual void mapCleared(const IMapEvent& event) = 0;

        };
    }
}


#endif //HAZELCAST_EntryListener
