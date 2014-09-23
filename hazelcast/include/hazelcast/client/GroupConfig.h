#ifndef HAZELCAST_GROUP_CONFIG
#define HAZELCAST_GROUP_CONFIG

#include "hazelcast/util/HazelcastDll.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {

        /**
         * Contains the configuration for Hazelcast groups.
         *
         * With groups it is possible to create multiple clusters where each cluster has its own group and doesn't
         * interfere with other clusters.
         */
        class HAZELCAST_API GroupConfig {
        public:

            /*
             * Default value for name = dev
             *               for password = dev-pass
             */
            GroupConfig();

            /**
             * Creates a GroupConfig with the given group-name and default group-password
             *
             * @param name  the name of the group
             * @param password  the password of the group
            */
            GroupConfig(const std::string &name, const std::string &password);


            /**
             * Gets the password to connect to to the group.
             *
             * @return the password
             */
            std::string getPassword() const;

            /**
             * Gets the name of the group.
             *
             * @return the name
             */
            std::string getName() const;

            /**
             * Sets the password.
             *
             * @param password the password to set
             * @return the updated GroupConfig.
             */
            GroupConfig &setPassword(const std::string &password);

            /**
             * Sets the group name.
             *
             * @param name the name to set
             * @return the updated GroupConfig.
             */
            GroupConfig &setName(const std::string &name);

        private:
            std::string name;
            std::string password;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_GROUP_CONFIG */
