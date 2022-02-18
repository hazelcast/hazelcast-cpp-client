/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/member.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
/**
 * Type of item event.
 */
enum struct HAZELCAST_API item_event_type
{
    ADDED = 1,
    REMOVED = 2
};

class HAZELCAST_API item_event_base
{
public:
    item_event_base(const std::string& name,
                    const member& member,
                    const item_event_type& event_type);

    virtual ~item_event_base();

    /**
     * Returns the member fired this event.
     *
     * @return the member fired this event.
     */
    const member& get_member() const;

    /**
     * Return the event type
     *
     * @return event type ItemEventType
     */
    item_event_type get_event_type() const;

    /**
     * Returns the name of the collection for this event.
     *
     * @return name of the collection.
     */
    const std::string& get_name() const;

private:
    std::string name_;
    member member_;
    item_event_type event_type_;
};

/**
 *
 * @see Queue#add_item_listener
 * @see List#add_item_listener
 * @see Set#add_item_listener
 */
class HAZELCAST_API item_event : public item_event_base
{
public:
    item_event(const std::string& name,
               item_event_type event_type,
               typed_data&& item,
               const member& member)
      : item_event_base(name, member, event_type)
      , item_(item)
    {}

    /**
     * @returns the item.
     */
    const typed_data& get_item() const { return item_; }

private:
    typed_data item_;
};
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
