# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (azure-security-keyvault-secrets-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")
include(TestProxyPrep)
SetUpTestProxy("sdk/keyvault")

add_executable (
  azure-security-keyvault-secrets-test
    challenge_based_authentication_policy_test.cpp
    macro_guard.cpp
    secret_client_base_test.hpp
    secret_client_test.cpp
)

target_compile_definitions(azure-security-keyvault-secrets-test PRIVATE _azure_BUILDING_TESTS)

create_per_service_target_build(keyvault azure-security-keyvault-secrets-test)
create_map_file(azure-security-keyvault-secrets-test azure-security-keyvault-secrets-test.map)

if (MSVC)
  target_compile_options(azure-security-keyvault-secrets-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(azure-security-keyvault-secrets-test PRIVATE azure-security-keyvault-secrets azure-identity azure-core-test-fw gtest gtest_main gmock)

# Adding private headers so we can test the private APIs with no relative paths include.
target_include_directories(
    azure-security-keyvault-secrets-test
      PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../azure-security-keyvault-shared/inc>
)

# gtest_add_tests will scan the test from azure-core-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-security-keyvault-secrets-test
  TEST_PREFIX azure-security-keyvault-secrets-unittest.
  NO_PRETTY_TYPES
  NO_PRETTY_VALUES
  DISCOVERY_TIMEOUT 600)
