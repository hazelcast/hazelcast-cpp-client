//
//  Created by ihsan demir on 9/9/15.
//  Copyright (c) 2015 ihsan demir. All rights reserved.
//

#include "hazelcast/client/serialization/FieldType.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            FieldType::FieldType() : id(0) {
            }

            FieldType::FieldType(int type) : id((byte)type) {
            }

            FieldType::FieldType(FieldType const& rhs) : id(rhs.id) {
            }

            const byte FieldType::getId() const {
                return id;
            }

            FieldType& FieldType::operator=(FieldType const& rhs) {
                this->id = rhs.id;
                return (*this);
            }

            bool FieldType::operator==(FieldType const& rhs) const {
                if (id != rhs.id) return false;
                return true;
            }

            bool FieldType::operator!=(FieldType const& rhs) const {
                if (id == rhs.id) return false;
                return true;
            }
        }
    }
}

