#ifndef HAZELCAST_GROUP_CONFIG
#define HAZELCAST_GROUP_CONFIG

#include <string>

namespace hazelcast {
    namespace client {

        class GroupConfig {
        public:

            /*
             * Default value for name = dev
             *               for password = dev-pass
             */
            GroupConfig();

            GroupConfig(std::string name, std::string password);

            std::string getPassword() const;

            std::string getName() const;

            GroupConfig& setPassword(std::string password);

            GroupConfig& setName(std::string name);

        private:
            std::string name;
            std::string password;
        };

    }
}
#endif /* HAZELCAST_GROUP_CONFIG */