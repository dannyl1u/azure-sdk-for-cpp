// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Keyvault Deleted Secret definition
 */

#pragma once
#include <azure/core/datetime.hpp>
#include <azure/keyvault/secrets/keyvault_secret.hpp>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  class SecretClient;
  class DeletedSecretPagedResponse;
  namespace _detail { namespace Models {
    struct DeletedSecretBundle;
    struct DeletedSecretItem;
  }} // namespace _detail::Models
  /**
   * @brief A Deleted Secret consisting of its previous id, attributes and its tags,
   * as well as information on when it will be purged.
   */
  struct DeletedSecret : public KeyVaultSecret
  {
    /**
     * @brief A Deleted Secret consisting of its previous id, attributes and its tags,
     * as well as information on when it will be purged.
     */
    std::string RecoveryId;

    /**
     * @brief The time when the secret is scheduled to be purged, in UTC.
     */
    Azure::Nullable<Azure::DateTime> ScheduledPurgeDate;

    /**
     * @brief The time when the secret was deleted, in UTC.
     */
    Azure::Nullable<Azure::DateTime> DeletedOn;

    /**
     * @brief Default constructor.
     */
    DeletedSecret() = default;

    /**
     * @brief Constructor.
     *
     * @param name Name of the deleted secret.
     */
    DeletedSecret(std::string name) : KeyVaultSecret(std::move(name)) {}

  private:
    DeletedSecret(_detail::Models::DeletedSecretBundle const& deletedSecret);
    DeletedSecret(_detail::Models::DeletedSecretItem const& deletedSecret);
    friend class SecretClient;
    friend class DeletedSecretOperation;
    friend class DeletedSecretPagedResponse;
  };
}}}} // namespace Azure::Security::KeyVault::Secrets
