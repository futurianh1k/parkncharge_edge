// File:	ServerNetworkHandler.cpp
// Author:	Seongdo Kim
// Contact:	sdland85@gmail.com
//
// Copyright (c) 2017, Seongdo Kim <sdland85@gmail.com>
// All rights reserved.
// The copyright to the computer program(s) herein is
// the property of Seongdo Kim. The program(s) may be
// used and/or copied only with the written permission
// of Seongdo Kim or in accordance with the terms and
// conditions stipulated in the agreement/contract under
// which the program(s) have been supplied.
//
// Written by Seongdo Kim <sdland85@gmail.com>, June, 2017

#include "ServerNetworkHandler.h"
#include "SecureClient.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/chrono.hpp>
#include <boost/asio.hpp>

#include <glog/logging.h>

using namespace seevider;

ServerNetworkHandler::ServerNetworkHandler(std::shared_ptr<MessageQueue> &messageQueue,
	const std::shared_ptr<SensorInfo> &sensorInfo, std::string serverDataFilename) :
	mSettingsFilename(SYSTEM_FOLDER_CORE + serverDataFilename),
	mSensorInfo(sensorInfo) {
	assert(messageQueue != nullptr);
	mMessageQueue = messageQueue;

	loadSettings(mSettingsFilename);

	// Start the handling thread
	boost::thread t(boost::bind(&ServerNetworkHandler::run, this));
	mHandlerThread.swap(t);
}

ServerNetworkHandler::~ServerNetworkHandler() {
    if (mHandlerThread.joinable()) {
        // If the thread is still joinable, destroy it.
        destroy();
	}

	writeSettings();
}

void ServerNetworkHandler::destroy() {
	LOG(INFO) << "Destroying the HTTP message uploading handler";

    mOperation = false;
    mHandlerThread.try_join_for(boost::chrono::seconds(mWaitSeconds));
    if (mHandlerThread.joinable()) {
        // If the thread is still joinable, force to cancel it.
        mHandlerThread.interrupt();
        mHandlerThread.try_join_for(boost::chrono::seconds(mDestroySeconds));
	}

	LOG(INFO) << "The HTTP message uploading handler has destroyed";
}

void ServerNetworkHandler::run() {
	LOG(INFO) << "Starting the HTTP message uploading handler";

    while (mOperation) {
		std::unique_ptr<IMessageData> data;
		mMessageQueue->wait_and_pop(data);

        // upload data to destination server in designated format
		while (!upload(data)) {
			// if upload is failed, wait for few seconds and try it again.
			LOG(WARNING) << "Upload failed. Try it again in 3 seconds.. " << std::endl;
			boost::this_thread::sleep_for(boost::chrono::seconds(3));

			// TODO: if the uploading failed for specific times, ignore this message
		}
	}

	LOG(INFO) << "The HTTP message uploading handler has initialized";
}

bool ServerNetworkHandler::upload(const std::unique_ptr<IMessageData> &data) const {
	DLOG(INFO) << data->toString();

	boost::property_tree::ptree root = data->toPTree();

	std::stringstream ss;
	int request = root.get<int>("httpRequest", -1);
	std::string targetAddr;
	ServerDestinations_t dest;

	root.erase("httpRequest");

	if (request < 0) {
		LOG(ERROR) << "Current data has negative request type: " << request;
		// TODO: Handle the error (non-recoverable error)
		return false;
	}

	if (mServerDestinations.find(request) == mServerDestinations.end()) {
		LOG(ERROR) << "Current data has wrong request type: " << request;
		// TODO: Handle the error (non-recoverable error)
		return false;
	}
	else {
		dest = mServerDestinations.find(request)->second;
	}

	// Add common properties
	root.put<std::string>("timeZone", mSensorInfo->TimeZone);
	write_json(ss, root);

<<<<<<< HEAD
	//std::ofstream fout("sync_upload.txt");
	//fout << ss.str() << std::endl;
	//fout.close();

=======
	std::ofstream fout("sync_upload.txt");
	fout << ss.str() << std::endl;
	fout.close();

/*
>>>>>>> 3441677ab1565ffd6fa168e9812f543a820cd17e
	if (mSecureConnection.compare("none") == 0) {
		return sendHTTP(dest.HTTPRequestMethod, ss.str(), dest.TargetPath);
	}
	else {
		return sendSecureHTTP(dest.HTTPRequestMethod, ss.str(), dest.TargetPath);
	}
<<<<<<< HEAD
=======
*/
	return sendHTTP(dest.HTTPRequestMethod, ss.str(), dest.TargetPath);
>>>>>>> 3441677ab1565ffd6fa168e9812f543a820cd17e
}

