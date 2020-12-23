#include <vector>
#include <atomic>
#include <signal.h>

#include <hazelcast/client/hazelcast.h>

struct identified_entry_processor {
    std::string value;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<identified_entry_processor> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 66;
                }

                static int32_t get_class_id() noexcept {
                    return 1;
                }

                static void write_data(const identified_entry_processor &object,
                                       hazelcast::client::serialization::object_data_output &out) {
                    out.write(object.value);
                }

                static identified_entry_processor read_data(hazelcast::client::serialization::object_data_input &in) {
                    return identified_entry_processor{in.read<std::string>()};
                }
            };
        }
    }
}

std::atomic<bool> is_cancelled{false};

void signalHandler(int s) {
    std::cerr << "Caught signal: " << s << std::endl;
    is_cancelled = true;
}

void register_signal_handler() {
    signal(SIGINT, signalHandler);
}

int main(int argc, char *args[]) {
    if (argc > 4 || argc < 3) {
        std::cerr << "USAGE: soak_test thread_count server_address [client_name] " << std::endl;
        return -1;
    }

    const int thread_count = atoi(args[1]);
    const std::string server_address = args[2];
    hazelcast::client::client_config config;

    if (argc > 3) {
        config.set_instance_name(args[3]);
    }

    config.get_network_config().add_address(hazelcast::client::address(server_address, 5701));
    hazelcast::client::hazelcast_client hz(std::move(config));
    hazelcast::client::spi::ClientContext context(hz);
    auto &logger_ = context.get_logger();
    auto map = hz.get_map("test").get();

    HZ_LOG(logger_, info, (boost::format(
            "Soak test is starting with the following parameters: threadCount = %1% ,  server address = %2%") %
                           thread_count % server_address).str());

    register_signal_handler();

    std::vector<boost::future<void>> tasks;

    for (int i = 0; i < thread_count; i++) {
        tasks.emplace_back(boost::async([&]() {
            HZ_LOG(logger_, info, (boost::format("Thread %1% is started") % std::this_thread::get_id()).str());

            int64_t get_count = 0;
            int64_t put_count = 0;
            int64_t values_count = 0;
            int64_t execute_on_key_count = 0;
            int entry_count = 10000;

            std::random_device rd;  //Will be used to obtain a seed for the random number engine
            std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
            std::uniform_int_distribution<> distrib(0, entry_count);

            while (!is_cancelled) {
                int operation = distrib(gen) % 100;
                auto key = std::to_string(distrib(gen) % entry_count);
                try {
                    if (operation < 30) {
                        map->get<std::string, std::string>(key).get();
                        ++get_count;
                    } else if (operation < 60) {
                        map->put(key, std::to_string(distrib(gen))).get();
                        ++put_count;
                    } else if (operation < 80) {
                        map->values<std::string>(hazelcast::client::query::between_predicate(hz,
                                                                                             hazelcast::client::query::query_constants::THIS_ATTRIBUTE_NAME,
                                                                                             std::string("1"),
                                                                                             std::string("10"))).get();
                        ++values_count;
                    } else {
                        map->execute_on_key<std::string, std::string, identified_entry_processor>(key, identified_entry_processor{std::to_string(distrib(gen))}).get();
                        ++execute_on_key_count;
                    }

                    int64_t total_count = put_count + get_count + values_count + execute_on_key_count;
                    if (total_count % 10000 == 0) {
                        HZ_LOG(logger_, info, (boost::format(
                                "Thread %1% --> Total: %2% {get count: %3% , put count: %4% , values count: %5% , execute_on_key count: %6%") %
                                               std::this_thread::get_id() % total_count % get_count % put_count
                                               %values_count %execute_on_key_count).str());
                    }
                } catch (std::exception &e) {
                    HZ_LOG(logger_, warning, (boost::format("Exception occured: %1%") % e.what()).str());
                }
            }

            int64_t total_count = put_count + get_count + values_count + execute_on_key_count;
            HZ_LOG(logger_, info, (boost::format(
                    "Thread %1% is finished. --> Total: %2% {get count: %3% , put count: %4% , values count: %5% , execute_on_key count: %6%") %
                                   std::this_thread::get_id() % total_count % get_count % put_count % values_count
                                   %execute_on_key_count).str());
        }));
    }

    boost::wait_for_all(tasks.begin(), tasks.end());

    return 0;
}

