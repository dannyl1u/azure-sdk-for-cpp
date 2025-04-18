// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#if ENABLE_UAMQP
#undef USE_NATIVE_BROKER
#elif ENABLE_RUST_AMQP
#define USE_NATIVE_BROKER
#endif

#include "azure/core/amqp/internal/common/async_operation_queue.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/connection.hpp"

#if defined(USE_NATIVE_BROKER)
#include "azure/core/amqp/internal/message_receiver.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/internal/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/internal/network/socket_listener.hpp"
#include "azure/core/amqp/internal/network/socket_transport.hpp"
#include "azure/core/amqp/internal/session.hpp"
#endif

#include <azure/core/context.hpp>
#include <azure/core/platform.hpp>
#include <azure/core/url.hpp>

#include <functional>
#include <random>

#include <gtest/gtest.h>
#if !defined(USE_NATIVE_BROKER)
#if defined(AZ_PLATFORM_POSIX)
#include <poll.h> // for poll()

#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket shutdown
#elif defined(AZ_PLATFORM_WINDOWS)
#include <winsock2.h> // for WSAPoll();
#ifdef max
#undef max
#endif
#endif // AZ_PLATFORM_POSIX/AZ_PLATFORM_WINDOWS
#endif // USE_NATIVE_BROKER

#if !defined(USE_NATIVE_BROKER)
#include "mock_amqp_server.hpp"
#else // USE_NATIVE_BROKER
#include <azure/core/internal/environment.hpp>
#endif

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  class TestSessions : public testing::Test {
  protected:
    void SetUp() override
    {
#if defined(USE_NATIVE_BROKER)
      auto testBrokerUrl = Azure::Core::_internal::Environment::GetVariable("TEST_BROKER_ADDRESS");
      if (testBrokerUrl.empty())
      {
        GTEST_FATAL_FAILURE_("Could not find required environment variable TEST_BROKER_ADDRESS");
      }
      Azure::Core::Url brokerUrl(testBrokerUrl);
      m_brokerEndpoint = brokerUrl;
#else
      m_brokerEndpoint
          = Azure::Core::Url("amqp://localhost:" + std::to_string(m_mockServer.GetPort()));
#endif
    }
    void TearDown() override
    { // When the test is torn down, the global state MUST be idle. If it is not,
      // something leaked.
      Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    std::string GetBrokerEndpoint() { return m_brokerEndpoint.GetAbsoluteUrl(); }

    std::uint16_t GetPort() { return m_brokerEndpoint.GetPort(); }

    void StartServerListening()
    {
#if !defined(USE_NATIVE_BROKER)
      m_mockServer.StartListening();
#endif
    }

    void StopServerListening()
    {
#if !defined(USE_NATIVE_BROKER)
      m_mockServer.StopListening();
#endif
    }

    auto CreateAmqpConnection(
        std::string const& containerId
        = testing::UnitTest::GetInstance()->current_test_info()->name(),
        bool enableTracing = false,
        Azure::Core::Context const& context = {})
    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.ContainerId = containerId;
      options.EnableTrace = enableTracing;
      options.Port = GetPort();

      auto connection = Azure::Core::Amqp::_internal::Connection("localhost", nullptr, options);
#if ENABLE_RUST_AMQP
      connection.Open(context);
#endif
      return connection;
      (void)context;
    }

    void CloseAmqpConnection(
        Azure::Core::Amqp::_internal::Connection& connection,
        Azure::Core::Context const& context = {})
    {
#if ENABLE_RUST_AMQP
      connection.Close(context);
#endif
      (void)connection;
      (void)context;
    }

  protected:
#if !defined(USE_NATIVE_BROKER)
    MessageTests::AmqpServerMock m_mockServer;
#endif

  private:
    Azure::Core::Url m_brokerEndpoint{};
  };

  using namespace Azure::Core::Amqp::_internal;
  using namespace Azure::Core::Amqp;

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestSessions, SimpleSession)
  {

    // Create a connection
    auto connection{CreateAmqpConnection()};

    {
      // Create a session
      Session session{connection.CreateSession()};
    }

    {
      // Create two sessions
      Session session1{connection.CreateSession({})};
      Session session2{connection.CreateSession({})};

      EXPECT_ANY_THROW(session1.End({}));
    }

    CloseAmqpConnection(connection);
  }

  TEST_F(TestSessions, SessionProperties)
  { // Create a connection
    auto connection{CreateAmqpConnection()};
    {
      Session session{connection.CreateSession()};

      // Verify defaults are something "reasonable".
      EXPECT_EQ(1, session.GetIncomingWindow());
      EXPECT_EQ((std::numeric_limits<uint32_t>::max)(), session.GetHandleMax());
      EXPECT_EQ(1, session.GetOutgoingWindow());
    }

    {
      SessionOptions options;
      options.MaximumLinkCount = 37;
      Session session{connection.CreateSession(options)};
      EXPECT_EQ(37, session.GetHandleMax());
    }
    {
      SessionOptions options;
      options.InitialIncomingWindowSize = 1909119;
      Session session{connection.CreateSession(options)};
      EXPECT_EQ(1909119, session.GetIncomingWindow());
    }
    {
      SessionOptions options;
      options.InitialOutgoingWindowSize = 1909119;
      Session session{connection.CreateSession(options)};
      EXPECT_EQ(1909119, session.GetOutgoingWindow());
    }
    CloseAmqpConnection(connection);
  }
