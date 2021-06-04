#include "../main/rossa/rossa.h"
#include "../main/rossa/symbol/symbol.h"
#include "../main/rossa/rossa_error/rossa_error.h"
#include "../main/rossa/function/function.h"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include "encode.h"

ROSSA_EXT_SIG(_service_init, args, token, hash, stack_trace)
{
	auto service = std::make_shared<boost::asio::io_service>();
	return symbol_t::Pointer(service);
}

ROSSA_EXT_SIG(_socket_init, args, token, hash, stack_trace)
{
	auto io_service_object = COERCE_PTR(
		args[2].getPointer(token, stack_trace),
		boost::asio::io_service);

	auto sock = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_object);
	boost::system::error_code ec;
	if (sock->connect(
		boost::asio::ip::tcp::endpoint(
			boost::asio::ip::address::from_string(args[0].getString(token, stack_trace)),
			args[1].getNumber(token, stack_trace).getLong()),
		ec))
		if (ec)
			throw rossa_error_t(ec.message(), *token, stack_trace);
	return symbol_t::Pointer(sock);
}

ROSSA_EXT_SIG(_socket_send, args, token, hash, stack_trace)
{
	auto sock = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		boost::asio::ip::tcp::socket);

	std::string content = args[1].getString(token, stack_trace);
	boost::asio::write(*sock, boost::asio::buffer(content));
	return symbol_t();
}

ROSSA_EXT_SIG(_socket_read, args, token, hash, stack_trace)
{
	auto sock = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		boost::asio::ip::tcp::socket);

	boost::asio::streambuf sb;
	boost::system::error_code ec;
	boost::asio::read(*sock, sb, ec);
	std::string str(boost::asio::buffers_begin(sb.data()), boost::asio::buffers_begin(sb.data()) + sb.data().size());
	return symbol_t::String(str);
}

ROSSA_EXT_SIG(_socket_read_until, args, token, hash, stack_trace)
{
	auto sock = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		boost::asio::ip::tcp::socket);

	boost::asio::streambuf sb;
	if (boost::asio::read_until(*sock, sb, args[1].getString(token, stack_trace))) {
		std::string str(boost::asio::buffers_begin(sb.data()), boost::asio::buffers_begin(sb.data()) + sb.data().size());
		return symbol_t::String(str);
	}
	return symbol_t();
}

ROSSA_EXT_SIG(_socket_close, args, token, hash, stack_trace)
{
	auto sock = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		boost::asio::ip::tcp::socket);

	boost::system::error_code ec;
	sock->close(ec);
	if (ec)
		return symbol_t::String(ec.message());
	return symbol_t();
}

ROSSA_EXT_SIG(_server_init, args, token, hash, stack_trace)
{
	auto io_service_object = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		boost::asio::io_service);

	auto acc = std::make_shared<boost::asio::ip::tcp::acceptor>(*io_service_object, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), args[0].getNumber(token, stack_trace).getLong()));
	return symbol_t::Pointer(acc);
}

ROSSA_EXT_SIG(_server_accept, args, token, hash, stack_trace)
{
	auto acc = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		boost::asio::ip::tcp::acceptor);

	auto io_service_object = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		boost::asio::io_service);

	auto sock = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_object);
	acc->accept(*sock);
	return symbol_t::Pointer(sock);
}

ROSSA_EXT_SIG(_tcp_stream_init, args, token, hash, stack_trace)
{
	const std::string address = args[0].getString(token, stack_trace);
	const std::string port = std::to_string(args[1].getNumber(token, stack_trace).getLong());

	auto io_service_object = COERCE_PTR(
		args[2].getPointer(token, stack_trace),
		boost::asio::io_service);

	boost::asio::ip::tcp::resolver resolver(*io_service_object);
	auto results = resolver.resolve(address, port);

	auto tcpstream = std::make_shared<boost::beast::tcp_stream>(*io_service_object);
	boost::system::error_code ec;
	tcpstream->connect(results);

	return symbol_t::Pointer(tcpstream);
}

ROSSA_EXT_SIG(_tcp_stream_request, args, token, hash, stack_trace)
{
	const std::string target = args[0].getString(token, stack_trace);
	const int version = args[1].getNumber(token, stack_trace).getLong();
	const auto params = args[2].getDictionary(token, stack_trace);

	auto tcpstream = COERCE_PTR(
		args[3].getPointer(token, stack_trace),
		boost::beast::tcp_stream);

	boost::beast::http::request<boost::beast::http::string_body> req(boost::beast::http::verb::get, target, version);

	for (auto &p : params) {
		req.set(p.first, p.second.getString(token, stack_trace));
	}

	boost::beast::http::write(*tcpstream, req);
	boost::beast::flat_buffer buff;
	boost::beast::http::response<boost::beast::http::string_body> res;
	boost::beast::http::read(*tcpstream, buff, res);

	std::map<const std::string, const symbol_t> ret;

	for (auto &r : res) {
		ret.insert({ r.name_string().to_string(), symbol_t::String(r.value().to_string()) });
	}

	ret.insert({ "CONTENT",symbol_t::String(res.body()) });

	return symbol_t::Dictionary(ret);
}

ROSSA_EXT_SIG(_tcp_stream_close, args, token, hash, stack_trace)
{
	auto tcpstream = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		boost::beast::tcp_stream);

	tcpstream->close();

	return symbol_t();
}

ROSSA_EXT_SIG(_encodeURI, args, token, hash, stack_trace)
{
	const std::string s = args[0].getString(token, stack_trace);
	return symbol_t::String(encodeURIComponent(s));
}

ROSSA_EXT_SIG(_decodeURI, args, token, hash, stack_trace)
{
	const std::string s = args[0].getString(token, stack_trace);
	return symbol_t::String(decodeURIComponent(s));
}

EXPORT_FUNCTIONS(libnet)
{
	ADD_EXT(_decodeURI);
	ADD_EXT(_encodeURI);
	ADD_EXT(_server_accept);
	ADD_EXT(_server_init);
	ADD_EXT(_service_init);
	ADD_EXT(_socket_close);
	ADD_EXT(_socket_init);
	ADD_EXT(_socket_read_until);
	ADD_EXT(_socket_read);
	ADD_EXT(_socket_send);
	ADD_EXT(_tcp_stream_close);
	ADD_EXT(_tcp_stream_init);
	ADD_EXT(_tcp_stream_request);
}