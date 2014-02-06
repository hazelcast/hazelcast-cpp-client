//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_ENTRY_EVENT
#define HAZELCAST_ENTRY_EVENT

#include "hazelcast/client/Member.h"
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace client {
        /**
         * Type of entry event.
         */
        class HAZELCAST_API EntryEventType {
        public:
            /**
             * Type enum.
             */
            enum Type {
                ADDED = 1, REMOVED = 2, UPDATED = 3, EVICTED = 4
            };
            /**
             * Type value
             */
            Type value;

            /**
             * Constrcutor
             */
            EntryEventType() {

            }

            /**
             * Constrcutor
             */
            EntryEventType(Type value)
            :value(value) {

            }

            /**
             * cast to int.
             */
            operator int() const {
                return value;
            }

            /**
             * copy method.
             */
            void operator = (int i) {
                switch (i) {
                    case 1:
                        value = ADDED;
                        break;
                    case 2:
                        value = REMOVED;
                        break;
                    case 3:
                        value = UPDATED;
                        break;
                    case 4:
                        value = EVICTED;
                        break;
                }
            }

        };

        /**
         * Map Entry event.
         *
         * @param <K> key of the map entry
         * @param <V> value of the map entry
         * @see EntryListener
         * @see IMap#addEntryListener(EntryListener, boolean)
         */
        template <typename K, typename V>
        class HAZELCAST_API EntryEvent {
        public:
            /**
             * Constrcutor
             */
            EntryEvent(const std::string &name, const Member &member, EntryEventType eventType, boost::shared_ptr<K> key, boost::shared_ptr<V> value)
            : name(name)
            , member(member)
            , eventType(eventType)
            , key(key)
            , value(value) {

            };

            /**
             * Constrcutor
             */
            EntryEvent(const std::string &name, const Member &member, EntryEventType eventType, boost::shared_ptr<K> key, boost::shared_ptr<V> value, boost::shared_ptr<V> oldValue)
            : name(name)
            , member(member)
            , eventType(eventType)
            , key(key)
            , value(value)
            , oldValue(oldValue) {

            };

            /**
             * Returns the key of the entry event
             *
             * @return the key
             */
            const K &getKey() const {
                return *key;
            };

            /**
             * Returns the old value of the entry event
             *
             * @return
             */
            const V &getOldValue() const {
                return *oldValue;
            };

            /**
             * Returns the value of the entry event
             *
             * @return
             */
            const V &getValue() const {
                return *value;
            };

            /**
             * Returns the member fired this event.
             *
             * @return the member fired this event.
             */
            Member getMember() const {
                return member;
            };

            /**
             * Return the event type
             *
             * @return event type
             */
            EntryEventType getEventType() const {
                return eventType;
            };

            /**
             * Returns the name of the map for this event.
             *
             * @return name of the map.
             */
            std::string getName() const {
                return name;
            };

        private:
            boost::shared_ptr<K> key;
            boost::shared_ptr<V> value;
            boost::shared_ptr<V> oldValue;
            EntryEventType eventType;
            Member member;
            std::string name;

        };
    }
}

#endif //__EntryEvent_H_
