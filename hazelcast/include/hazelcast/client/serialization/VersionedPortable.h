//
// Created by sancar koyunlu on 23/07/14.
//


#ifndef HAZELCAST_VersionedPortable
#define HAZELCAST_VersionedPortable

#include "hazelcast/client/serialization/Portable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            /**
            * VersionedPortable is an extension to {@link Portable}
            * to support per class version instead of a global serialization version.
            *
            * @see Portable
            */
            class HAZELCAST_API VersionedPortable : public Portable {
            public:
                /**
                * Returns version for this Portable class
                * @return class version
                */
                virtual int getClassVersion() const = 0;

                /**
                * Destructor
                */
                virtual ~VersionedPortable();

            };
        }
    }
}

#endif //HAZELCAST_VersionedPortable
