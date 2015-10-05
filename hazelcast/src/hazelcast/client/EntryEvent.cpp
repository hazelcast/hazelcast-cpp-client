#include "hazelcast/client/EntryEvent.h"

namespace hazelcast {
    namespace client {
        EntryEventType::EntryEventType() : value(UNDEFINED){

        }
        EntryEventType::EntryEventType(Type value)
        :value(value) {

        }

        EntryEventType::operator int() const {
            return value;
        }

        void EntryEventType::operator = (int i) {
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
                case 5:
                    value = EVICT_ALL;
                    break;
                case 6:
                    value = CLEAR_ALL;
                    break;
                case 7:
                    value = MERGED;
                    break;
                case 8:
                    value = EXPIRED;
                    break;
                default:
                    break;
            }
        }
    }
}