bool ServerNetworkHandler::sendHTTP(const std::string method,
	const std::string jsonstring, const std::string targetAddr) const {
	using boost::asio::ip::tcp;

	try {
		boost::asio::io_service io_service;

		// Get a list of endpoints corresponding to the server name.
		tcp::resolver resolver(io_service);
<<<<<<< HEAD
		tcp::resolver::query query(mServerAddr, "http");
=======
		tcp::resolver::query query(mServerAddr, "20180");
>>>>>>> 3441677ab1565ffd6fa168e9812f543a820cd17e
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::resolver::iterator end;

		// Try each endpoint until we successfully establish a connection.
		tcp::socket socket(io_service);
		boost::system::error_code error = boost::asio::error::host_not_found;
		while (error && endpoint_iterator != end) {
			socket.close();
			socket.connect(*endpoint_iterator++, error);
		}

		if (error) {
			throw boost::system::system_error(error);
		}

		// Form the request. We specify the "Connection: close" header so that the
		// server will close the socket after transmitting the response. This will
		// allow us to treat all data up until the EOF as the content.
		boost::asio::streambuf request;
		std::ostream request_stream(&request);

		request_stream << method << " " << targetAddr << " HTTP/1.1\r\n";
		request_stream << "Host: " << mServerAddr << "\r\n";
		request_stream << "User-Agent: C/1.0\r\n";
		request_stream << "Content-Type: application/json; charset=utf-8 \r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Content-Length: " << jsonstring.length() << "\r\n";
		request_stream << "sensorid: " << mSensorInfo->SensorID << "\r\n";
		request_stream << "Connection: close\r\n\r\n";  //NOTE THE Double line feed
		request_stream << jsonstring;
		
		//std::string s((std::istreambuf_iterator<char>(&request)), std::istreambuf_iterator<char>());
		//std::cout << s << std::endl << std::endl;

		// Send the request.
		boost::asio::write(socket, request);

		// Read the response status line. The response streambuf will automatically
		// grow to accommodate the entire line. The growth may be limited by passing
		// a maximum size to the streambuf constructor.
		boost::asio::streambuf response;
		boost::asio::read_until(socket, response, "\r\n");

		// Check that response is OK.
		std::istream response_stream(&response);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
			std::cout << "Invalid response\n";
			return false;
		}
		if (status_code != 200) {
			std::cout << "Response returned with status code " << status_code << "\n";
			std::cout << status_message << std::endl;
			return false;
		}

		// Read the response headers, which are terminated by a blank line.
		boost::asio::read_until(socket, response, "\r\n\r\n");

		// Process the response headers.
		std::string header;
		while (std::getline(response_stream, header) && header != "\r")
			std::cout << header << "\n";
		std::cout << "\n";

		// Write whatever content we already have to output.
		if (response.size() > 0)
			std::cout << &response;

		// Read until EOF, writing data to output as we go.
		while (boost::asio::read(socket, response,
			boost::asio::transfer_at_least(1), error))
			std::cout << &response;
		if (error != boost::asio::error::eof)
			throw boost::system::system_error(error);
	}
	catch (std::exception& e) {
		LOG(ERROR) << e.what();
		return false;
	}

	return true;
}

