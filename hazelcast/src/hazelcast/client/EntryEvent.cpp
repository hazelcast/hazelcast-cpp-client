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
            value = (EntryEventType::Type)i;
        }
    }
}
