#include <vector>
#include <string>
#include <fstream>

#include <gtest/gtest.h>

#include <hazelcast/client/impl/metrics/metric_descriptor.h>
#include <hazelcast/client/impl/metrics/metrics_compressor.h>
#include <hazelcast/client/impl/metrics/metrics_dictionary.h>


namespace hazelcast {
namespace client {
namespace impl {
namespace metrics {
namespace test {

TEST(metric_descriptor_test, test_ctor) {
    metric_descriptor d{ "foo", "bar", "disc", "disc_val", probe_unit::MS };

    ASSERT_EQ("foo", d.prefix());
    ASSERT_EQ("bar", d.metric());
    ASSERT_TRUE(d.discriminator().has_value());
    ASSERT_EQ("disc", d.discriminator().get());
    ASSERT_TRUE(d.discriminator_value().has_value());
    ASSERT_EQ("disc_val", d.discriminator_value().get());
    ASSERT_EQ(probe_unit::MS, d.unit());
}

TEST(metric_descriptor_test, test_ctor_without_unit) {
    metric_descriptor d{ "foo", "bar", "disc", "disc_val" };

    ASSERT_EQ("foo", d.prefix());
    ASSERT_EQ("bar", d.metric());
    ASSERT_TRUE(d.discriminator().has_value());
    ASSERT_EQ("disc", d.discriminator().get());
    ASSERT_TRUE(d.discriminator_value().has_value());
    ASSERT_EQ("disc_val", d.discriminator_value().get());
    ASSERT_EQ(probe_unit::NONE, d.unit());
}

TEST(metric_descriptor_test, test_ctor_without_discriminator_and_unit) {
    metric_descriptor d{ "foo", "bar" };

    ASSERT_EQ("foo", d.prefix());
    ASSERT_EQ("bar", d.metric());
    ASSERT_FALSE(d.discriminator().has_value());
    ASSERT_FALSE(d.discriminator_value().has_value());
    ASSERT_EQ(probe_unit::NONE, d.unit());
}


TEST(metrics_dictionary_test, test_get_dictionary_id) {
    metrics_dictionary dict;

    ASSERT_EQ(0, dict.get_dictionary_id("foo"));
    ASSERT_EQ(0, dict.get_dictionary_id("foo"));

    ASSERT_EQ(1, dict.get_dictionary_id("bar"));
    ASSERT_EQ(1, dict.get_dictionary_id("bar"));
    ASSERT_EQ(0, dict.get_dictionary_id("foo"));
}

TEST(metrics_dictionary_test, test_begin_end) {
    metrics_dictionary dict;

    dict.get_dictionary_id("foo");
    dict.get_dictionary_id("bar");

    using item = std::pair<std::string, int>;

    auto actual_items = std::vector<item>{ dict.begin(), dict.end() };
    auto expected_items = std::vector<item>{ {"bar", 1}, {"foo", 0} };
   
    ASSERT_EQ(expected_items, actual_items);
}


TEST(metrics_compressor_test, test_blob) {
    metrics_compressor compressor;

    compressor.add_long({ "prefix1", "metric1", "name", "john", probe_unit::COUNT }, 42);

    compressor.add_long({ "prefix1", "metric2", "name", "josh", probe_unit::MS }, -111);

    compressor.add_long({ "prefix2", "metric1", "disc", "val", probe_unit::BYTES }, 0);
    
    compressor.add_long({ "prefix2", "metric3", "name", "john" }, 123);
    
    compressor.add_long({ "prefix2", "metric5", "id", "15" }, 32);

    compressor.add_long({ "prefix3", "metric5", "id2", "15", probe_unit::PERCENT }, 83);

    compressor.add_long(
      { std::string(255, 'a'), std::string(255, 'b'), "name", "x", probe_unit::COUNT },
      0x7FFFFFFFFFFFFFFFLL);

    auto actual_blob = compressor.get_blob();
    
    std::ifstream file("hazelcast/test/resources/metrics_blob.bin", std::ios::binary);
    file.unsetf(std::ios::skipws);

    std::vector<byte> expected_blob{ std::istream_iterator<byte>{ file }, std::istream_iterator<byte>{} };

    ASSERT_EQ(expected_blob, actual_blob);
}

} // namespace test
} // namespace metrics
} // namespace impl
} // namespace client
} // namespace hazelcast
