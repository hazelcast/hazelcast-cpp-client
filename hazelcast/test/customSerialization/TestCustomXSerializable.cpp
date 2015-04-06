#include "customSerialization/TestCustomXSerializable.h"

namespace hazelcast {
    namespace client {
        namespace test {


            TestCustomXSerializable::TestCustomXSerializable() {

            }

            TestCustomXSerializable::TestCustomXSerializable(int id) :id(id){

            }

            bool TestCustomXSerializable::operator==(const TestCustomXSerializable& rhs) const {
                if (this == &rhs)
                    return true;
                if (id != rhs.id) return false;
                return true;
            }

            bool TestCustomXSerializable::operator!=(const TestCustomXSerializable& rhs) const {
                return !(*this == rhs);
            }

            int TestCustomXSerializable::getTypeId() const {
                return 666;
            }

            TestCustomPerson::TestCustomPerson() {

            }

            TestCustomPerson::TestCustomPerson(const std::string& name):name(name) {

            }

            bool TestCustomPerson::operator==(const TestCustomPerson& rhs) const {
                if (this == &rhs)
                    return true;
                if (name.compare(rhs.name))
                    return false;
                return true;
            }

            bool TestCustomPerson::operator!=(const TestCustomPerson& rhs) const {
                return !(*this == rhs);
            }

            void TestCustomPerson::setName(const std::string& name) {
                this->name = name;
            }

            std::string TestCustomPerson::getName() const {
                return name;
            }

            int TestCustomPerson::getTypeId() const {
                return 999;
            }

        }
    }
}

