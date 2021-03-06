#ifndef MOCK_RESOLVER_H
#define MOCK_RESOLVER_H

#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/system/system_error.hpp>

namespace NetworkMonitor
{

class MockResolver
{
public:
    static boost::system::error_code resolve_ec;
    template <typename ExecutionContext> MockResolver(ExecutionContext&& context);

    template <typename ResolveHandler>
    auto async_resolve(boost::string_view host,
                       boost::string_view service,
                       ResolveHandler&& handler = ResolveHandler{});

private:
    boost::asio::strand<boost::asio::io_context::executor_type> m_context;
};

inline boost::system::error_code MockResolver::resolve_ec{};

template <typename ExecutionContext>
MockResolver::MockResolver(ExecutionContext&& context)
    : m_context(context)
{
}

template <typename ResolveHandler>
auto MockResolver::async_resolve(boost::string_view host,
                                 boost::string_view service,
                                 ResolveHandler&& handler)
{
    using resolver = boost::asio::ip::tcp::resolver;
    return boost::asio::async_initiate<ResolveHandler,
                                       void(const boost::system::error_code&,
                                            resolver::results_type)>(
        [](auto&& handler, auto resolver, auto host, auto service) {
            if (MockResolver::resolve_ec)
            {
                boost::asio::post(resolver->m_context,
                                  boost::beast::bind_handler(std::move(handler),
                                                             MockResolver::resolve_ec,
                                                             resolver::results_type{}));
            }
            else
            {
                boost::asio::post(resolver->m_context,
                                  boost::beast::bind_handler(
                                      std::move(handler),
                                      MockResolver::resolve_ec,
                                      resolver::results_type::create(
                                          boost::asio::ip::tcp::endpoint{
                                              boost::asio::ip::make_address("127.0.0.1"), 443},
                                          host,
                                          service)));
            }
        },
        handler,
        this,
		host.to_string(),
		service.to_string());
}

/*! \brief Mock the TCP socket stream from Boost.Beast.
 *
 *  We do not mock all available methods- only the ones we are interested in
 *  for testing.
 */
class MockTcpStream : public boost::beast::tcp_stream
{
public:
    /*! \brief Inherit all constructors from the parent class.
     */
    using boost::beast::tcp_stream::tcp_stream;

    /*! \brief Use this static member in a test to set the error code returned
     *  by async connect
     */
    static boost::system::error_code connect_ec;

    /*! \brief Mock for tcp_stream::async_connect
     */
    template <typename ConnectHandler>
    auto async_connect(const endpoint_type& endpoint, ConnectHandler&& handler);
};

// Out-of-line static member initialization.
inline boost::system::error_code MockTcpStream::connect_ec{};

template <typename ConnectHandler>
auto MockTcpStream::async_connect(
    const boost::asio::ip::tcp::resolver::results_type::endpoint_type& endpoint,
    ConnectHandler&& handler)
{
    return boost::asio::async_initiate<ConnectHandler, void(boost::system::error_code)>(
        [](auto&& handler, auto stream) {
            boost::asio::post(
                stream->get_executor(),
                boost::beast::bind_handler(std::move(handler), MockTcpStream::connect_ec));
        },
        handler,
        this);
}

// This overload is required by Boost.Beast when you define a custom stream.
template <typename TeardownHandler>
void async_teardown(boost::beast::role_type role, MockTcpStream& socket, TeardownHandler&& handler)
{
    return;
}

using MockWebSocketClient =
    WebSocketClient<MockResolver,
                    boost::beast::websocket::stream<boost::beast::ssl_stream<MockTcpStream>>>;

} // namespace NetworkMonitor

#endif // MOCK_RESOLVER_H
