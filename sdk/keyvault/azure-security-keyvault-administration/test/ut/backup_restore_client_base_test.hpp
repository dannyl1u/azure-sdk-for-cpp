// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief The base class to construct and init a Key Vault client.
 *
 */

#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/administration.hpp>

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Administration {
        namespace Test {

  class BackupRestoreClientTest : public Azure::Core::Test::TestBase,
                                  public ::testing::WithParamInterface<int> {

  public:
    BackupRestoreClientTest() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); }
    void CreateHSMClientForTest(std::string hsmUrl = "")
    {
      BackupClientOptions options;
      m_client = InitTestClient<
          Azure::Security::KeyVault::Administration::BackupClient,
          BackupClientOptions>(
          hsmUrl.length() == 0 ? m_keyVaultHsmUrl : hsmUrl, m_credential, options);
    }

    Models::SasTokenParameter GetSasTokenBackup(bool managedIdentity = false)
    {
      Models::SasTokenParameter sasTokenParameter;
      // the backup/restore needs a SAS token to access the storage account
      sasTokenParameter.Token = GetEnv("AZURE_KEYVAULT_BACKUP_TOKEN");
      // the backup/restore needs a url to a blob storage resource
      m_blobUrl = Azure::Core::Url(GetEnv("AZURE_KEYVAULT_BACKUP_URL"));

      sasTokenParameter.UseManagedIdentity = managedIdentity;
      return sasTokenParameter;
    }

  private:
    std::unique_ptr<Azure::Security::KeyVault::Administration::BackupClient> m_client;

  protected:
    std::shared_ptr<const Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_keyVaultUrl;
    std::string m_keyVaultHsmUrl;
    Azure::Core::Url m_blobUrl;
    std::chrono::milliseconds m_testPollingIntervalMs = std::chrono::seconds(1);

    // Reads the current test instance name.
    // Name gets also sanitized (special chars are removed) to avoid issues when recording or
    // creating. This also return the name with suffix if the "AZURE_LIVE_TEST_SUFFIX" exists.
    std::string GetTestName(bool sanitize = true)
    {
      auto output = m_keyVaultUrl.compare(m_keyVaultHsmUrl) == 0 ? "Same" : "NotSame";
      std::cout << "\n Keyvault and HSM are" << output;
      return Azure::Core::Test::TestBase::GetTestNameSuffix(sanitize);
    }

    Azure::Security::KeyVault::Administration::BackupClient& GetClientForTest(
        std::string const& testName)
    {
      // set the interceptor for the current test
      m_testContext.RenameTest(testName);
      return *m_client;
    }

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
      m_keyVaultUrl = GetEnv("AZURE_KEYVAULT_URL");
      m_keyVaultHsmUrl = GetEnv("AZURE_KEYVAULT_HSM_URL");

      // Options and credential for the client
      BackupClientOptions options;
      m_credential = GetTestCredential();

      // `InitTestClient` takes care of setting up Record&Playback.
      m_client = InitTestClient<
          Azure::Security::KeyVault::Administration::BackupClient,
          BackupClientOptions>(m_keyVaultUrl, m_credential, options);

      UpdateWaitingTime(m_testPollingIntervalMs);
    }

  public:
    template <class T>
    static inline void CheckValidResponse(
        Azure::Response<T>& response,
        Azure::Core::Http::HttpStatusCode expectedCode = Azure::Core::Http::HttpStatusCode::Ok)
    {
      auto const& rawResponse = response.RawResponse;
      EXPECT_EQ(
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              rawResponse->GetStatusCode()),
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              expectedCode));
    }

    static inline std::string GetUniqueName() { return Azure::Core::Uuid::CreateUuid().ToString(); }
  };
}}}}} // namespace Azure::Security::KeyVault::Administration::Test
