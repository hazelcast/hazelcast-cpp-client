#!/bin/bash

# Formats all source and header files using ClangFormat.
# However, you should probably use git-clang-format or your IDE to format your
# changes before committing.

find hazelcast examples    \
  | grep -E '\.(cpp|h)$'   \
  | xargs clang-format -i
