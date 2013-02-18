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
            ClientConfig& operator=(const ClientConfig&);
            ~ClientConfig();

            GroupConfig& getGroupConfig();
            void setAddress(std::string);
            Address getAddress() const;
            void setPortableFactory(serialization::PortableFactory*);
            serialization::PortableFactory const * getPortableFactory() const;
        private:
            GroupConfig groupConfig;
            Address address;
            serialization::PortableFactory* portableFactory;
        };

    }
}
#endif /* HAZELCAST_CLIENT_CONFIG */