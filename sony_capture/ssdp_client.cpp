#include "SSDP_Client.h"
#include <boost/bind.hpp>
#include <iostream>
#include <chrono>

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

const asio::ip::address SSDP_Client::multicast_address = address_v4::from_string("239.255.255.250");
const int SSDP_Client::multicast_port = 1900;

SSDP_Client::SSDP_Client(asio::io_service& io_service, string my_own_ip)
	:endpoint(multicast_address, multicast_port),
	socket(io_service, endpoint.protocol()),
	timer(io_service) 
{
	stringstream os;
	os << "M-SEARCH * HTTP/1.1\r\n";
	os << "HOST: 239.255.255.250:1900\r\n";
	os << "MAN: \"ssdp:discover\"\r\n";
	os << "MX: 4\r\n";
	os << "ST: urn:schemas-sony-com:service:ScalarWebAPI:1\r\n";
	os << "\r\n";
	multicast_search_request = os.str();
	try {
		socket.set_option(multicast::outbound_interface(address_v4::from_string(my_own_ip)));
	}
	catch (std::exception& e) {
		ssdp_failure f("Could not bind to the specified ip.\n");
		throw(f);
	}
	socket.async_send_to(buffer(multicast_search_request), endpoint, 
		boost::bind(&SSDP_Client::Handle_Send_Discovery_Request, this,
		boost::asio::placeholders::error));		

};


