//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_SERVER_ERROR
#define HAZELCAST_SERVER_ERROR

#include "hazelcast/client/impl/PortableResponse.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace exception {

            /**
             * Thrown when an exception occured in server nodes.
             */
            class HAZELCAST_API ServerException : public impl::PortableResponse, public std::exception {
            public:
                /**
                 * Constructor
                 */
                ServerException();

                /**
                 * Destructor
                 */
                virtual ~ServerException() throw();

                /**
                 * return exception explanation string.
                 */
                virtual char const *what() const throw();

                /**
                 * @see Portable#getClassId
                 */
                int getClassId() const;

                /**
                 * @see Portable#getFactoryId
                 */
                int getFactoryId() const;

                /**
                 * @see Portable#readPortable
                 */
                void readPortable(serialization::PortableReader &reader);

                /**
                 * return true if exception isInstanceNotActiveException.
                 */
                bool isInstanceNotActiveException() const;

            private:
                std::string name;

                std::string details;

                std::string message;

                int type;
            };
        }
    }
}


#endif //HAZELCAST_SERVER_ERROR
