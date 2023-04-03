#include <hazelcast/client/hazelcast_client.h>
int main() {
    auto hz = hazelcast::new_client().get(); // Connects to the cluster

    auto personel = hz.get_map("personel_map").get();
    personel->put<std::string, std::string>("Alice", "IT").get();
    personel->put<std::string, std::string>("Bob", "IT").get();
    personel->put<std::string, std::string>("Clark", "IT").get();
    std::cout << "Added IT personel. Logging all known personel" << std::endl;
    for (const auto &entry : personel->entry_set<std::string, std::string>().get()) {
        std::cout << entry.first << " is in " << entry.second << " department." << std::endl;
    }
    
    return 0;
}