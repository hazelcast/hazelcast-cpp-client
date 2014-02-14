//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MEMBER
#define HAZELCAST_MEMBER

#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/client/Address.h"

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
        class HAZELCAST_API Member : public IdentifiedDataSerializable {
        public:
            friend class connection::ClusterListenerThread;

            /**
             * Constructor
             */
            Member();

            /**
             * comparison operation
             */
            bool operator ==(const Member &) const;

            /**
             * comparison operation
             */
            int operator <(const Member &) const;

            /**
             * @see IdentifiedDataSerializable
             */
            int getFactoryId() const;

            /**
             * @see IdentifiedDataSerializable
             */
            int getClassId() const;

            /**
             * @see IdentifiedDataSerializable
             */
            void writeData(serialization::ObjectDataOutput &writer) const;

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
             * @param AttributeType type template for attribute type
             * @param key The key to lookup.
             * @return The value for this members key.
             */
            template <typename AttributeType>
            AttributeType getAttribute(const std::string &key) {
                AttributeType *tag;
                return getAttributeResolved(key, tag);
            }

            /**
             * check if an attribute is defined for given key.
             *
             * @return true if attribute is defined.
             */
            template <typename AttributeType>
            bool lookupAttribute(const std::string &key) const {
                AttributeType *tag;
                return lookupAttributeResolved(key, tag);
            };

        private:
            template <typename AttributeType>
            void setAttribute(const std::string &key, AttributeType value) {
                setAttributeResolved(key, value);
            }

            template <typename AttributeType>
            bool removeAttribute(const std::string &key) {
                AttributeType *tag;
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
            std::map< std::string, std::string > stringAttributes;
            std::map< std::string, bool > boolAttributes;
            std::map< std::string, byte > byteAttributes;
            std::map< std::string, int > intAttributes;
            std::map< std::string, float > floatAttributes;
            std::map< std::string, short > shortAttributes;
            std::map< std::string, long > longAttributes;
            std::map< std::string, double > doubleAttributes;
        };

        inline std::ostream &operator <<(std::ostream &strm, const Member &a) {
            return strm << "Member[" << a.getAddress().getHost() << "]:" << util::to_string(a.getAddress().getPort());
        };
    }
}

#endif //HAZELCAST_MEMBER
