//
// Created by sancar koyunlu on 04/09/14.
//


#ifndef HAZELCAST_ItemListener
#define HAZELCAST_ItemListener

#include "hazelcast/client/ItemEvent.h"

namespace hazelcast {
    namespace client {

        /**
        * Item listener for  IQueue, ISet and IList
        *
        * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
        * otherwise it will slow down the system.
        *
        * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
        *
        * @param <E> item
        */
        template<typename E>
        class ItemListener {
        public:
            virtual ~ItemListener() {

            }

            /**
            * Invoked when an item is added.
            *
            * @param item added item
            */
            virtual void itemAdded(const ItemEvent<E>& item) = 0;

            /**
            * Invoked when an item is removed.
            *
            * @param item removed item.
            */
            virtual void itemRemoved(const ItemEvent<E>& item) = 0;
        };
    }
}


#endif //HAZELCAST_ItemListener
