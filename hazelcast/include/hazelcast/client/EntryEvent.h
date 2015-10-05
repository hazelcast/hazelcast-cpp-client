//
// Created by ihsan demir on 9/9/15.
// Copyright (c) 2015 hazelcast. All rights reserved.

#ifndef HAZELCAST_ENTRY_EVENT
#define HAZELCAST_ENTRY_EVENT

#include "hazelcast/client/Member.h"
#include <boost/shared_ptr.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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
                UNDEFINED = 0 ,
                ADDED =  1,
                REMOVED = 1 << 1,
                UPDATED = 1 << 2,
                EVICTED = 1 << 3 ,
                EVICT_ALL = 1 << 4 ,
                CLEAR_ALL = 1 << 5 ,
                MERGED = 1 << 6 ,
                EXPIRED = 1 << 7
            };
            /**
             * Type value
             */
            Type value;

            /**
             * Constructor
             */
            EntryEventType();

            /**
             * Constructor
             */
            EntryEventType(Type value);

            /**
             * cast to int.
             */
            operator int() const;

            /**
             * copy method.
             */
            void operator = (int i);

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
        class EntryEvent {
        public:
            /**
             * Constructor
             */
            EntryEvent(const std::string &name, const Member &member, EntryEventType eventType,
                        boost::shared_ptr<K> key, boost::shared_ptr<V> value)
            : name(name)
            , member(member)
            , eventType(eventType)
            , key(key)
            , value(value) {

            };

            /**
             * Constructor
             */
            EntryEvent(const std::string &name, const Member &member, EntryEventType eventType,
                           boost::shared_ptr<K> key, boost::shared_ptr<V> value,
                            boost::shared_ptr<V> oldValue, boost::shared_ptr<V> mergingValue)
            : name(name)
            , member(member)
            , eventType(eventType)
            , key(key)
            , value(value)
            , oldValue(oldValue)
            , mergingValue(mergingValue) {

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
            * Returns the incoming merging value of the entry event.
            *
            * @return
            */
            const V &getMErgingValue() const {
                return *mergingValue;
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
            std::string name;
            Member member;
            EntryEventType eventType;
            boost::shared_ptr<K> key;
            boost::shared_ptr<V> value;
            boost::shared_ptr<V> oldValue;
            boost::shared_ptr<V> mergingValue;

        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //__EntryEvent_H_

