#include "Sony_Remote_Camera.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>

using namespace std;
using namespace boost;

std::shared_ptr<Sony_Remote_Camera_Interface> GetSonyRemoteCamera(string my_own_ip){
	try {
		return std::shared_ptr<Sony_Remote_Camera_Interface>(new Sony_Remote_Camera_Implementation(my_own_ip));
	}
	catch (std::exception e) {
		cerr << e.what();
		return std::shared_ptr<Sony_Remote_Camera_Interface>(nullptr);
	}
}


Sony_Remote_Camera_Implementation::Sony_Remote_Camera_Implementation(string my_own_ip_address) 
	:SSDP_Client(io_service,my_own_ip_address),
	socket_options(io_service),
	socket_liveview(io_service_liveview),
	tcp_resolver(io_service)
	//endpoint(io_service)
{
	io_service.run();
}

Sony_Remote_Camera_Implementation::~Sony_Remote_Camera_Implementation() {
}


Sony_Remote_Camera_Interface::Sony_Capture_Error
Sony_Remote_Camera_Implementation::Retrieve_Decription_File(){
	const string description_url(SSDP_Client.Get_Description_Url());
	size_t last_slash = description_url.rfind('/');
	size_t colon = description_url.rfind(':');
	string server = description_url.substr(7, colon-7);
	string port = description_url.substr(colon+1, last_slash - colon -1);
	string path = description_url.substr(last_slash);
	ostream request_stream(&tcp_request_options);
	request_stream << "GET " << path << " HTTP/1.1\r\n";
	request_stream << "Host: " << server << "\r\n";
	request_stream << "Accept: */*\r\n";
	//request_stream << "Connection: close\r\n";
	request_stream << "\r\n";
	asio::ip::tcp::resolver::query query(server, port);
	asio::ip::tcp::resolver::iterator endpoint_iterator = tcp_resolver.resolve(query);
	asio::connect(socket_options, endpoint_iterator);
	boost::asio::async_write(socket_options, tcp_request_options,
		boost::bind(&Sony_Remote_Camera_Implementation::Handle_Write_HTTP_Request, this, false, boost::asio::placeholders::error));
	try {
		io_service.reset();
		io_service.run();
		Parse_Description();
	}
	catch (ios_base::failure e) {
		cerr << e.what();
		return SC_ERROR;
	}
	return SC_NO_ERROR;
}


Sony_Remote_Camera_Implementation::Sony_Capture_Error Sony_Remote_Camera_Implementation::Launch_Liveview(){
	ostream request_stream(&tcp_request_options);
	size_t last_slash = camera_service_url.rfind('/');
	size_t colon = camera_service_url.rfind(':');
	string server = camera_service_url.substr(7, colon - 7);
	string port = camera_service_url.substr(colon + 1, last_slash - colon - 1);
	string path = camera_service_url.substr(last_slash);
	string json_request("{\"method\": \"startLiveview\",\"params\" : [],\"id\" : 1,\"version\" : \"1.0\"}\r\n");
	request_stream << "POST " << path << "/camera HTTP/1.1\r\n";
	request_stream << "Accept: application/json-rpc\r\n";
	request_stream << "Content-Length: " << json_request.length() << "\r\n";
	request_stream << "Content-Type: application/json-rpc\r\n";
	request_stream << "Host:" << camera_service_url << "\r\n";
	request_stream << "\r\n";
	request_stream << json_request;
	asio::ip::tcp::resolver::query query(server, port);
	//text_content.clear();
	//text_content.str("");
	asio::ip::tcp::resolver::iterator endpoint_iterator = tcp_resolver.resolve(query);
	asio::connect(socket_options, endpoint_iterator);

	boost::asio::async_write(socket_options, tcp_request_options,
		boost::bind(&Sony_Remote_Camera_Implementation::Handle_Write_HTTP_Request, this, false, 
			boost::asio::placeholders::error));
	try {
		io_service.reset();
		io_service.run();
		using boost::property_tree::ptree;
		ptree pt;
		string c = text_content.str();
		read_json(text_content, pt);
		auto v = pt.get_child("result").begin();
		liveview_url = v->second.data();		
	}
	catch (ios_base::failure e) {
		cerr << e.what();
		return SC_ERROR;
	}
	liveview_thread = thread(bind(&Sony_Remote_Camera_Implementation::Read_Liveview_Continuously, this));
	return SC_NO_ERROR;
}

