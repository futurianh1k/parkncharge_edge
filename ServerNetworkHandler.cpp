#include "ServerNetworkHandler.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/chrono.hpp>
#include <boost/asio.hpp>

using namespace seevider;

using std::cout;
using std::endl;

ServerNetworkHandler::ServerNetworkHandler(std::shared_ptr<MessageQueue> &messageQueue,
	const std::shared_ptr<Settings> &settings) :
	mSettingsFilename(SYSTEM_FOLDER_CORE + "/" + settings->ServerDataFilename),
	mHandlerThread(boost::bind(&ServerNetworkHandler::run, this)),
	mSensorInfo(std::dynamic_pointer_cast<SensorInfo, Settings>(settings)) {
	assert(messageQueue != nullptr);
	mMessageQueue = messageQueue;

	loadSettings(mSettingsFilename);
}

ServerNetworkHandler::~ServerNetworkHandler() {
    if (mHandlerThread.joinable()) {
        // If the thread is still joinable, destroy it.
        destroy();
	}

	writeSettings();
}

void ServerNetworkHandler::destroy() {
    std::cout << "Destroy network handler...";
    mOperation = false;
    mHandlerThread.try_join_for(boost::chrono::seconds(mWaitSeconds));
    if (mHandlerThread.joinable()) {
        // If the thread is still joinable, force to cancel it.
        mHandlerThread.interrupt();
        mHandlerThread.try_join_for(boost::chrono::seconds(mDestroySeconds));
    }
    std::cout << "done." << std::endl;
}

void ServerNetworkHandler::run() {
    std::cout << "Starts the network handler" << std::endl;
    while (mOperation) {
		std::unique_ptr<IMessageData> data;
		mMessageQueue->wait_and_pop(data);

        // upload data to destination server in designated format
		if (!upload(data)) {
			// if upload is failed, wait for few seconds and try it again.
			std::cout << "Upload failed. Try it again in 3 seconds.. " << std::endl;
			boost::this_thread::sleep_for(boost::chrono::seconds(3));
		}
    }
    std::cout << "Ends the network handler" << std::endl;
}

bool ServerNetworkHandler::upload(const std::unique_ptr<IMessageData> &data) const {
    cout << "Processing data: " << data->toString() << endl;

	boost::property_tree::ptree root = data->toPTree();

	std::stringstream ss;
	int request = root.get<int>("httpRequest", -1);
	std::string targetAddr;
	ServerDestinations_t dest;

	root.erase("httpRequest");

	if (request < 0) {
		// TODO: current data does not have any event type.
		return false;
	}

	if (mServerDestinations.find(request) == mServerDestinations.end()) {
		// TODO: current data does not have any event type.
		return false;
	}
	else {
		dest = mServerDestinations.find(request)->second;
	}

	// Add common properties
	root.put<std::string>("timeZone", mSensorInfo->TimeZone);
	write_json(ss, root);

	std::ofstream fout("sync_upload.txt");
	fout << ss.str() << endl;
	fout.close();

	return sendHTTP(dest.HTTPRequestMethod, ss.str(), dest.TargetPath);
}

bool ServerNetworkHandler::sendHTTP(const std::string method, const std::string jsonstring, const std::string targetAddr) const {
	using boost::asio::ip::tcp;

	try {
		boost::asio::io_service io_service;

		// Get a list of endpoints corresponding to the server name.
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(mServerAddr, "http");
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
		std::cout << "Exception: " << e.what() << "\n";
		return false;
	}

	return true;
}

bool ServerNetworkHandler::loadSettings(const std::string filename) {
	if (filename.empty()) {
		std::cout << "ERROR! Server data filename is empty." << std::endl;
		return false;
	}

	using boost::property_tree::ptree;

	ptree root;
	boost::property_tree::read_xml(filename, root);

	for (ptree::value_type const& v : root.get_child("ServerSettings")) {
		if (v.first == "ServerAddress") {
			mServerAddr = v.second.get_value<std::string>();
		}
		else if (v.first == "Request") {
			ServerDestinations_t dest;

			dest.RequestType = v.second.get<int>("RequestType");
			dest.HTTPRequestMethod = v.second.get<std::string>("HTTPRequestMethod");
			dest.TargetPath = v.second.get<std::string>("TargetPath");
			mServerDestinations[dest.RequestType] = dest;
		}
	}

	return true;
}

bool ServerNetworkHandler::writeSettings() {
	if (mSettingsFilename.empty()) {
		std::cout << "ERROR! Server data filename is empty." << std::endl;
		return false;
	}

	using boost::property_tree::ptree;

	ptree root;

	root.add("ServerSettings.ServerAddress", mServerAddr);
	for (const auto& data : mServerDestinations) {
		ptree &node = root.add("ServerSettings.Request", "");

		node.put("RequestType", data.second.RequestType);
		node.put("HTTPRequestMethod", data.second.HTTPRequestMethod);
		node.put("HTTPRequestMethod", data.second.TargetPath);
	}

	write_xml(mSettingsFilename, root, std::locale(), boost::property_tree::xml_writer_make_settings<std::string >('\t', 1));

	return true;
}