#endif // !AZ_PLATFORM_MAC

#if !defined(USE_NATIVE_BROKER)

  uint16_t FindAvailableSocket()
  {
    // Ensure that the global state for the AMQP stack is initialized. Normally this is done by
    // the network facing objects, but this is called before those objects are initialized.
    //
    // This may hide bugs in some of the global objects, but it is needed to ensure that the
    // port we choose for the tests is available.
    {
      auto instance = Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance();
      (void)instance;
    }

    std::random_device dev;
    int count = 0;
    while (count < 20)
    {
      uint16_t testPort;
      // Make absolutely sure that we don't accidentally use the TLS port.
      do
      {
        testPort = dev() % 1000 + 0xBFFF;
      } while (testPort == AmqpTlsPort);

      GTEST_LOG_(INFO) << "Trying Test port: " << testPort;

      auto sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (sock != -1)
      {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(testPort);

        auto bindResult = bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        // We were able to bind to the port, so it's available.
#if defined(AZ_PLATFORM_WINDOWS)
        closesocket(sock);
#else
        close(sock);
#endif
        if (bindResult != -1)
        {
          return testPort;
        }
        else
        {
#if defined(AZ_PLATFORM_WINDOWS)
          auto err = WSAGetLastError();
#else
          auto err = errno;
#endif
          GTEST_LOG_(INFO) << "Error " << std::to_string(err) << " binding to socket.";
        }
      }
      else
      {
#if defined(AZ_PLATFORM_WINDOWS)
        auto err = WSAGetLastError();
#else
        auto err = errno;
#endif
        GTEST_LOG_(INFO) << "Error " << std::to_string(err) << " opening port.";
      }
      count += 1;
    }
    throw std::runtime_error("Could not find available test port.");
  }
#endif

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestSessions, SessionBeginEnd)
  {

    StartServerListening();

    // Create a connection
    auto connection{CreateAmqpConnection()};

    {
      Session session{connection.CreateSession()};

      session.Begin({});
      session.End({});
    }

    {
      Session session{connection.CreateSession()};

      session.Begin({});
      session.End("", "", {});
    }

    {
      Session session{connection.CreateSession()};

      session.Begin({});
      session.End("amqp:link:detach-forced", "Forced detach.", {});
    }
    StopServerListening();

    CloseAmqpConnection(connection);
  }

  TEST_F(TestSessions, MultipleSessionBeginEnd)
  {
#if ENABLE_UAMQP
    m_mockServer.EnableTrace(false);
    StartServerListening();

    // Create a connection
    Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
    connectionOptions.Port = GetPort();
    connectionOptions.EnableTrace = true;

    class OutgoingConnectionEvents : public ConnectionEvents {
      /** @brief Called when the connection state changes.
       *
       * @param newState The new state of the connection.
       * @param oldState The previous state of the connection.
       */
      void OnConnectionStateChanged(
          Connection const&,
          ConnectionState newState,
          ConnectionState oldState) override
      {
        GTEST_LOG_(INFO) << "Connection state changed. OldState: " << oldState << " -> "
                         << newState;
      };

      /** @brief called when an I/O error has occurred on the connection.
       *
       */
      void OnIOError(Connection const&) override { GTEST_LOG_(INFO) << "Connection IO Error."; };
    };

    OutgoingConnectionEvents connectionEvents;
    Azure::Core::Amqp::_internal::Connection connection(
        "localhost", nullptr, connectionOptions, &connectionEvents);
#else
    auto connection{CreateAmqpConnection()};
#endif

    {
      constexpr const size_t sessionCount = 30;
      GTEST_LOG_(INFO) << "Opening " << sessionCount << " sessions.";
      std::vector<Session> sessions;
      for (size_t i = 0; i < sessionCount; i += 1)
      {
        sessions.push_back(connection.CreateSession());
        sessions.back().Begin({});
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      GTEST_LOG_(INFO) << "Closing " << sessionCount << " sessions.";
      for (auto& session : sessions)
      {
        session.End({});
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }
    CloseAmqpConnection(connection);
#if ENABLE_UAMQP
    StopServerListening();
#endif
  }
#endif // !AZ_PLATFORM_MAC
}}}} // namespace Azure::Core::Amqp::Tests
