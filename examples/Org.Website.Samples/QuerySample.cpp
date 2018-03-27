#include <hazelcast/client/HazelcastAll.h>
#include <hazelcast/client/query/SqlPredicate.h>
#include <hazelcast/client/query/AndPredicate.h>
#include <hazelcast/client/query/EqualPredicate.h>
#include <hazelcast/client/query/BetweenPredicate.h>
#include <hazelcast/client/serialization/PortableWriter.h>
#include <hazelcast/client/serialization/PortableReader.h>
#include <ostream>

using namespace hazelcast::client;

class User : public serialization::Portable {
public:
    static const int CLASS_ID = 1;

    User(const std::string &username, int age, bool active) : username(username), age(age), active(active) {
    }

    User() : age(0), active(false) {
    }

    virtual int getFactoryId() const {
        return 1;
    }

    virtual int getClassId() const {
        return CLASS_ID;
    }

    virtual void writePortable(serialization::PortableWriter &writer) const {
        writer.writeUTF("username", &username);
        writer.writeInt("age", age);
        writer.writeBoolean("active", active);
    }

    virtual void readPortable(serialization::PortableReader &reader) {
        username = *reader.readUTF("username");
        age = reader.readInt("age");
        active = reader.readBoolean("active");
    }

    friend std::ostream &operator<<(std::ostream &os, const User &user) {
        os << "User{" << " username: " << user.username << " age: " << user.age << " active: " << user.active << '}';
        return os;
    }

private:
    std::string username;
    int age;
    bool active;
};

class ThePortableFactory : public serialization::PortableFactory {
public:
    static const int FACTORY_ID = 1;

    virtual std::auto_ptr<serialization::Portable> create(int32_t classId) const {
        if (classId == User::CLASS_ID) {
            return std::auto_ptr<serialization::Portable>(new User());
        }

        return std::auto_ptr<serialization::Portable>();
    }
};

void generateUsers(IMap<std::string, User> &users) {
    users.put("Rod", User("Rod", 19, true));
    users.put("Jane", User("Jane", 20, true));
    users.put("Freddy", User("Freddy", 23, true));
}

int main() {
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().addPortableFactory(ThePortableFactory::FACTORY_ID,
                                                             boost::shared_ptr<serialization::PortableFactory>(
                                                                     new ThePortableFactory()));
    HazelcastClient hz(clientConfig);
    // Get a Distributed Map called "users"
    IMap<std::string, User> users = hz.getMap("users");
    // Add some users to the Distributed Map
    generateUsers(users);
    // Create a Predicate from a String (a SQL like Where clause)
    query::SqlPredicate sqlQuery = query::SqlPredicate("active AND age BETWEEN 18 AND 21)");
    // Creating the same Predicate as above but with AndPredicate builder
    query::AndPredicate criteriaQuery;
    criteriaQuery.add(std::auto_ptr<query::Predicate>(new query::EqualPredicate<bool>("active", true)))
            .add(std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>("age", 18, 21)));
    // Get result collections using the two different Predicates
    std::vector<User> result1 = users.values(sqlQuery);
    std::vector<User> result2 = users.values(criteriaQuery);
    // Print out the results
    std::cout << "Result 1:" << std::endl;
    for (std::vector<User>::const_iterator it = result1.begin(); it != result1.end(); ++it) {
        std::cout << (*it) << std::endl;
    }
    std::cout << "Result 2:" << std::endl;
    for (std::vector<User>::const_iterator it = result2.begin(); it != result2.end(); ++it) {
        std::cout << (*it) << std::endl;
    }
    hz.shutdown();

    return 0;
}
