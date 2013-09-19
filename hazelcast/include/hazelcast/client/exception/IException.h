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
        namespace exception {
            class IException : public std::exception {
            public:
                IException(const std::string& source, const std::string& message);

                virtual ~IException() throw();

                virtual char const *what() const throw();

            private:
                std::string message;
                std::string source;
            };
        }
    }
}

#endif //HAZELCAST_EXCEPTION

