//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MEMBER
#define HAZELCAST_MEMBER

#include <map>
#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"

#include <map>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class ClusterListenerThread;
        }

        /**
         * Cluster member class. The default implementation
         *
         * @see Cluster
         * @see MembershipListener
         */
        class HAZELCAST_API Member : public impl::IdentifiedDataSerializableResponse {
        public:
            friend class connection::ClusterListenerThread;

            /**
             * comparison operation
             */
            bool operator ==(const Member &) const;

            /**
             * @see IdentifiedDataSerializable
             */
            int getFactoryId() const;

            /**
             * @see IdentifiedDataSerializable
             */
            int getClassId() const;

            /**
             *
             * Lite member is does not hold data.
             * @return true if member is lite.
             */
            bool isLiteMember() const;

            /**
             * @see IdentifiedDataSerializable
             */
            void readData(serialization::ObjectDataInput &reader);

            /**
             * Returns the socket address of this member.
             *
             * @return socket address of this member
             */
            const Address &getAddress() const;

            /**
             * Returns UUID of this member.
             *
             * @return UUID of this member.
             */
            const std::string &getUuid() const;

            /**
             * Returns the value of the specified key for this member or
             * default constructed value if value is undefined.
             *
             * @tparam AttributeType type template for attribute type
             * @param key The key to lookup.
             * @return The value for this members key.
             */
            template <typename AttributeType>
            AttributeType getAttribute(const std::string &key) {
                AttributeType *tag = NULL;
                return getAttributeResolved(key, tag);
            }

            /**
             * check if an attribute is defined for given key.
             *
             * @tparam AttributeType type template for attribute type
             * @return true if attribute is defined.
             */
            template <typename AttributeType>
            bool lookupAttribute(const std::string &key) const {
                AttributeType *tag = NULL;
                return lookupAttributeResolved(key, tag);
            };

        private:
            template <typename AttributeType>
            void setAttribute(const std::string &key, AttributeType value) {
                setAttributeResolved(key, value);
            }

            template <typename AttributeType>
            bool removeAttribute(const std::string &key) {
                AttributeType *tag = NULL;
                return removeAttributeResolved(key, tag);
            };

            std::string getAttributeResolved(const std::string &key, std::string *tag);

            bool getAttributeResolved(const std::string &key, bool *tag);

            byte getAttributeResolved(const std::string &key, byte *tag);

            short getAttributeResolved(const std::string &key, short *tag);

            int getAttributeResolved(const std::string &key, int *tag);

            long getAttributeResolved(const std::string &key, long *tag);

            float getAttributeResolved(const std::string &key, float *tag);

            double getAttributeResolved(const std::string &key, double *tag);

            void setAttributeResolved(const std::string &key, std::string value);

            void setAttributeResolved(const std::string &key, bool value);

            void setAttributeResolved(const std::string &key, byte value);

            void setAttributeResolved(const std::string &key, short value);

            void setAttributeResolved(const std::string &key, int value);

            void setAttributeResolved(const std::string &key, long value);

            void setAttributeResolved(const std::string &key, float value);

            void setAttributeResolved(const std::string &key, double value);

            bool removeAttributeResolved(const std::string &key, std::string *tag);

            bool removeAttributeResolved(const std::string &key, bool *tag);

            bool removeAttributeResolved(const std::string &key, byte *tag);

            bool removeAttributeResolved(const std::string &key, short *tag);

            bool removeAttributeResolved(const std::string &key, int *tag);

            bool removeAttributeResolved(const std::string &key, long *tag);

            bool removeAttributeResolved(const std::string &key, float *tag);

            bool removeAttributeResolved(const std::string &key, double *tag);

            bool lookupAttributeResolved(const std::string &key, std::string *tag) const;

            bool lookupAttributeResolved(const std::string &key, bool *tag) const;

            bool lookupAttributeResolved(const std::string &key, byte *tag) const;

            bool lookupAttributeResolved(const std::string &key, short *tag) const;

            bool lookupAttributeResolved(const std::string &key, int *tag) const;

            bool lookupAttributeResolved(const std::string &key, long *tag) const;

            bool lookupAttributeResolved(const std::string &key, float *tag) const;

            bool lookupAttributeResolved(const std::string &key, double *tag) const;

            Address address;
            std::string uuid;
            bool liteMember;
            std::map<std::string, std::string> stringAttributes;
            std::map<std::string, bool> boolAttributes;
            std::map<std::string, byte> byteAttributes;
            std::map<std::string, int> intAttributes;
            std::map<std::string, float> floatAttributes;
            std::map<std::string, short> shortAttributes;
            std::map<std::string, long> longAttributes;
            std::map<std::string, double> doubleAttributes;
        };

        std::ostream HAZELCAST_API &operator <<(std::ostream &stream, const Member &member);
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#endif //HAZELCAST_MEMBER

