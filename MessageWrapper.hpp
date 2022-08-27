#pragma once
#include "socket.pb.h"

class MessageWrapper
{
public:
	using SocketMessage = pb::socket::ExternalMessage;
	
	auto setCmdCode(int32_t cmdcode) {
		return message_.set_cmdcode(cmdcode);
	}
	auto cmdCode() const {
		return message_.cmdcode();
	}

	auto setProtocolSwitch(int32_t protocolswitch) {
		return message_.set_protocolswitch(protocolswitch);
	}
	auto protocolSwitch() const {
		return message_.protocolswitch();
	}

	auto setCmdMerge(int32_t cmdmerge) {
		return message_.set_cmdmerge(cmdmerge);
	}
	auto cmdMerge() const {
		return message_.cmdmerge();
	}

	auto setResponseStatus(int32_t responsestatus) {
		return message_.set_responsestatus(responsestatus);
	}
	auto reponseStatus() const {
		return message_.responsestatus();
	}
	
	auto setValidMsg(std::string validmsg) {
		return message_.set_validmsg(validmsg);
	}
	auto validMsg() const {
		return message_.validmsg();
	}

	auto setData(std::string data) {
		return message_.set_data(data);
	}
	auto data() const {
		return message_.data();
	}

	auto setData(const google::protobuf::Message& data) {
		return message_.set_data(data.SerializeAsString());
	}
	
	template<typename T>
	typename std::enable_if<std::is_base_of<google::protobuf::Message, T>::value, T>::type object() const
	{
		T t;
		t.ParseFromString(data());
		return t;
	}

	auto parseToObject(google::protobuf::Message& msg) const
	{
		return msg.ParseFromString(data());
	}
	
	auto serializeAsString() const
	{
		return message_.SerializeAsString();
	}

	auto parseFromString(std::string str)
	{
		return message_.ParseFromString(str);
	}

	SocketMessage& message() {
		return message_;
	}
	
	const SocketMessage& message() const{
		return message_;
	}

protected:
	SocketMessage message_{};
};

