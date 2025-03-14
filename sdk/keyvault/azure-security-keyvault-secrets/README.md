# Azure Security Keyvault Secrets Package client library for C++

Azure Security Keyvault Secrets Package client library for C++ (`azure-security-keyvault-secrets`)  matches necessary patterns that the development team has established to create a unified SDK written in the C++ programming language. These libraries follow the Azure SDK Design Guidelines for C++.

The library allows client libraries to expose common functionality in a consistent fashion.  Once you learn how to use these APIs in one client library, you will know how to use them in other client libraries.

[Source code][secrets_client_src] | [Package (vcpkg)](https://vcpkg.io/en/package/azure-security-keyvault-secrets-cpp) | [API reference documentation][api_reference] | [Product documentation][keyvault_docs] | [Samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-secrets/samples)

## Getting started

### Prerequisites
- [vcpkg](https://learn.microsoft.com/vcpkg/get_started/overview) for package acquisition and dependency management
- [CMake](https://cmake.org/download/) for project build
- An [Azure subscription][azure_sub].
- An existing Azure Key Vault. If you need to create an Azure Key Vault, you can use the Azure Portal or [Azure CLI][azure_cli].

If you use the Azure CLI, replace `<your-resource-group-name>` and `<your-key-vault-name>` with your own, unique names:

```PowerShell
az login 
az keyvault create --resource-group <your-resource-group-name> --name <your-key-vault-name>
```

### Install the package
The easiest way to acquire the C++ SDK is leveraging the vcpkg package manager and CMake. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install]. We'll use vcpkg in manifest mode. To start a vcpkg project in manifest mode use the following command at the root of your project: 

```batch
vcpkg new --application
```

To install the Azure \<Service-Name> package via vcpkg:
To add the Azure \<Service-Name> package to your vcpkg enter the following command (We'll also add the Azure Identity library for authentication):

```batch
vcpkg add port azure-security-keyvault-secrets-cpp azure-identity-cpp
```

Then, add the following in your CMake file:

```CMake
find_package(azure-security-keyvault-secrets-cpp CONFIG REQUIRED)
find_package(azure-identity-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-security-keyvault-secrets Azure::azure-identity)
```

Remember to set `CMAKE_TOOLCHAIN_FILE` to the path to `vcpkg.cmake` either by adding the following to your `CMakeLists.txt` file before your project statement:

```CMake
set(CMAKE_TOOLCHAIN_FILE "vcpkg-root/scripts/buildsystems/vcpkg.cmake")
```

Or by specifying it in your CMake commands with the `-DCMAKE_TOOLCHAIN_FILE` argument.

There is more than one way to acquire and install this library. Check out [our samples on different ways to set up your Azure C++ project][project_set_up_examples].


## Key concepts

### KeyVaultSecret
A `Secret` is the fundamental resource within Azure Key Vault. From a developer's perspective, Azure Key Vault APIs accept and return secret values as strings.

### SecretClient
`SecretClient` provides synchronous operations exists in the SDK. Once you've initialized a `SecretClient`, you can interact with the primary resource types in Azure Key Vault.

### Thread safety
We guarantee that all client instance methods are thread-safe and independent of each other ([guideline](https://azure.github.io/azure-sdk/cpp_introduction.html#thread-safety)). This ensures that the recommendation of reusing client instances is always safe, even across threads.

### Additional concepts

<!-- CLIENT COMMON BAR -->
[Replaceable HTTP transport adapter](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core#http-transport-adapter) |
[Long-running operations](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core#long-running-operations) |
<!-- CLIENT COMMON BAR -->

## SecretClient

For detailed samples please review the samples provided.

### Create a client

First step is to create  a SecretClient.

```cpp
auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

// create client
SecretClient secretClient(keyVaultUrl, credential);
```

### Create a secret

We call the secret client to create a secret.

```cpp
std::string secretName("MySampleSecret");
std::string secretValue("my secret value");

secretClient.SetSecret(secretName, secretValue);
```

### Get a secret

We retrieve a secret by name.

```cpp
// get secret
KeyVaultSecret secret = secretClient.GetSecret(secretName).Value;

std::string valueString = secret.Value.HasValue() ? secret.Value.Value() : "NONE RETURNED";
std::cout << "Secret is returned with name " << secret.Name << " and value " << valueString
          << std::endl;
```

### Update a secret

Updating an existing secret

```cpp 
// change one of the properties
secret.Properties.ContentType = "my content";
// update the secret
KeyVaultSecret updatedSecret = secretClient.UpdateSecretProperties(secret.Properties).Value;
std::string updatedValueString = updatedSecret.Properties.ContentType.ValueOr("NONE RETURNED");
std::cout << "Secret's content type is now " << updatedValueString << std::endl;
```

### Delete a secret

Delete an existing secret.

```cpp
// start deleting the secret
DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);

// You only need to wait for completion if you want to purge or recover the secret.
// The duration of the delete operation might vary
// in case returns too fast increase the timeout value
operation.PollUntilDone(20s);

// purge the deleted secret
secretClient.PurgeDeletedSecret(secret.Name);
```

### Delete and purge a secret

Delete and Purge a secret.

```cpp
// start deleting the secret
DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);

// You only need to wait for completion if you want to purge or recover the secret.
// The duration of the delete operation might vary
// in case returns too fast increase the timeout value
operation.PollUntilDone(20s);

// purge the deleted secret
secretClient.PurgeDeletedSecret(secret.Name);
```

### List Secrets

List all the secrets in keyvault. 

```cpp
// get all the versions of a secret
for (auto secretsVersion = secretClient.GetPropertiesOfSecretsVersions(secret1.Name);
     secretsVersion.HasPage();
     secretsVersion.MoveToNextPage())
{ // go through each version of the secret
  // the number of results returned for in a page is not guaranteed
  // it can be anywhere from 0 to 25
  for (auto const& secret : secretsVersion.Items)
  {
    std::cout << "Found Secret with name: " << secret.Name
              << " and with version: " << secret.Version << std::endl;
  }
}
```

## Troubleshooting

When you interact with the Azure Key Vault Secrets client library using the C++ SDK, errors returned by the service correspond to the same HTTP status codes returned for requests.

For example, if you try to retrieve a key that doesn't exist in your Azure Key Vault, a `404` error is returned, indicating "Not Found".

```cpp
try
{
  Secret secret = client.GetSecret("some_secret").Value;
}
catch (const Azure::Core::RequestFailedException& ex)
{
  std::cout << std::underlying_type<Azure::Core::Http::HttpStatusCode>::type(ex.StatusCode);
}
```

You will notice that additional information is logged, like the client request ID of the operation.


# Next steps

Several Azure Key Vault secrets client library samples are available to you in this GitHub repository. These samples provide example code for additional scenarios commonly encountered while working with Azure Key Vault:

* [Sample1-Basic-Operations](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-secrets/samples/sample1_basic_operations.md):
  * Create a secret
  * Get a secret
  * Update a secret
  * Delete and Purge a secret

* [Sample2-Backup-Restore](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-secrets/samples/sample2_backup_restore.md):
  * Backup a secret
  * Restore a deleted secret

* [Sample3-Delete-Recover](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-secrets/samples/sample3_delete_recover.md):
  * Delete a secret
  * Recover a deleted Secret

* [Sample4-Get-Secrets-Deleted](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-secrets/samples/sample4_get_secrets_deleted.md):
  * List all secrets
  * List all of a secrets versions
  * List all deletes secrets
  * Get properties of a deleted secret

## Contributing
For details on contributing to this repository, see the [contributing guide][azure_sdk_for_cpp_contributing].

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit the [Contributor License Agreement](https://cla.microsoft.com).

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Additional Helpful Links for Contributors
Many people all over the world have helped make this project better.  You'll want to check out:

* [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
* [How to build and test your change][azure_sdk_for_cpp_contributing_developer_guide]
* [How you can make a change happen!][azure_sdk_for_cpp_contributing_pull_requests]
* Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

<!-- ### Community-->
### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt) license.

<!-- LINKS -->
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests
[azure_cli]: https://learn.microsoft.com/cli/azure
[azure_sub]: https://azure.microsoft.com/free/
[api_reference]: https://azure.github.io/azure-sdk-for-cpp/keyvault.html
[secrets_client_src]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-secrets
[keyvault_docs]: https://learn.microsoft.com/azure/key-vault/
[azsdk_vcpkg_install]: https://github.com/Azure/azure-sdk-for-cpp#getting-started
[project_set_up_examples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/samples/integration
