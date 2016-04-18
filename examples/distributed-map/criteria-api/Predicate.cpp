/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/IdentifiedDataSerializable.h>
#include <hazelcast/client/query/PagingPredicate.h>
#include <hazelcast/client/query/QueryConstants.h>
#include <hazelcast/client/query/GreaterLessPredicate.h>
#include <hazelcast/client/query/SqlPredicate.h>
#include "Employee.h"

using namespace hazelcast::client::examples::criteriaapi;
using namespace hazelcast::client;

class Person : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    Person() { }

    Person(const Person &p) : male(p.male), age(p.age) {
        if (NULL != p.name.get()) {
            name = std::auto_ptr<std::string>(new std::string(*p.name));
        }
    }

    Person &operator=(const Person &p) {
        if (NULL != p.name.get()) {
            name = std::auto_ptr<std::string>(new std::string(*p.name));
        }
        male = p.male;
        age = p.age;

        return *this;
    }

    Person(const char *n, bool male, int age)
            : name(std::auto_ptr<std::string>(new std::string(n))), male(male), age(age) { }

    int getFactoryId() const {
        return 1;
    }

    int getClassId() const {
        return 3;
    }

    void writeData(hazelcast::client::serialization::ObjectDataOutput &out) const {
        out.writeUTF(name.get());
        out.writeBoolean(male);
        out.writeInt(age);
    }

    void readData(hazelcast::client::serialization::ObjectDataInput &in) {
        name = in.readUTF();
        male = in.readBoolean();
        age = in.readInt();
    }


    const std::string *getName() const {
        return name.get();
    }

    bool isMale() const {
        return male;
    }

    int getAge() const {
        return age;
    }

private:
    std::auto_ptr<std::string> name;
    bool male;
    int age;
};

std::ostream &operator<<(std::ostream &out, const Person &p) {
    const std::string *n = p.getName();
    out << "Person{"
    << "male=" << p.isMale()
    << ", name='" << (NULL == n ? "No name" : n->c_str()) << "\'"
    << ", age=" << p.getAge()
    << '}';
    return out;
}

class PredicateMember {
public:
    std::vector<Person> getWithName(const char *name, hazelcast::client::IMap<std::string, Person> &personMap) {
        char buf[200];
        hazelcast::util::snprintf(buf, 200, "name == %s", name);
        hazelcast::client::query::SqlPredicate predicate(buf);

        return personMap.values(predicate);
    }

    std::vector<Person> getNotWithName(const char *name, hazelcast::client::IMap<std::string, Person> &personMap) {
        char buf[200];
        hazelcast::util::snprintf(buf, 200, "name != %s", name);
        hazelcast::client::query::SqlPredicate predicate(buf);

        return personMap.values(predicate);
    }

    std::vector<Person> getWithNameAndAge(const char *name, int age,
                                          hazelcast::client::IMap<std::string, Person> &personMap) {
        char buf[300];
        hazelcast::util::snprintf(buf, 300, "name == %s AND age == %d", name, age);
        hazelcast::client::query::SqlPredicate predicate(buf);

        return personMap.values(predicate);
    }

    void run() {
        hazelcast::client::ClientConfig config;
        hazelcast::client::HazelcastClient hz(config);

        hazelcast::client::IMap<std::string, Person> personMap = hz.getMap<std::string, Person>("personMap");

        Person p1("Peter", true, 36);
        Person p2("John", true, 50);
        Person p3("Marry", false, 20);
        Person p4("Mike", true, 35);
        Person p5("Rob", true, 60);
        Person p6("Jane", false, 43);
        personMap.put("1", p1);
        personMap.put("2", p2);
        personMap.put("3", p3);
        personMap.put("4", p4);
        personMap.put("5", p5);
        personMap.put("6", p6);

        hazelcast::client::ISet<Person> s = hz.getSet<Person>("foo");
        Person p("Peter", true, 37);
        s.add(p);
        std::vector<Person> personsInSet = s.toArray();

        std::cout << "Get with name Peter" << std::endl;
        std::vector<Person> values = getWithName("Peter", personMap);
        for (std::vector<Person>::iterator it = values.begin(); it != values.end(); ++it) {
            std::cout << *it << std::endl;
        }

        std::cout << "Get not with name Peter" << std::endl;
        values = getNotWithName("Peter", personMap);
        for (std::vector<Person>::const_iterator it = values.begin(); it != values.end(); ++it) {
            std::cout << *it << std::endl;
        }

        std::cout << "Find name Peter and age 36" << std::endl;
        values = getWithNameAndAge("Peter", 36, personMap);
        for (std::vector<Person>::const_iterator it = values.begin(); it != values.end(); ++it) {
            std::cout << *it << std::endl;
        }

        std::cout << "Find name Peter and age 37" << std::endl;
        values = getWithNameAndAge("Peter", 37, personMap);
        for (std::vector<Person>::const_iterator it = values.begin(); it != values.end(); ++it) {
            std::cout << *it << std::endl;
        }
    }
};

void queryMapUsingPagingPredicate() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient client(config);

    IMap<int, int> intMap = client.getMap<int, int>("testIntMapValuesWithPagingPredicate");

    int predSize = 5;
    const int totalEntries = 25;

    for (int i = 0; i < totalEntries; ++i) {
        intMap.put(i, i);
    }

    query::PagingPredicate<int, int> predicate(predSize);

    std::vector<int> values = intMap.values(predicate);

    predicate.nextPage();
    values = intMap.values(predicate);

    predicate.setPage(4);

    values = intMap.values(predicate);

    predicate.previousPage();
    values = intMap.values(predicate);

    // PagingPredicate with inner predicate (value < 10)
    std::auto_ptr<query::Predicate> lessThanTenPredicate(std::auto_ptr<query::Predicate>(
            new query::GreaterLessPredicate<int>(query::QueryConstants::THIS_ATTRIBUTE_NAME, 9, false, true)));
    query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
    values = intMap.values(predicate2);

    predicate2.nextPage();
    // match values 5,6, 7, 8
    values = intMap.values(predicate2);

    predicate2.nextPage();
    values = intMap.values(predicate2);

    // test paging predicate with comparator
    IMap<int, Employee> employees = client.getMap<int, Employee>("testComplexObjectWithPagingPredicate");

    Employee empl1("ahmet", 35);
    Employee empl2("mehmet", 21);
    Employee empl3("deniz", 25);
    Employee empl4("ali", 33);
    Employee empl5("veli", 44);
    Employee empl6("aylin", 5);

    employees.put(3, empl1);
    employees.put(4, empl2);
    employees.put(5, empl3);
    employees.put(6, empl4);
    employees.put(7, empl5);
    employees.put(8, empl6);

    predSize = 2;
    query::PagingPredicate<int, Employee, EmployeeEntryComparator> predicate3(std::auto_ptr<EmployeeEntryComparator>(new EmployeeEntryComparator()), (size_t)predSize);
    std::vector<Employee> result = employees.values(predicate3);

    predicate3.nextPage();
    result = employees.values(predicate3);

}

void queryMapUsingDifferentPredicates() {

}

int main() {
    PredicateMember m;
    m.run();

    queryMapUsingDifferentPredicates();

    queryMapUsingPagingPredicate();

    std::cout << "Finished" << std::endl;

    return 0;
}

