/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/export.h"
#include "hazelcast/client/member.h"
#include "hazelcast/client/entry_event.h"
#include <string>
#include <ostream>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /**
        * Map events common contract.
        */
        class HAZELCAST_API map_event {
        public:
            /**
            * Constructor
            */
            map_event(member &&member, entry_event::type event_type, const std::string& name, int number_of_entries_affected);

            /**
            * Returns the member fired this event.
            *
            * @return the member fired this event.
            */
            const member &get_member() const;

            /**
            * Return the event type
            *
            * @return event type
            */
            entry_event::type get_event_type() const;

            /**
            * Returns the name of the map for this event.
            *
            * @return name of the map.
            */
            const std::string& get_name() const;

            /**
            * Returns the number of entries affected by this event.
            *
            * @return number of entries affected.
            */
            int get_number_of_entries_affected() const;

            friend std::ostream HAZELCAST_API &operator<<(std::ostream &os, const map_event &event);

        private:
            member member_;
            entry_event::type event_type_;
            std::string name_;
            int number_of_entries_affected_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


