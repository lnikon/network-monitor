#ifndef MOCK_RESOLVER_H
#define MOCK_RESOLVER_H

#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/utility/string_view.hpp>

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
        [](auto&& handler, auto resolver) {
            if (MockResolver::resolve_ec)
            {
                boost::asio::post(resolver->m_context,
                                  boost::beast::bind_handler(std::move(handler),
                                                             MockResolver::resolve_ec,
                                                             resolver::results_type{}));
            }
            else
            {
                // TODO: The usual handling of successfull resolve should go here.
            }
        },
        handler,
        this);
}

using MockWebSocketClient = WebSocketClient<
    MockResolver,
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>>;

} // namespace NetworkMonitor

#endif // MOCK_RESOLVER_H
