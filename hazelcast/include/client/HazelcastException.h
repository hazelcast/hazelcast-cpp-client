//
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_EXCEPTION
#define HAZELCAST_EXCEPTION

#include <string>
#include <stdexcept>


namespace hazelcast {
    namespace client {

        class HazelcastException : public std::domain_error {

        public:

            HazelcastException(::std::string message);

            virtual ~HazelcastException() throw();

            virtual char const *what() const throw();
        };

    }
}

#endif //HAZELCAST_EXCEPTION

