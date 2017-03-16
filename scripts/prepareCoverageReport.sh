#!/usr/bin/env bash

set -e #abort the script at first failure

gcovr --xml-pretty -o cpp_coverage.xml -r . -e ".*boost.*" -e ".*test.*" -e ".*iTest.*" -e ".*usr.*include.*" -e ".*asio.*" -d



