# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-identity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Compile Options
option(FETCH_SOURCE_DEPS "build source dependencies" OFF)

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(IDENTITY)
endif()

include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
include(AzureBuildTargetForCI)
# Add create_map_file function
include(CreateMapFile)

if(FETCH_SOURCE_DEPS)
    GetFolderList(IDENTITY)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder} EXCLUDE_FROM_ALL)
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

set(
  AZURE_IDENTITY_HEADER
    inc/azure/identity.hpp
    inc/azure/identity/azure_cli_credential.hpp
    inc/azure/identity/azure_pipelines_credential.hpp
    inc/azure/identity/chained_token_credential.hpp
    inc/azure/identity/client_assertion_credential.hpp
    inc/azure/identity/client_certificate_credential.hpp
    inc/azure/identity/client_secret_credential.hpp
    inc/azure/identity/default_azure_credential.hpp
    inc/azure/identity/detail/client_credential_core.hpp
    inc/azure/identity/detail/token_cache.hpp
    inc/azure/identity/dll_import_export.hpp
    inc/azure/identity/environment_credential.hpp
    inc/azure/identity/managed_identity_credential.hpp
    inc/azure/identity/rtti.hpp
    inc/azure/identity/workload_identity_credential.hpp
)

set(
  AZURE_IDENTITY_SOURCE
    src/azure_cli_credential.cpp
    src/azure_pipelines_credential.cpp
    src/chained_token_credential.cpp
    src/client_assertion_credential.cpp
    src/client_certificate_credential.cpp
    src/client_credential_core.cpp
    src/client_secret_credential.cpp
    src/default_azure_credential.cpp
    src/environment_credential.cpp
    src/managed_identity_credential.cpp
    src/managed_identity_source.cpp
    src/private/chained_token_credential_impl.hpp
    src/private/client_assertion_credential_impl.hpp
    src/private/identity_log.hpp
    src/private/managed_identity_source.hpp
    src/private/package_version.hpp
    src/private/tenant_id_resolver.hpp
    src/private/token_credential_impl.hpp
    src/tenant_id_resolver.cpp
    src/token_cache.cpp
    src/token_credential_impl.cpp
    src/workload_identity_credential.cpp
)

add_library(azure-identity ${AZURE_IDENTITY_HEADER} ${AZURE_IDENTITY_SOURCE})
create_per_service_target_build(identity azure-identity)

# make sure that users can consume the project as a library.
add_library(Azure::azure-identity ALIAS azure-identity)

create_code_coverage(identity azure-identity azure-identity-test "tests?/*;samples?/*")

target_include_directories(
  azure-identity
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-identity PUBLIC Azure::azure-core)

target_compile_definitions(azure-identity PRIVATE _azure_BUILDING_SDK)

if(WIN32 AND NOT(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore" AND CMAKE_SYSTEM_VERSION STREQUAL "10.0"))
  find_package(wil CONFIG REQUIRED)
  target_link_libraries(azure-identity PRIVATE WIL::WIL bcrypt crypt32)
else()
  find_package(OpenSSL REQUIRED)
  target_link_libraries(azure-identity PRIVATE OpenSSL::Crypto)
endif()

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
set_target_properties(azure-identity PROPERTIES VERSION ${AZ_LIBRARY_VERSION})
generate_documentation(azure-identity ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-identity
    IDENTITY
    "azure/identity/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-identity
  IDENTITY
  "azure/identity/rtti.hpp"
)

if(BUILD_TESTING)
  # define a symbol that enables some test hooks in code
  add_compile_definitions(_azure_TESTING_BUILD)

  # tests
  if (NOT AZ_ALL_LIBRARIES OR FETCH_SOURCE_DEPS)
    include(AddGoogleTest)
    enable_testing ()
  endif()

  add_subdirectory(test/e2e)
  add_subdirectory(test/ut)

endif()

if (BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

if (BUILD_PERFORMANCE_TESTS)
  add_subdirectory(test/perf)
endif()

unset(FETCH_SOURCE_DEPS CACHE)
