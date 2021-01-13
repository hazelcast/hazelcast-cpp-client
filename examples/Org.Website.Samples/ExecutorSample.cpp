/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include <hazelcast/client/hazelcast.h>
#include <hazelcast/client/member_selectors.h>

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
//    public void writeData(object_data_output out)
//            throws io {
//        out.writeUTF(message);
//    }
//
//    @Override
//    public void readData(object_data_input in)
//            throws io {
//        message = in.readUTF();
//    }
//
//    @Override
//    public String call() throws Exception {
//        System.out.println(message);
//        return message;
//    }
//}

struct MessagePrinter {
    std::string message;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<MessagePrinter> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 555;
                }

                static void
                write_data(const MessagePrinter &object, hazelcast::client::serialization::object_data_output &out) {
                    out.write(object.message);
                }

                static MessagePrinter read_data(hazelcast::client::serialization::object_data_input &in) {
                    return MessagePrinter{in.read<std::string>()};
                }
            };
        }
    }
}

class PrinterCallback : public execution_callback<std::string> {
public:
    void on_response(const boost::optional<std::string> &response) override {
        std::cout << "The execution of the task is completed successfully and server returned:" << *response
                  << std::endl;
    }

    void on_failure(std::exception_ptr e) override {
        try {
            std::rethrow_exception(e);
        } catch (hazelcast::client::exception::iexception &e) {
            std::cout << "The execution of the task failed with exception:" << e << std::endl;
        }
    }
};

class MyMemberSelector : public member_selector {
public:
    bool select(const member &member) const override {
        const std::string *attribute = member.get_attribute("my.special.executor");
        if (attribute == NULL) {
            return false;
        }

        return *attribute == "true";
    }
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    auto hz = hazelcast::new_client().get();
    // Get the Distributed Executor Service
    std::shared_ptr<iexecutor_service> ex = hz.get_executor_service("my-distributed-executor").get();
    // Submit the MessagePrinter Runnable to a random Hazelcast Cluster Member
    auto result_future = ex->submit<MessagePrinter, std::string>(MessagePrinter{"message to any node"});
    // Wait for the result of the submitted task and print the result
    auto result = result_future.get_future().get();
    std::cout << "Server result: " << *result << std::endl;
    // Get the first Hazelcast Cluster Member
    member firstMember = hz.get_cluster().get_members()[0];
    // Submit the MessagePrinter Runnable to the first Hazelcast Cluster Member
    ex->execute_on_member<MessagePrinter>(MessagePrinter{"message to very first member of the cluster"}, firstMember);
    // Submit the MessagePrinter Runnable to all Hazelcast Cluster Members
    ex->execute_on_all_members<MessagePrinter>(MessagePrinter{"message to all members in the cluster"});
    // Submit the MessagePrinter Runnable to the Hazelcast Cluster Member owning the key called "key"
    ex->execute_on_key_owner<MessagePrinter, std::string>(
            MessagePrinter{"message to the member that owns the key"}, "key");
    // Use a callback execution when the task is completed
    ex->submit<MessagePrinter, std::string>(MessagePrinter{"Message for the callback"},
                                            std::make_shared<PrinterCallback>());
    // Choose which member to submit the task to using a member selector
    ex->submit<MessagePrinter, std::string>(MessagePrinter{"Message when using the member selector"},
                                            MyMemberSelector());
    // Shutdown this Hazelcast Client
    hz.shutdown().get();

    return 0;
}
