/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 9/9/15.
// Copyright (c) 2015 hazelcast. All rights reserved.

#ifndef HAZELCAST_ENTRY_EVENT
#define HAZELCAST_ENTRY_EVENT

#include <memory>
#include "hazelcast/client/Member.h"

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
                EXPIRED = 1 << 7,
                ALL = 0xFF
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
                        std::auto_ptr<K> key, std::auto_ptr<V> value)
            : name(name)
            , member(member)
            , eventType(eventType)
            , key(key)
            , value(value) {
            }

            /**
             * Constructor
             */
            EntryEvent(const std::string &name, const Member &member, EntryEventType eventType,
                           std::auto_ptr<K> key, std::auto_ptr<V> value,
                            std::auto_ptr<V> oldValue, std::auto_ptr<V> mergingValue)
            : name(name)
            , member(member)
            , eventType(eventType)
            , key(key)
            , value(value)
            , oldValue(oldValue)
            , mergingValue(mergingValue) {
            }

            /**
             *
             * Returns the key of the entry event
             *
             * @return the key
             */
            const K *getKeyObject() const {
                return key.get();
            }

            /**
             *
             * Releases the key of the entry event. It resets the key in the event, calling getKeyObject,
             * releaseKeyObject after calling releaseKey shall return NULL pointer.
             *
             * @return the key
             */
            std::auto_ptr<K> releaseKey() {
                return key;
            }

            /**
             * @deprecated This API is deprecated in favor of @sa{getKeyObject} and @sa{releaseKey}. It may be
             * removed in the next releases.
             *
             * Returns the key of the entry event
             *
             * @return the key
             */
            const K &getKey() const {
                return *key;
            }

            /**
             * Returns the old value of the entry event
             *
             * @return The older value for the entry
             */
            const V *getOldValueObject() const {
                return oldValue.get();
            }

            /**
             * Releases the old value of the entry event. It resets the oldValue in the event, calling getOldValueObject,
             * releaseOldValue after calling releaseOldValue shall return NULL pointer.
             *
             * Returns the old value of the entry event
             *
             * @return The older value for the entry
             */
            std::auto_ptr<V> releaseOldValue() {
                return oldValue;
            }

            /**
             * @deprecated This API is deprecated in favor of @sa{getOldValueObject} and @sa{releaseOldValue}. It may be
             * removed in the next releases.
             *
             * Returns the old value of the entry event
             *
             * @return The older value for the entry
             */
            const V &getOldValue() const {
                return *oldValue;
            }

            /**
             * Returns the value of the entry event
             *
             * @return The value for the entry
             */
            const V *getValueObject() const {
                return value.get();
            }

            /**
             * Releases the value of the entry event. It resets the value in the event, calling getValueObject,
             * releaseValue after calling releaseValue shall return NULL pointer.
             *
             * Returns the old value of the entry event
             *
             * @return The older value for the entry
             */
            std::auto_ptr<V> releaseValue() {
                return value;
            }

            /**
             * @deprecated This API is deprecated in favor of @sa{getValueObject} and @sa{releaseValue}. It may be
             * removed in the next releases.
             *
             * Returns the value of the entry event
             *
             * @return The value of for the entry
             */
            const V &getValue() const {
                return *value;
            }

            /**
            * Returns the incoming merging value of the entry event.
            *
            * @return The merging value
            */
            const V *getMergingValueObject() const {
                return mergingValue.get();
            }

            /**
            * Releases the mergingValue of the entry event. It resets the value in the event, calling getmergingValueObject,
            * releasemergingValue after calling releasemergingValue shall return NULL pointer.
            *
            * Returns the incoming merging value of the entry event.
            *
            * @return merging value
            */
            std::auto_ptr<V> releaseMergingValue() const {
                return mergingValue;
            }

            /**
            * @deprecated This API is deprecated in favor of @sa{getMergingValueObject} and @sa{releaseMergingValue}. It may be
            * removed in the next releases.
            *
            * Returns the incoming merging value of the entry event.
            *
            * @return merging value
            */
            const V &getMergingValue() const {
                return *mergingValue;
            }

            /**
             * Returns the member fired this event.
             *
             * @return the member fired this event.
             */
            const Member &getMember() const {
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
            std::string &getName() const {
                return name;
            };

            std::ostream &operator<< (std::ostream &out) const {
                out << "EntryEvent{entryEventType=" << eventType.value << eventType <<
                ", member=" << member << ", name='" << name << "', key=" << *key;
                if (value.get()) {
                    out << ", value=" << *value;
                }
                if (oldValue.get()) {
                    out << ", oldValue=" << *oldValue;
                }
                if (mergingValue.get()) {
                    out << ", mergingValue=" << *mergingValue;
                }
                return out;
            }
        private:
            std::string name;
            Member member;
            EntryEventType eventType;
            std::auto_ptr<K> key;
            std::auto_ptr<V> value;
            std::auto_ptr<V> oldValue;
            std::auto_ptr<V> mergingValue;

        };
    }
}


template <typename K, typename V>
std::ostream &operator<<(std::ostream &out, const hazelcast::client::EntryEvent<K, V> &event) {
    event.operator<<(out);
    return out;
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //__EntryEvent_H_

