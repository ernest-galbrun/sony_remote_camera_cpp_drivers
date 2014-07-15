#pragma once
#include "SSDP_Client.h"
#include <vector>
#include <string>
#include <boost\asio.hpp>
#include <memory>
#include <boost/thread.hpp>

#ifdef SONY_CAPTURE_EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif


class Sony_Remote_Camera_Interface {
public:
	DLL enum Sony_Capture_Error {
		SC_NO_ERROR = 0,
		SC_ERROR
	};

	virtual ~Sony_Remote_Camera_Interface(){};
	virtual Sony_Capture_Error Retrieve_Decription_File() = 0;
	virtual Sony_Capture_Error Launch_Liveview() = 0;
	virtual std::vector<uint8_t> GetLastJPegImage() = 0;
};

class Sony_Remote_Camera_Implementation : public Sony_Remote_Camera_Interface {
public:
	Sony_Remote_Camera_Implementation() = delete;
	Sony_Remote_Camera_Implementation(std::string my_own_ip_address);
	~Sony_Remote_Camera_Implementation();


	virtual Sony_Capture_Error Retrieve_Decription_File();
	virtual Sony_Capture_Error Launch_Liveview();
	virtual std::vector<uint8_t> GetLastJPegImage();
	
private:
	void Handle_Write_HTTP_Request(bool mode_liveview, const boost::system::error_code& err);
	void Handle_Read_Status_Line(bool mode_liveview, const boost::system::error_code& err, const size_t bytes_transferred);
	void Handle_Read_Headers(bool mode_liveview, const boost::system::error_code& err);
	void Handle_Read_Text_Content(const boost::system::error_code& err);
	void Handle_Read_Common_Header(const boost::system::error_code& err);
	void Handle_Read_Payload_Header(const boost::system::error_code& err);
	void Handle_Read_Image(const boost::system::error_code& err);
	void Parse_Description();
	void Read_Jpeg_Content();
	void Read_Liveview_Continuously();


	boost::asio::io_service io_service;
	boost::asio::io_service io_service_liveview;
	boost::asio::streambuf tcp_request_options;
	boost::asio::streambuf tcp_response_options;
	boost::asio::streambuf tcp_request_liveview;
	boost::asio::streambuf tcp_response_liveview;
	boost::asio::ip::tcp::socket socket_options;
	boost::asio::ip::tcp::socket socket_liveview;
	boost::asio::ip::tcp::resolver tcp_resolver;
	SSDP_Client SSDP_Client;
	std::vector<uint8_t> jpeg_image;
	uint8_t liveview_common_header[12];
	uint8_t liveview_payload_header[128];
	std::stringstream text_content;
	std::string liveview_url; 
	std::string camera_service_url;
	boost::thread liveview_thread;
};

DLL std::shared_ptr<Sony_Remote_Camera_Interface> GetSonyRemoteCamera(std::string my_own_ip);