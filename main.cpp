#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/common/thread.hpp>

#include <spdlog/spdlog.h>

#include "MessageWrapper.hpp"


int main()
{
	// 配置log等级
#ifdef _DEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	// 读取路由表
	CommandRouterMap::instance().load("socket.router.json");

	// 注册工厂
	ohtoai::ProductRegistrar<google::protobuf::Message
		, pb::socket::UserInfo>
		_userInfoRegistrar(pb::socket::UserInfo{}.GetTypeName());
	ohtoai::ProductRegistrar<google::protobuf::Message
		, pb::socket::UserLogin>
		_userLoginRegistrar(pb::socket::UserLogin{}.GetTypeName());
	ohtoai::ProductRegistrar<google::protobuf::Message
		, pb::socket::ExternalMessage>
		_externalMessageRegistrar(pb::socket::ExternalMessage{}.GetTypeName());


	// Check protobuf
	{
		spdlog::info("Check protobuf");
		using pb::socket::UserInfo;
		UserInfo userInfo;
		userInfo.set_name("zhang3");
		userInfo.set_id(10086);

		MessageWrapper wrapper;
		wrapper.setCmdMerge(1);
		wrapper.setCmdCode(2);
		wrapper.setData(userInfo);
		auto serial = wrapper.serializeAsString();

		MessageWrapper wrapper_receive;
		wrapper_receive.parseFromString(serial);

		auto info_receive = wrapper_receive.object<UserInfo>();
		UserInfo info_receive2;
		wrapper_receive.toObject(info_receive2);

		spdlog::info("info_receive name = {}, id = {}", info_receive.name(), info_receive.id());
		spdlog::info("info_receive2 name = {}, id = {}", info_receive2.name(), info_receive2.id());

		assert(userInfo.name() == info_receive.name()
			&& userInfo.id() == info_receive.id());
		assert(userInfo.name() == info_receive2.name()
			&& userInfo.id() == info_receive2.id());

		spdlog::info("protobuf test PASS");
	}

	// Check websocket
	{
		spdlog::info("Check websocket");

		using WebsocketClient = websocketpp::client<websocketpp::config::asio_client>;
		using pb::socket::UserInfo;

		WebsocketClient client;


		// Set logging to be pretty verbose (everything except message payloads)
		client.set_access_channels(websocketpp::log::alevel::all);
		client.clear_access_channels(websocketpp::log::alevel::frame_payload);

		// Initialize ASIO
		client.init_asio();

		// Register our message handler
		client.set_message_handler([&](websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg) {

			spdlog::info("Message Handler = {}", hdl.lock().get());
			spdlog::info("payload[{} bytes] = {}", msg->get_payload().size(), msg->get_payload());

			auto userInfo = MessageWrapper::FromString(msg->get_payload()).object<UserInfo>();

			spdlog::info("user.name = {}, user.id = {}, user.jwt = {}", userInfo.name(), userInfo.id(), userInfo.jwt());
			});


		websocketpp::client<websocketpp::config::asio_client>::connection_ptr connection;
		client.set_open_handler([&](websocketpp::lib::weak_ptr<void> d) {

			MessageWrapper wrapper;

			pb::socket::UserLogin userLogin;
			userLogin.set_username("user");
			userLogin.set_password("123");

			wrapper.setRouter(userLogin);
			wrapper.setData(userLogin);

			client.send(connection->get_handle(), wrapper.serializeAsString(), websocketpp::frame::opcode::binary);
			});
		websocketpp::lib::error_code ec;
		connection = client.get_connection("ws://game.ohtoai.top/websocket", ec);

		if (ec) {
			spdlog::error("Could not create connection because: {}", ec.message());
			return 0;
		}
		// Note that connect here only requests a connection. No network messages are
		// exchanged until the event loop starts running in the next line.
		client.connect(connection);

		std::thread([&] {
			try {
				client.run();
			}
			catch (websocketpp::exception const& e) {
				spdlog::error("Error occur: {}", e.what());
			}}).join();
	}
	return 0;
}
