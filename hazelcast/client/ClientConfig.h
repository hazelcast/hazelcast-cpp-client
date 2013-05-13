//#ifndef HAZELCAST_CLIENT_CONFIG
//#define HAZELCAST_CLIENT_CONFIG
//
//#include "Address.h"
//#include "GroupConfig.h"
//
//namespace hazelcast {
//    namespace client {
//
//        class ClientConfig {
//        public:
//
//            ClientConfig(const Address&);
//
//            ClientConfig(const ClientConfig&);
//
//            ClientConfig& operator = (const ClientConfig&);
//
//            ~ClientConfig();
//
//            GroupConfig& getGroupConfig();
//
//            Address getAddress() const;
//
//        private:
//            GroupConfig groupConfig;
//            Address address;
//        };
//
//    }
//}
//#endif /* HAZELCAST_CLIENT_CONFIG */