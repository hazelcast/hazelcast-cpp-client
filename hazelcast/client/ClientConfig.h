#ifndef HAZELCAST_CLIENT_CONFIG
#define HAZELCAST_CLIENT_CONFIG

#include "Address.h"
#include "GroupConfig.h"
#include "serialization/PortableFactory.h"
#include <list>
#include <string>

namespace hazelcast {
    namespace client {

        class ClientConfig {
        public:

            ClientConfig();

            ClientConfig(const ClientConfig&);

            ClientConfig& operator = (const ClientConfig&);

            ~ClientConfig();

            GroupConfig& getGroupConfig();

            void setAddress(std::string, std::string);

            Address getAddress() const;

            void addPortableFactory(int, serialization::PortableFactory *);

            std::map< int, serialization::PortableFactory const *  > const *const getPortableFactories();

        private:
            GroupConfig groupConfig;
            Address address;
            std::map< int, serialization::PortableFactory const * > portableFactories;
        };

    }
}
#endif /* HAZELCAST_CLIENT_CONFIG */