void Sony_Remote_Camera_Implementation::Read_Liveview_Continuously(){
	io_service_liveview.reset();
	while (true){
		this_thread::interruption_point();
		ostream request_stream(&tcp_request_liveview);
		size_t last_slash = liveview_url.rfind('/');
		size_t colon = liveview_url.rfind(':');
		string server = liveview_url.substr(7, colon - 7);
		string port = liveview_url.substr(colon + 1, last_slash - colon - 1);
		string path = liveview_url.substr(last_slash);
		request_stream << "GET "<< path << " HTTP/1.1\r\n";
		request_stream << "Accept: image/jpeg\r\n";
		request_stream << "Host: " << server << "\r\n";
		request_stream << "\r\n";
		asio::ip::tcp::resolver::query query(server, port);
		boost::asio::ip::tcp::resolver resolver(io_service_liveview);
		asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		asio::connect(socket_liveview,endpoint_iterator);
		boost::asio::async_write(socket_liveview, tcp_request_liveview,
			boost::bind(&Sony_Remote_Camera_Implementation::Handle_Write_HTTP_Request, this, true,
			boost::asio::placeholders::error));
		io_service_liveview.run();
	}
}

void Sony_Remote_Camera_Implementation::Handle_Write_HTTP_Request(bool mode_liveview, const system::error_code& err) {
	asio::ip::tcp::socket& s = mode_liveview ? socket_liveview : socket_options;
	asio::streambuf& buf = mode_liveview ? tcp_response_liveview : tcp_response_options;
	if (!err) {
		// Read the response status line. The response_ streambuf will
		// automatically grow to accommodate the entire line. The growth may be
		// limited by passing a maximum size to the streambuf constructor.
		boost::asio::async_read_until(s, buf, "\r\n",
			boost::bind(&Sony_Remote_Camera_Implementation::Handle_Read_Status_Line, this, mode_liveview,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	} else {
		throw(ios_base::failure(err.message()));
	}
}

void Sony_Remote_Camera_Implementation::Handle_Read_Status_Line(bool mode_liveview, const system::error_code& err, const size_t bytes_transferred) {
	asio::ip::tcp::socket& s = mode_liveview ? socket_liveview : socket_options;
	asio::streambuf& buf = mode_liveview ? tcp_response_liveview : tcp_response_options;
	if (!err) {
		// Check that response is OK.
		std::istream response_stream(&buf);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
			stringstream m;
			m << "Invalid response\n";
			m << http_version << '\n';
			throw(ios_base::failure(m.str()));
		}
		if (status_code != 200) {
			stringstream m;
			m << "Response returned with status code: ";
			m << status_code << '\n';
			throw(ios_base::failure(m.str()));
		}
		// Read the response headers, which are terminated by a blank line.
		boost::asio::async_read_until(s, buf, "\r\n\r\n",
			boost::bind(&Sony_Remote_Camera_Implementation::Handle_Read_Headers, this, mode_liveview,
			boost::asio::placeholders::error));
	}
	else {
		stringstream m;
		m << "Error: " << err << "\n";
		throw(ios_base::failure(m.str()));
	}
}

void Sony_Remote_Camera_Implementation::Handle_Read_Headers(bool mode_liveview, const system::error_code& err) {
	asio::ip::tcp::socket& s = mode_liveview ? socket_liveview : socket_options;
	asio::streambuf& buf = mode_liveview ? tcp_response_liveview : tcp_response_options;
	if (!err) {
		// Process the response headers.
		std::istream response_stream(&buf);
		std::string header;
		while (std::getline(response_stream, header) && header != "\r"){
			//std::cout << header << "\n";
		}
		if (mode_liveview){
			Read_Jpeg_Content();				
		} else {
			// Write whatever content we already have to output.
			if (buf.size() > 0) {
				//std::cout << &tcp_response_;
				text_content << &buf;
			}
			// Start reading remaining data until EOF.
			boost::asio::async_read(socket_options, tcp_response_options,
				boost::asio::transfer_at_least(1),
				boost::bind(&Sony_Remote_Camera_Implementation::Handle_Read_Text_Content, this,
				boost::asio::placeholders::error));
		}
	}
	else {
		stringstream m;
		m << "Error: " << err << "\n";
		throw(ios_base::failure(m.str()));
	}
}

void Sony_Remote_Camera_Implementation::Handle_Read_Text_Content(const boost::system::error_code& err) {
	if (!err) {
		// Write all of the data that has been read so far.
		text_content << &tcp_response_options;
		// Continue reading remaining data until EOF.
		boost::asio::async_read(socket_options, tcp_response_options,
			boost::asio::transfer_at_least(1),
			boost::bind(&Sony_Remote_Camera_Implementation::Handle_Read_Text_Content, this,
			boost::asio::placeholders::error));
	} else if (err != boost::asio::error::eof) {
		std::cout << "Error: " << err << "\n";
	}
}

void Sony_Remote_Camera_Implementation::Read_Jpeg_Content() {
	size_t n = tcp_response_liveview.size();
	vector<uint8_t> v;
	if (n < 12) {
		tcp_response_liveview.sgetn((char *)liveview_common_header, n);
		boost::asio::async_read(socket_liveview, asio::buffer(liveview_common_header + n, 12 - n),
			boost::bind(&Sony_Remote_Camera_Implementation::Handle_Read_Common_Header, this,
			boost::asio::placeholders::error));
	}
	else if (n < 12 + 128) {
	n -= 12;
	tcp_response_liveview.sgetn((char *)liveview_common_header, 12);
	tcp_response_liveview.sgetn((char *)liveview_payload_header, n);
	boost::asio::async_read(socket_liveview, asio::buffer(liveview_payload_header + n, 128 - n),
		boost::bind(&Sony_Remote_Camera_Implementation::Handle_Read_Payload_Header, this,
	boost::asio::placeholders::error));
	} else {
		tcp_response_liveview.sgetn((char *)liveview_common_header, 12);
		tcp_response_liveview.sgetn((char *)liveview_payload_header, 128);
	n -= (128 + 12);
	tcp_response_liveview.sgetn((char *)&jpeg_image[0], n);
	size_t size = 256 * 256 * liveview_payload_header[4]
	+ 256 * liveview_payload_header[5]
	+ liveview_payload_header[6];
	boost::asio::async_read(socket_liveview, asio::buffer(&jpeg_image[0] + n, size - n),
		boost::bind(&Sony_Remote_Camera_Implementation::Handle_Read_Image, this,
	boost::asio::placeholders::error));
	}
}

void Sony_Remote_Camera_Implementation::Handle_Read_Common_Header(const boost::system::error_code& err) {

	boost::asio::async_read(socket_liveview, asio::buffer(liveview_payload_header, 128),
		boost::bind(&Sony_Remote_Camera_Implementation::Handle_Read_Payload_Header, this,
		boost::asio::placeholders::error));

}
void Sony_Remote_Camera_Implementation::Handle_Read_Payload_Header(const boost::system::error_code& err) {
	size_t size = liveview_payload_header[6]<<0 | liveview_payload_header[5]<<8 | liveview_payload_header[4]<<24;
	size += 10;
	jpeg_image.resize(size);
	boost::asio::async_read(socket_liveview, asio::buffer(&jpeg_image[0], size),
		boost::bind(&Sony_Remote_Camera_Implementation::Handle_Read_Image, this,
		boost::asio::placeholders::error));
}
void Sony_Remote_Camera_Implementation::Handle_Read_Image(const boost::system::error_code& err) {
	//cv::Mat imgbuf(image,false);
	//cv::Mat m(cv::imdecode(imgbuf, cv::IMREAD_COLOR)); 
	vector<uint8_t> start_marker = { 0xff, 0xd8 };
	vector<uint8_t> end_marker = { 0xff, 0xd9 };
	auto start = search(jpeg_image.begin(), jpeg_image.end(), start_marker.begin(), start_marker.end());
	auto end = search(jpeg_image.begin(), jpeg_image.end(), end_marker.begin(), end_marker.end());
	size_t a = start - jpeg_image.begin();
	size_t b = end - jpeg_image.begin();
	ofstream image_file("image.jpg", ios::out | ios::binary | ios::trunc);
	image_file.write((char*)&(*start), jpeg_image.end() - start);
	image_file.close();
	boost::asio::async_read(socket_liveview, asio::buffer(liveview_common_header, 12),
		boost::bind(&Sony_Remote_Camera_Implementation::Handle_Read_Common_Header, this,
		boost::asio::placeholders::error));
}

void Sony_Remote_Camera_Implementation::Parse_Description() {
	using boost::property_tree::ptree;
	ptree pt;
	read_xml(text_content, pt);
	liveview_url = pt.get<string>("root.device.av:X_ScalarWebAPI_DeviceInfo.av:X_ScalarWebAPI_ImagingDevice.av:X_ScalarWebAPI_LiveView_URL");
	for (ptree::value_type &v : pt.get_child("root.device.av:X_ScalarWebAPI_DeviceInfo.av:X_ScalarWebAPI_ServiceList")){
		string service = v.second.get<string>("av:X_ScalarWebAPI_ServiceType");
		if (service == "camera")
			camera_service_url = v.second.get<string>("av:X_ScalarWebAPI_ActionList_URL");
	}
}

vector<uint8_t> Sony_Remote_Camera_Implementation::GetLastJPegImage(){
	return jpeg_image;
}