bool ServerNetworkHandler::sendSecureHTTP(const std::string method,
	const std::string jsonstring, const std::string targetAddr) const {
	using boost::asio::ip::tcp;

	try {
		boost::asio::io_service io_service;

		// Get a list of endpoints corresponding to the server name.
		tcp::resolver resolver(io_service);
<<<<<<< HEAD
		tcp::resolver::query query(mServerAddr, "https");	// or default secure port 443
=======
		tcp::resolver::query query(mServerAddr, "20180");	// or default secure port 443
>>>>>>> 3441677ab1565ffd6fa168e9812f543a820cd17e
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv11);
		ctx.set_default_verify_paths();

		SecureClient c(io_service, ctx, endpoint_iterator,
			makeHTTPMessage(method, targetAddr, jsonstring));

		io_service.run();
	}
	catch (std::exception& e) {
		LOG(ERROR) << e.what();
		return false;
	}

	return true;
}

std::string ServerNetworkHandler::makeHTTPMessage(const std::string &method,
	const std::string &targetAddr, const std::string &contents) const {
	std::stringstream request_stream;

	request_stream << method << " " << targetAddr << " HTTP/1.1\r\n";
	request_stream << "Host: " << mServerAddr << "\r\n";
	request_stream << "User-Agent: C/1.0\r\n";
	request_stream << "Content-Type: application/json; charset=utf-8 \r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Content-Length: " << contents.length() << "\r\n";
	request_stream << "sensorid: " << mSensorInfo->SensorID << "\r\n";
	request_stream << "Connection: close\r\n\r\n";  //NOTE THE Double line feed
	request_stream << contents;

<<<<<<< HEAD
=======
	//LOG(FATAL) << "Request stream" << mServerAddr;
	//LOG(FATAL) << "Request stream" << targetAddr;

>>>>>>> 3441677ab1565ffd6fa168e9812f543a820cd17e
	return request_stream.str();
}

bool ServerNetworkHandler::loadSettings(const std::string filename) {
	if (filename.empty()) {
		LOG(FATAL) << "Server data filename is empty.";
		return false;
	}

	using boost::property_tree::ptree;

	ptree root;
	boost::property_tree::read_xml(filename, root);

	for (ptree::value_type const& v : root.get_child("ServerSettings")) {
		if (v.first == "ServerAddress") {
			mServerAddr = v.second.get_value<std::string>();
		}
		else if (v.first == "SecureConnection") {
			mSecureConnection = v.second.get_value<std::string>();
		}
		else if (v.first == "Request") {
			ServerDestinations_t dest;

			dest.RequestType = v.second.get<int>("RequestType");
			dest.HTTPRequestMethod = v.second.get<std::string>("HTTPRequestMethod");
			dest.TargetPath = v.second.get<std::string>("TargetPath");
			mServerDestinations[dest.RequestType] = dest;
		}
	}

<<<<<<< HEAD
=======
	//LOG(FATAL) << "Request stream" << mServerAddr;


>>>>>>> 3441677ab1565ffd6fa168e9812f543a820cd17e
	return true;
}

bool ServerNetworkHandler::writeSettings() {
	if (mSettingsFilename.empty()) {
		LOG(FATAL) << "Server data filename is empty.";
		return false;
	}

	using boost::property_tree::ptree;

	ptree root;

	root.add("ServerSettings.ServerAddress", mServerAddr);
	root.add("ServerSettings.SecureConnection", mSecureConnection);
	for (const auto& data : mServerDestinations) {
		ptree &node = root.add("ServerSettings.Request", "");

		node.put("RequestType", data.second.RequestType);
		node.put("HTTPRequestMethod", data.second.HTTPRequestMethod);
		node.put("TargetPath", data.second.TargetPath);
	}

#if BOOST_VERSION_MINOR > 55
	write_xml(mSettingsFilename, root, std::locale(), boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
#else
	write_xml(mSettingsFilename, root, std::locale(), boost::property_tree::xml_writer_make_settings<char>('\t', 1));
#endif

	return true;
}
