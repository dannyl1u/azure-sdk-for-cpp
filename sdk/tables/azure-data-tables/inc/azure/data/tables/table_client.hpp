// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "azure/data/tables/models.hpp"
#include "azure/data/tables/table_audience.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/response.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifdef _azure_TABLES_TESTING_BUILD
namespace Azure { namespace Data { namespace Tables { namespace StressTest {
  class TransactionStressTest;
}}}} // namespace Azure::Data::Tables::StressTest
namespace Azure { namespace Data { namespace Test {
  class TransactionsBodyTest_TransactionCreate_Test;
  class TransactionsBodyTest_TransactionBodyInsertMergeOp_Test;
  class TransactionsBodyTest_TransactionBodyInsertReplaceOp_Test;
  class TransactionsBodyTest_TransactionBodyDeleteOp_Test;
  class TransactionsBodyTest_TransactionBodyUpdateMergeOp_Test;
  class TransactionsBodyTest_TransactionBodyUpdateReplaceOp_Test;
  class TransactionsBodyTest_TransactionBodyAddOp_Test;
}}} // namespace Azure::Data::Test
#endif

namespace Azure { namespace Data { namespace Tables {
  /**
   * @brief Optional parameters for constructing a new TableClient.
   */
  struct TableClientOptions final : Azure::Core::_internal::ClientOptions
  {
    /**
     * API version used by this client.
     */
    std::string ApiVersion{"2019-02-02"};

    /**
     * Enables tenant discovery through the authorization challenge when the client is configured to
     * use a TokenCredential. When enabled, the client will attempt an initial un-authorized request
     * to prompt a challenge in order to discover the correct tenant for the resource.
     */
    bool EnableTenantDiscovery = false;

    /**
     * The Audience to use for authentication with Azure Active Directory (AAD).
     * Audience will be assumed based on serviceUrl if it is not set.
     */
    Azure::Nullable<TableAudience> Audience;
  };

  /**
   * @brief Table Client
   */
  class TableClient final {
  public:
    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param serviceUrl The URL of the service account that is the target of the desired operation.
     * The URL may contain SAS query parameters.
     * @param tableName The name of the table.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableClient(
        std::string const& serviceUrl,
        std::string const& tableName,
        const TableClientOptions& options = {});
    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param serviceUrl The URL of the service account that is the target of the desired operation.
     * The URL may contain SAS query parameters.
     * @param tableName The name of the table.
     * @param credential The shared key credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableClient(
        const std::string& serviceUrl,
        const std::string& tableName,
        std::shared_ptr<const Core::Credentials::TokenCredential> credential,
        const TableClientOptions& options = {});

    /**
     * @brief Add entity in a table.
     *
     * @param tableEntity The TableEntity to set.
     * @param context for canceling long running operations.
     * @return Add entity result.
     */
    Response<Models::AddEntityResult> AddEntity(
        Models::TableEntity const& tableEntity,
        Core::Context const& context = {}) const;

    /**
     * @brief Update entity in a table.
     *
     * @param tableEntity The TableEntity to set.
     * @param context for canceling long running operations.
     * @return Update entity result.
     */
    Response<Models::UpdateEntityResult> UpdateEntity(
        Models::TableEntity const& tableEntity,
        Core::Context const& context = {}) const;

    /**
     * @brief Merge entity in a table.
     *
     * @param tableEntity The TableEntity to merge.
     * @param context for canceling long running operations.
     * @return Merge entity result.
     */
    Response<Models::MergeEntityResult> MergeEntity(
        Models::TableEntity const& tableEntity,
        Core::Context const& context = {}) const;

    /**
     * @brief Deletes the specified entity in a table.
     *
     * @param tableEntity The TableEntity to delete.
     * @param context for canceling long running operations.
     * @return Delete entity result.
     */
    Response<Models::DeleteEntityResult> DeleteEntity(
        Models::TableEntity const& tableEntity,
        Core::Context const& context = {}) const;

    /**
     * @brief Update or insert specified entity in a table.
     *
     * @param tableEntity The TableEntity to update or insert.
     * @param context for canceling long running operations.
     * @return Update entity result.
     */
    Response<Models::UpdateEntityResult> UpdateOrInsertEntity(
        Models::TableEntity const& tableEntity,
        Core::Context const& context = {}) const;

    /**
     * @brief Merge or insert the specified entity in a table.
     *
     * @param tableEntity The TableEntity to merge or insert.
     * @param context for canceling long running operations.
     * @return Merge entity result.
     */
    Response<Models::MergeEntityResult> MergeOrInsertEntity(
        Models::TableEntity const& tableEntity,
        Core::Context const& context = {}) const;
    /**
     * @brief Queries entities in a table.
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Entity list paged response.
     */
    Models::QueryEntitiesPagedResponse QueryEntities(
        Models::QueryEntitiesOptions const& options = {},
        Core::Context const& context = {}) const;

    /**
     * @brief Queries a single entity in a table.
     *
     * @param partitionKey The partition key of the entity.
     * @param rowKey The row key of the entity.
     * @param context for canceling long running operations.
     * @return Table entity.
     */
    Response<Models::TableEntity> GetEntity(
        std::string const& partitionKey,
        std::string const& rowKey,
        Core::Context const& context = {}) const;

    /**
     * @brief Submits a transaction.
     *
     * @param steps The transaction steps to execute.
     * @param context for canceling long running operations.
     * @return Submit transaction result.
     */
    Response<Models::SubmitTransactionResult> SubmitTransaction(
        std::vector<Models::TransactionStep> const& steps,
        Core::Context const& context = {}) const;

  private:
#ifdef _azure_TABLES_TESTING_BUILD
    friend class Azure::Data::Tables::StressTest::TransactionStressTest;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionCreate_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyInsertMergeOp_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyInsertReplaceOp_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyDeleteOp_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyUpdateMergeOp_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyUpdateReplaceOp_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyAddOp_Test;
#endif

    Response<Models::UpdateEntityResult> UpdateEntityImpl(
        Models::TableEntity const& tableEntity,
        bool isUpsert,
        Core::Context const& context = {}) const;

    Response<Models::MergeEntityResult> MergeEntityImpl(
        Models::TableEntity const& tableEntity,
        bool isUpsert,
        Core::Context const& context = {}) const;
    std::string PreparePayload(
        std::string const& batchId,
        std::string const& changesetId,
        std::vector<Models::TransactionStep> const& steps) const;
    std::string PrepAddEntity(std::string const& changesetId, Models::TableEntity entity) const;
    std::string PrepDeleteEntity(std::string const& changesetId, Models::TableEntity entity) const;
    std::string PrepMergeEntity(std::string const& changesetId, Models::TableEntity entity) const;
    std::string PrepUpdateEntity(std::string const& changesetId, Models::TableEntity entity) const;
    std::string PrepInsertEntity(std::string const& changesetId, Models::TableEntity entity) const;
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    Core::Url m_url;
    std::string m_tableName;
  };
}}} // namespace Azure::Data::Tables
