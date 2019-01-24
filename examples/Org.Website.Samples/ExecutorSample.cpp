/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include <string>

#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

// The Java server side example MessagePrinter implementation looks like the following (Please observe that Java class
// should implement the Callable and the IdentifiedDataSerializable interfaces):
//
//public class MessagePrinter implements IdentifiedDataSerializable, Callable<String> {
//    private String message;
//
//    public MessagePrinter(String message) {
//        this.message = message;
//    }
//
//    @Override
//    public int getFactoryId() {
//        return 1;
//    }
//
//    @Override
//    public int getId() {
//        return 555;
//    }
//
//    @Override
//    public void writeData(ObjectDataOutput out)
//            throws IOException {
//        out.writeUTF(message);
//    }
//
//    @Override
//    public void readData(ObjectDataInput in)
//            throws IOException {
//        message = in.readUTF();
//    }
//
//    @Override
//    public String call()
//            throws Exception {
//        System.out.println(message);
//        return message;
//    }
//}

class MessagePrinter : public serialization::IdentifiedDataSerializable {
public:
    MessagePrinter(const std::string &message) : message(message) {}

    virtual int getFactoryId() const {
        return 1;
    }

    virtual int getClassId() const {
        return 555;
    }

    virtual void writeData(serialization::ObjectDataOutput &writer) const {
        writer.writeUTF(&message);
    }

    virtual void readData(serialization::ObjectDataInput &reader) {
        // no need to implement since it will not be read by the client in our example
    }

private:
    std::string message;
};

class PrinterCallback : public ExecutionCallback<std::string> {
public:
    virtual void onResponse(const boost::shared_ptr<std::string> &response) {
        std::cout << "The execution of the task is completed successfully and server returned:" << *response
                  << std::endl;
    }

    virtual void onFailure(const boost::shared_ptr<exception::IException> &e) {
        std::cout << "The execution of the task failed with exception:" << e << std::endl;
    }
};

class MyMemberSelector : public hazelcast::client::cluster::memberselector::MemberSelector {
public:
    virtual bool select(const Member &member) const {
        const std::string *attribute = member.getAttribute("my.special.executor");
        if (attribute == NULL) {
            return false;
        }

        return *attribute == "true";
    }

    virtual void toString(std::ostream &os) const {
        os << "MyMemberSelector";
    }
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get the Distributed Executor Service
    boost::shared_ptr<IExecutorService> ex = hz.getExecutorService("my-distributed-executor");
    // Submit the MessagePrinter Runnable to a random Hazelcast Cluster Member
    boost::shared_ptr<ICompletableFuture<std::string> > future = ex->submit<MessagePrinter, std::string>(
            MessagePrinter("message to any node"));
    // Wait for the result of the submitted task and print the result
    boost::shared_ptr<std::string> result = future->get();
    std::cout << "Server result: " << *result << std::endl;
    // Get the first Hazelcast Cluster Member
    Member firstMember = hz.getCluster().getMembers()[0];
    // Submit the MessagePrinter Runnable to the first Hazelcast Cluster Member
    ex->executeOnMember<MessagePrinter>(MessagePrinter("message to very first member of the cluster"), firstMember);
    // Submit the MessagePrinter Runnable to all Hazelcast Cluster Members
    ex->executeOnAllMembers<MessagePrinter>(MessagePrinter("message to all members in the cluster"));
    // Submit the MessagePrinter Runnable to the Hazelcast Cluster Member owning the key called "key"
    ex->executeOnKeyOwner<MessagePrinter, std::string>(
            MessagePrinter("message to the member that owns the key"), "key");
    // Instantiate a callback instance
    boost::shared_ptr<ExecutionCallback<std::string> > callback(new PrinterCallback());
    // Use a callback execution when the task is completed
    ex->submit<MessagePrinter, std::string>(MessagePrinter("Message for the callback"), callback);
    // Choose which member to submit the task to using a member selector
    ex->submit<MessagePrinter, std::string>(MessagePrinter("Message when using the member selector"),
                                            MyMemberSelector());
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
