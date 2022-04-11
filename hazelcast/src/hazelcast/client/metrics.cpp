/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <vector>
#include <iostream>
#include <algorithm>

#include <boost/endian/conversion.hpp>

#include <hazelcast/client/impl/metrics/metric_descriptor.h>
#include <hazelcast/client/impl/metrics/metrics_compressor.h>
#include <hazelcast/client/impl/metrics/metrics_dictionary.h>
#include <hazelcast/util/byte.h>

namespace hazelcast {
namespace client {
namespace impl {
namespace metrics {

namespace {

constexpr int MAX_WORD_LENGTH = 255;
constexpr int NULL_DICTIONARY_ID = -1;

constexpr byte MASK_PREFIX = 1 << 0;
constexpr byte MASK_METRIC = 1 << 1;
constexpr byte MASK_DISCRIMINATOR = 1 << 2;
constexpr byte MASK_DISCRIMINATOR_VALUE = 1 << 3;
constexpr byte MASK_UNIT = 1 << 4;
constexpr byte MASK_EXCLUDED_TARGETS = 1 << 5;
constexpr byte MASK_TAG_COUNT = 1 << 6;

constexpr int VALUE_TYPE_LONG = 0;

constexpr byte BINARY_FORMAT_VERSION = 1;

std::size_t
find_common_prefix_length(const std::string& s1, const std::string& s2)
{
    std::size_t len = 0;
    while (len < s1.size() && len < s2.size() && s1[len] == s2[len]) {
        len++;
    }
    return len;
}

/**
 * ZLIB compress with compression level of 0 (no compression)
 *
 * References:
 *   https://datatracker.ietf.org/doc/html/rfc1950
 *   https://datatracker.ietf.org/doc/html/rfc1951
 *   https://en.wikipedia.org/wiki/Adler-32
 */
std::vector<byte>
zlib_compress(const std::vector<byte>& input)
{
    constexpr std::size_t max_block_size = (1 << 16) - 1;

    const size_t num_blocks =
      (std::max)(static_cast<std::size_t>(1),
                 (input.size() + max_block_size - 1) / max_block_size);

    std::vector<byte> output;

    // reserve enough space beforehand
    output.reserve(input.size()     // input size itself
                   + 2              // zlib header
                   + 4              // Adler32 checksum
                   + 5 * num_blocks // block headers
    );

    // ZLIB header
    output.push_back(static_cast<byte>(120)); // CMF
    output.push_back(static_cast<byte>(1));   // FLG

    constexpr long adler32_mod = 65521;
    long a1 = 1, a2 = 0; // accumulators for Adler32 checksum

    for (std::size_t block_start = 0U;
         block_start == 0U || block_start < input.size();
         block_start += max_block_size) {
        const auto block_end =
          (std::min)(block_start + max_block_size, input.size());
        const auto block_size = block_end - block_start;

        const bool is_final = block_end == input.size();

        // block header
        output.push_back(
          static_cast<byte>(is_final));         // BFINAL = is_final, BTYPE = 00
        output.push_back(block_size & 0xff);    // LEN - least significant
        output.push_back(block_size >> 8);      // LEN - most significant
        output.push_back((~block_size) & 0xff); // NLEN - least significant
        output.push_back((~block_size) >> 8);   // NLEN - most significant

        // copy uncompressed bytes and accumulate checksum
        for (std::size_t i = block_start; i < block_end; i++) {
            const auto x = input[i];

            output.push_back(x);

            a1 += x;
            if (a1 >= adler32_mod) {
                a1 -= adler32_mod;
            }

            a2 += a1;
            if (a2 >= adler32_mod) {
                a2 -= adler32_mod;
            }
        }
    }

    // Adler32 checksum
    output.push_back(a2 >> 8);
    output.push_back(a2 & 0xff);
    output.push_back(a1 >> 8);
    output.push_back(a1 & 0xff);

    return output;
}

} // namespace

metric_descriptor::metric_descriptor(std::string prefix,
                                     std::string metric,
                                     std::string discriminator,
                                     std::string discriminator_value,
                                     probe_unit unit)
  : prefix_{ std::move(prefix) }
  , metric_{ std::move(metric) }
  , discriminator_{ std::move(discriminator) }
  , discriminator_value_{ std::move(discriminator_value) }
  , unit_{ unit }
{}

metric_descriptor::metric_descriptor(std::string prefix,
                                     std::string metric,
                                     probe_unit unit)
  : prefix_{ std::move(prefix) }
  , metric_{ std::move(metric) }
  , discriminator_{}
  , discriminator_value_{}
  , unit_{ unit }
{}

const std::string&
metric_descriptor::prefix() const
{
    return prefix_;
}

const std::string&
metric_descriptor::metric() const
{
    return metric_;
}

const boost::optional<std::string>&
metric_descriptor::discriminator() const
{
    return discriminator_;
}

const boost::optional<std::string>&
metric_descriptor::discriminator_value() const
{
    return discriminator_value_;
}

probe_unit
metric_descriptor::unit() const
{
    return unit_;
}

int
metrics_dictionary::get_dictionary_id(const std::string& word)
{
    if (word.size() > MAX_WORD_LENGTH) {
        throw std::invalid_argument("too long value in metric descriptor");
    }

    auto word_position = word_to_id.find(word);

    if (word_position == word_to_id.end()) {
        const int next_id = static_cast<int>(word_to_id.size());
        word_to_id[word] = next_id;

        return next_id;
    } else {
        return word_position->second;
    }
}

metrics_dictionary::const_iterator
metrics_dictionary::begin() const noexcept
{
    return word_to_id.cbegin();
}

metrics_dictionary::const_iterator
metrics_dictionary::end() const noexcept
{
    return word_to_id.cend();
}

std::size_t
metrics_dictionary::size() const noexcept
{
    return word_to_id.size();
}

void
output_buffer::write(byte val)
{
    buffer_.push_back(val);
}

void
output_buffer::write(int32_t val)
{
    auto pos = buffer_.size();
    buffer_.resize(pos + sizeof(int32_t));

    boost::endian::endian_store<int32_t,
                                sizeof(int32_t),
                                boost::endian::order::big>(buffer_.data() + pos,
                                                           val);
}

void
output_buffer::write(int64_t val)
{
    auto pos = buffer_.size();
    buffer_.resize(pos + sizeof(int64_t));

    boost::endian::endian_store<int64_t,
                                sizeof(int64_t),
                                boost::endian::order::big>(buffer_.data() + pos,
                                                           val);
}

void
output_buffer::write(const std::string& str)
{
    for (char c : str) {
        buffer_.push_back(static_cast<byte>(0));
        buffer_.push_back(c);
    }
}

void
output_buffer::write(const std::vector<byte>& vec)
{
    buffer_.insert(buffer_.end(), vec.begin(), vec.end());
}

const std::vector<byte>&
output_buffer::content() const
{
    return buffer_;
}

std::vector<byte>&
output_buffer::content()
{
    return buffer_;
}

void
metrics_compressor::add_long(const metric_descriptor& descriptor, int64_t value)
{
    write_descriptor(descriptor);
    metrics_buffer_.write(static_cast<byte>(VALUE_TYPE_LONG));
    metrics_buffer_.write(value);
}

std::vector<byte>
metrics_compressor::get_blob()
{
    write_dictionary();

    std::vector<byte> compressed_dictionary =
      zlib_compress(dictionary_buffer_.content());
    std::vector<byte> compressed_metrics =
      zlib_compress(metrics_buffer_.content());

    output_buffer blob;

    blob.write(static_cast<byte>(0));
    blob.write(BINARY_FORMAT_VERSION);
    blob.write(static_cast<int32_t>(compressed_dictionary.size()));
    blob.write(compressed_dictionary);
    blob.write(static_cast<int32_t>(metrics_count));
    blob.write(compressed_metrics);

    return std::move(blob.content());
}

byte
metrics_compressor::calculate_descriptor_mask(
  const metric_descriptor& descriptor)
{
    byte mask = 0;

    if (last_descriptor_) {
        if (descriptor.prefix() == last_descriptor_->prefix()) {
            mask |= MASK_PREFIX;
        }

        if (descriptor.metric() == last_descriptor_->metric()) {
            mask |= MASK_METRIC;
        }

        if (descriptor.discriminator() == last_descriptor_->discriminator()) {
            mask |= MASK_DISCRIMINATOR;
        }

        if (descriptor.discriminator_value() ==
            last_descriptor_->discriminator_value()) {
            mask |= MASK_DISCRIMINATOR_VALUE;
        }

        if (descriptor.unit() == last_descriptor_->unit()) {
            mask |= MASK_UNIT;
        }

        // include excludedTargets and tagCount bits for compatibility purposes
        mask |= MASK_EXCLUDED_TARGETS;
        mask |= MASK_TAG_COUNT;
    }

    return mask;
}

int32_t
metrics_compressor::get_dictionary_id(const boost::optional<std::string>& word)
{
    if (!word) {
        return NULL_DICTIONARY_ID;
    }

    return static_cast<int32_t>(dictionary_.get_dictionary_id(word.get()));
}

void
metrics_compressor::write_descriptor(const metric_descriptor& descriptor)
{
    byte mask = calculate_descriptor_mask(descriptor);

    metrics_buffer_.write(mask);

    if ((mask & MASK_PREFIX) == 0) {
        metrics_buffer_.write(get_dictionary_id(descriptor.prefix()));
    }

    if ((mask & MASK_METRIC) == 0) {
        metrics_buffer_.write(get_dictionary_id(descriptor.metric()));
    }

    if ((mask & MASK_DISCRIMINATOR) == 0) {
        metrics_buffer_.write(get_dictionary_id(descriptor.discriminator()));
    }

    if ((mask & MASK_DISCRIMINATOR_VALUE) == 0) {
        metrics_buffer_.write(static_cast<int32_t>(
          get_dictionary_id(descriptor.discriminator_value())));
    }

    if ((mask & MASK_UNIT) == 0) {
        metrics_buffer_.write(static_cast<byte>(descriptor.unit()));
    }

    if ((mask & MASK_EXCLUDED_TARGETS) == 0) {
        metrics_buffer_.write(static_cast<byte>(0));
    }

    if ((mask & MASK_TAG_COUNT) == 0) {
        metrics_buffer_.write(static_cast<byte>(0));
    }

    last_descriptor_ = descriptor;
    metrics_count++;
}

void
metrics_compressor::write_dictionary()
{
    std::string last_word{ "" };

    dictionary_buffer_.write(static_cast<int32_t>(dictionary_.size()));

    for (const auto& item : dictionary_) {
        const auto& word = item.first;
        const auto& id = item.second;

        auto common_len = find_common_prefix_length(last_word, word);

        dictionary_buffer_.write(static_cast<int32_t>(id));
        dictionary_buffer_.write(static_cast<byte>(common_len));
        dictionary_buffer_.write(static_cast<byte>(word.size() - common_len));
        dictionary_buffer_.write(word.substr(common_len));

        last_word = word;
    }
}

} // namespace metrics
} // namespace impl
} // namespace client
} // namespace hazelcast
