#include "hazelcast/client/ItemEvent.h"

namespace hazelcast {
    namespace client {
        ItemEventType::ItemEventType() {
        }

        ItemEventType::ItemEventType(Type value) :value(value) {
        }

        ItemEventType::operator int() const {
            return value;
        }

        /**
         * copy function.
         */
        void ItemEventType::operator = (int i) {
            switch (i) {
                case 1:
                    value = ADDED;
                    break;
                case 2:
                    value = REMOVED;
                    break;
            }
        }
    }
}
