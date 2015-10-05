//
// Created by sancar koyunlu on 04/09/14.
//


#ifndef HAZELCAST_EntryAdapter
#define HAZELCAST_EntryAdapter


#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/EntryListener.h"

namespace hazelcast {
    namespace client {
        class MapEvent;

        /**
        * Adapter for EntryListener.
        *
        * @param <K> key of the map entry
        * @param <V> value of the map entry.
        * @see EntryListener
        */
        template<typename K, typename V>
        class EntryAdapter : public EntryListener<K, V> {
        public:
            virtual ~EntryAdapter() {

            }

            virtual void entryAdded(const EntryEvent<K, V>& event) {

            }

            virtual void entryRemoved(const EntryEvent<K, V>& event) {

            }

            virtual void entryUpdated(const EntryEvent<K, V>& event) {

            }

            virtual void entryEvicted(const EntryEvent<K, V>& event) {

            }


            virtual void entryExpired(const EntryEvent<K, V>& event){

            }

            virtual void entryMerged(const EntryEvent<K, V>& event){

            }

            virtual void mapEvicted(const MapEvent& event) {

            }

            virtual void mapCleared(const MapEvent& event) {

            }
        };
    }
}


#endif //HAZELCAST_EntryAdapter