void SSDP_Client::Handle_Send_Discovery_Request(const boost::system::error_code& error){
	if (!error) {
		socket.async_receive(asio::buffer(data_),
			boost::bind(&SSDP_Client::Handle_Read_Header, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
		timer.expires_from_now(chrono::milliseconds(5000));
		timer.async_wait(boost::bind(&SSDP_Client::Handle_Discovery_Timeout, this,
			boost::asio::placeholders::error));
	}
	
}

void SSDP_Client::Handle_Discovery_Timeout(const boost::system::error_code& error){
	if (!error) {
		socket.close();
		ssdp_failure e("No response from the device: wrong ip or no device listening on this network\n");
		throw(e);
	}
	else {
		cout << error;
		cout << '\n' << error.message() << '\n';
		if (error == asio::error::operation_aborted){
			//our job here is done
		}
		else {
			stringstream m;
			m << "Error trying to send ssdp discovery request\n" << error << "\n";
			m << error.message();
			ssdp_failure e(m.str());
			throw(e);
		}
	}
}

void SSDP_Client::Handle_Read_Header(const boost::system::error_code& err, size_t bytes_recvd){
	if (!err) {
		// Check that response is OK.
		stringstream http_response = stringstream(data_);
		string http_version;
		http_response >> http_version;
		unsigned int status_code;
		http_response >> status_code;
		string status_message;
		getline(http_response, status_message);
		if (!http_response || http_version.substr(0, 5) != "HTTP/") {
			ssdp_failure e("invalid response from the device:\n" + http_version);
			throw(e);
		}
		if (status_code != 200) {
			stringstream m;
			m << "Response returned with status code " << http_version;
			ssdp_failure e(m.str());
			throw(e);
		}
		string header;
		while (getline(http_response, header) && header != "\r") {
			if (header.substr(0, 9) == "LOCATION:") {
				description_url = header.substr(10, header.length()-11);
				break;
			}
		}
		timer.cancel();
	} else {
		stringstream m;
		m << "Error trying to receive ssdp answer\n" << err << "\n";
		m << err.message();
		ssdp_failure e(m.str());
		throw(e);
	}
}

//#include <iostream>
//#include <sstream>
//#include <string>
//#include <thread>
//#include <chrono>
//#include <boost/asio.hpp>
//#include <boost/bind.hpp>
//#include <boost/date_time/posix_time/posix_time_types.hpp>
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/xml_parser.hpp>
//#include <boost/property_tree/json_parser.hpp>
//#include <opencv2/opencv.hpp>
//
//
//using namespace std;
//using namespace boost;
//
//
//class SSDP_Client
//{
//public:
//	SSDP_Client(boost::asio::io_service& io_service,
//		const boost::asio::ip::address& multicast_address)
//		:endpoint_(multicast_address, 1900),
//		socket_(io_service, endpoint_.protocol()),
//		tcp_socket_(io_service),
//		tcp_resolver_(io_service),
//		liveview_socket_(io_service)
//	{
//		stringstream os;
//		os << "M-SEARCH * HTTP/1.1\r\n";
//		os << "HOST: 239.255.255.250:1900\r\n";
//		os << "MAN: \"ssdp:discover\"\r\n";
//		os << "MX: 4\r\n";
//		os << "ST: urn:schemas-sony-com:service:ScalarWebAPI:1\r\n";
//		os << "\r\n";
//		message_ = os.str();
//		socket_.set_option(asio::ip::multicast::outbound_interface(asio::ip::address_v4::from_string("10.0.1.1")));
//		socket_.async_send_to(
//			boost::asio::buffer(message_), endpoint_,
//			boost::bind(&SSDP_Client::handle_send_to, this,
//			boost::asio::placeholders::error));		
//	}
//
//	void handle_send_to(const boost::system::error_code& error) {
//		if (!error) {
//			socket_.async_receive(asio::buffer(data_),// "\r\n",
//				boost::bind(&SSDP_Client::handle_read_header, this,
//				boost::asio::placeholders::error,
//				boost::asio::placeholders::bytes_transferred));
//		}
//	}
//
//	void handle_read_header(const boost::system::error_code& err, size_t bytes_recvd) {
//		if (!err) {
//			// Check that response is OK.
//			http_response = stringstream(data_);
//			string http_version;
//			http_response >> http_version;
//			unsigned int status_code;
//			http_response >> status_code;
//			string status_message;
//			getline(http_response, status_message);
//			if (!http_response || http_version.substr(0, 5) != "HTTP/") {
//				std::cout << "Invalid response\n";
//				return;
//			}
//			if (status_code != 200) {
//				std::cout << "Response returned with status code ";
//				std::cout << status_code << "\n";
//				return;
//			}
//			string header;
//			string description_url;
//			while (getline(http_response, header) && header != "\r") {
//				if (header.substr(0, 9) == "LOCATION:") {
//					description_url = header.substr(10, header.length()-11);
//				}
//				cout << header << "\n";
//			}
//			cout << "\n";
//			LaunchConnection(description_url);
//		} else {
//			std::cout << "Error: " << err << "\n";
//		}
//	}
//
//	void LaunchConnection(string description_url){
//		size_t last_slash = description_url.rfind('/');
//		size_t colon = description_url.rfind(':');
//		string server = description_url.substr(7, colon-7);
//		string port = description_url.substr(colon+1, last_slash - colon -1);
//		//server = "10.0.0.1";
//		string path = description_url.substr(last_slash);
//		ostream request_stream(&tcp_request_);
//		request_stream << "GET " << path << " HTTP/1.1\r\n";
//		request_stream << "Host: " << server << "\r\n";
//		request_stream << "Accept: */*\r\n";
//		request_stream << "Connection: close\r\n\r\n";
//		asio::ip::tcp::resolver::query query(server, port);
//		tcp_resolver_.async_resolve(query,
//			boost::bind(&SSDP_Client::handle_resolve, this,
//			boost::asio::placeholders::error,
//			boost::asio::placeholders::iterator));
//	}
//
//	void handle_resolve(const boost::system::error_code& err,
//		asio::ip::tcp::resolver::iterator endpoint_iterator)	{
//		if (!err) {
//			// Attempt a connection to each endpoint in the list until we
//			// successfully establish a connection.
//			boost::asio::async_connect(tcp_socket_, endpoint_iterator,
//				boost::bind(&SSDP_Client::handle_connect, this,
//				boost::asio::placeholders::error));	
//		} else {
//			cout << "Error: " << err.message() << "\n";
//		}
//	}
//
//	void handle_connect(const boost::system::error_code& err) {
//		if (!err) {
//			// The connection was successful. Send the request.
//			boost::asio::async_write(tcp_socket_, tcp_request_,
//				boost::bind(&SSDP_Client::handle_write_request, this,
//				boost::asio::placeholders::error));
//		} else {
//			std::cout << "Error: " << err.message() << "\n";
//		}
//	}
//
//
//	void handle_write_request(const boost::system::error_code& err) {
//		if (!err) {
//			// Read the response status line. The response_ streambuf will
//			// automatically grow to accommodate the entire line. The growth may be
//			// limited by passing a maximum size to the streambuf constructor.
//			boost::asio::async_read_until(tcp_socket_, tcp_response_, "\r\n",
//				boost::bind(&SSDP_Client::handle_read_status_line, this,
//				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
//		} else {
//			std::cout << "Error: " << err.message() << "\n";
//		}
//	}
//
//	void handle_read_status_line(const boost::system::error_code& err, const size_t bytes_transferred) {
//		if (!err) {
//			// Check that response is OK.
//			std::istream response_stream(&tcp_response_);
//			std::string http_version;
//			response_stream >> http_version;
//			unsigned int status_code;
//			response_stream >> status_code;
//			std::string status_message;
//			std::getline(response_stream, status_message);
//			if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
//				std::cout << "Invalid response\n";
//				return;
//			}
//			if (status_code != 200) {
//				std::cout << "Response returned with status code ";
//				std::cout << status_code << "\n";
//				return;
//			}
//			// Read the response headers, which are terminated by a blank line.
//			boost::asio::async_read_until(tcp_socket_, tcp_response_, "\r\n\r\n",
//				boost::bind(&SSDP_Client::handle_read_headers, this,
//				boost::asio::placeholders::error));
//		} else {
//			std::cout << "Error: " << err << "\n";
//			cout << err.message();
//		}
//	}
//
//	void handle_read_headers(const boost::system::error_code& err) {
//		if (!err) {
//			// Process the response headers.
//			std::istream response_stream(&tcp_response_);
//			std::string header;
//			while (std::getline(response_stream, header) && header != "\r")
//				std::cout << header << "\n";
//			std::cout << "\n";
//
//			
//			if (reading_image) {
//				size_t n = tcp_response_.size();
//				vector<uint8_t> v;
//				if (n < 12) {
//					tcp_response_.sgetn((char *)liveview_common_header, n);
//					boost::asio::async_read(liveview_socket_, asio::buffer(liveview_common_header+n, 12-n),
//						boost::bind(&SSDP_Client::handle_read_common_header, this,
//						boost::asio::placeholders::error));
//				}
//				else if (n < 12 + 128) {
//					n -= 12;
//					tcp_response_.sgetn((char *)liveview_common_header, 12);
//					tcp_response_.sgetn((char *)liveview_payload_header, n); 
//					boost::asio::async_read(liveview_socket_, asio::buffer(liveview_payload_header + n, 128 - n),
//						boost::bind(&SSDP_Client::handle_read_payload_header, this,
//						boost::asio::placeholders::error));
//				} else {
//					tcp_response_.sgetn((char *)liveview_common_header, 12);
//					tcp_response_.sgetn((char *)liveview_payload_header, 128);
//					n -= (128 + 12);
//					tcp_response_.sgetn((char *)&image[0], n);
//					size_t size = 256 * 256 * liveview_payload_header[4]
//						+ 256 * liveview_payload_header[5]
//						+ liveview_payload_header[6];
//					boost::asio::async_read(liveview_socket_, asio::buffer(&image[0] + n, size - n),
//						boost::bind(&SSDP_Client::handle_read_image, this,
//						boost::asio::placeholders::error));
//				}
//				
//				
//			} else {
//				// Write whatever content we already have to output.
//				if (tcp_response_.size() > 0) {
//					//std::cout << &tcp_response_;
//					content << &tcp_response_;
//				}
//				// Start reading remaining data until EOF.
//				boost::asio::async_read(tcp_socket_, tcp_response_,
//					boost::asio::transfer_at_least(1),
//					boost::bind(&SSDP_Client::handle_read_content, this,
//					boost::asio::placeholders::error));
//			}
//		} else {
//			std::cout << "Error: " << err << "\n";
//		}
//	}
//
//	void handle_read_content(const boost::system::error_code& err) {
//		if (!err) {
//			// Write all of the data that has been read so far.
//			content << &tcp_response_;
//			// Continue reading remaining data until EOF.
//			boost::asio::async_read(tcp_socket_, tcp_response_,
//				boost::asio::transfer_at_least(1),
//				boost::bind(&SSDP_Client::handle_read_content, this,
//				boost::asio::placeholders::error));
//		} else if (err != boost::asio::error::eof) {
//			std::cout << "Error: " << err << "\n";
//		}
//	}
//	
//	void parse_description() {
//		//tcp_socket_.close();
//		using boost::property_tree::ptree;
//		ptree pt;
//		read_xml(content, pt);
//		liveview_url = pt.get<string>("root.device.av:X_ScalarWebAPI_DeviceInfo.av:X_ScalarWebAPI_ImagingDevice.av:X_ScalarWebAPI_LiveView_URL");
//		for (ptree::value_type &v : pt.get_child("root.device.av:X_ScalarWebAPI_DeviceInfo.av:X_ScalarWebAPI_ServiceList")){
//			string service = v.second.get<string>("av:X_ScalarWebAPI_ServiceType");
//			if (service == "camera")
//				camera_service_url = v.second.get<string>("av:X_ScalarWebAPI_ActionList_URL");
//		}
//	}
//
//	void start_liveview() {
//		ostream request_stream(&tcp_request_);
//		size_t last_slash = camera_service_url.rfind('/');
//		size_t colon = camera_service_url.rfind(':');
//		string server = camera_service_url.substr(7, colon - 7);
//		string port = camera_service_url.substr(colon + 1, last_slash - colon - 1);
//		string path = camera_service_url.substr(last_slash);
//		string json_request("{\"method\": \"startLiveview\",\"params\" : [],\"id\" : 1,\"version\" : \"1.0\"}\r\n");
//		request_stream << "POST " << path << "/camera HTTP/1.1\r\n";
//		request_stream << "Accept: application/json-rpc\r\n"; 
//		request_stream << "Content-Length: " << json_request.length() << "\r\n";
//		request_stream << "Content-Type: application/json-rpc\r\n";
//		request_stream << "Host:" << camera_service_url<<"\r\n";
//		request_stream << "\r\n";
//		request_stream << json_request;
//		asio::ip::tcp::resolver::query query(server, port);
//		content.clear();
//		content.str("");
//		tcp_resolver_.async_resolve(query,
//			boost::bind(&SSDP_Client::handle_resolve, this,
//			boost::asio::placeholders::error,
//			boost::asio::placeholders::iterator));
//
//	}
//
//	void get_live_image() {
//		using boost::property_tree::ptree;
//		ptree pt;
//		string c = content.str();
//		read_json(content, pt);
//		auto v = pt.get_child("result").begin();
//		liveview_url = v->second.data();		
//		ostream request_stream(&tcp_request_);
//		size_t last_slash = liveview_url.rfind('/');
//		size_t colon = liveview_url.rfind(':');
//		string server = liveview_url.substr(7, colon - 7);
//		string port = liveview_url.substr(colon + 1, last_slash - colon - 1);
//		string path = liveview_url.substr(last_slash);
//		request_stream << "GET "<< path << " HTTP/1.1\r\n";
//		request_stream << "Accept: image/jpeg\r\n";
//		//request_stream << "Content-Length: " << json_request.length() << "\r\n";
//		//request_stream << "Content-Type: application/json-rpc\r\n";
//		request_stream << "Host: " << server << "\r\n";
//		request_stream << "\r\n";
//		asio::ip::tcp::resolver::query query(server, port);
//		asio::ip::tcp::resolver::iterator endpoint_iterator = tcp_resolver_.resolve(query);
//		asio::connect(liveview_socket_,endpoint_iterator);
//		reading_image = true;
//		boost::asio::async_write(liveview_socket_, tcp_request_,
//			boost::bind(&SSDP_Client::handle_write_request_liveview, this,
//			boost::asio::placeholders::error));
//	}
//
//	void handle_write_request_liveview(const boost::system::error_code& err) {
//
//		boost::asio::async_read_until(liveview_socket_, tcp_response_, "\r\n",
//			boost::bind(&SSDP_Client::handle_read_status_line, this,
//			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));		
//	}
//
//	void handle_read_common_header(const boost::system::error_code& err) {
//
//		boost::asio::async_read(liveview_socket_, asio::buffer(liveview_payload_header, 128),
//			boost::bind(&SSDP_Client::handle_read_payload_header, this,
//			boost::asio::placeholders::error));
//
//	}
//	void handle_read_payload_header(const boost::system::error_code& err) {
//		size_t size = liveview_payload_header[6]<<0 | liveview_payload_header[5]<<8 | liveview_payload_header[4]<<24;
//		size += 10;
//		image.resize(size);
//		boost::asio::async_read(liveview_socket_, asio::buffer(&image[0], size),
//			boost::bind(&SSDP_Client::handle_read_image, this,
//			boost::asio::placeholders::error));
//	}
//	void handle_read_image(const boost::system::error_code& err) {
//		cv::Mat imgbuf(image,false);
//		cv::Mat m(cv::imdecode(imgbuf, cv::IMREAD_COLOR)); 
//		vector<uint8_t> start_marker = { 0xff, 0xd8 };
//		vector<uint8_t> end_marker = { 0xff, 0xd9 };
//		auto start = search(image.begin(), image.end(), start_marker.begin(), start_marker.end());
//		auto end = search(image.begin(), image.end(), end_marker.begin(), end_marker.end());
//		int a = start - image.begin();
//		int b = end - image.begin();
//		ofstream image_file("image.jpg", ios::out | ios::binary | ios::trunc);
//		image_file.write((char*)&(*start), image.end()-start);
//		image_file.close();
//		boost::asio::async_read(liveview_socket_, asio::buffer(liveview_common_header, 12),
//			boost::bind(&SSDP_Client::handle_read_common_header, this,
//			boost::asio::placeholders::error));
//	}
//
//
//
//private:
//	boost::asio::ip::udp::endpoint endpoint_;
//	boost::asio::ip::udp::socket socket_;
//	boost::asio::ip::tcp::socket tcp_socket_;
//	boost::asio::ip::tcp::socket liveview_socket_;
//	int message_count_;
//	std::string message_;
//	enum { max_length = 1024 };
//	char data_[max_length];
//	stringstream http_response;
//	stringstream content;
//	asio::ip::tcp::resolver tcp_resolver_;
//	boost::asio::streambuf tcp_request_;
//	boost::asio::streambuf tcp_response_;
//	//iostream image;
//	string liveview_url;
//	string camera_service_url;
//	uint8_t liveview_common_header[12];
//	uint8_t liveview_payload_header[128];
//	vector<uint8_t> image;
//	bool reading_image;
//};
//
//int main(int argc, char* argv[])
//{
//	try
//	{
//		boost::asio::io_service io_service;
//		SSDP_Client m(io_service, boost::asio::ip::address::from_string("239.255.255.250"));
//		io_service.run();
//		m.parse_description();
//		m.start_liveview();
//		io_service.reset();
//		io_service.run();
//		m.get_live_image();
//		io_service.reset();
//		io_service.run();
//			
//	} catch (std::exception& e) {
//		std::cerr << "Exception: " << e.what() << "\n";
//	}
//	return 0;
//} 