#include "ssdp_client.h"
#include "sony_remote_camera.h"
#include "camera_state.h"
#include <map>

namespace src {

	class Sony_Remote_Camera_Implementation : public Sony_Remote_Camera_Interface {
	public:
		Sony_Remote_Camera_Implementation() = delete;
		Sony_Remote_Camera_Implementation(std::string my_own_ip_address);
		~Sony_Remote_Camera_Implementation();


		virtual Sony_Capture_Error Launch_Liveview();
		virtual Sony_Capture_Error Get_Last_JPeg_Image(uint8_t*& data, size_t& size, int& frame_number, int& timestamp);
		virtual Sony_Capture_Error Set_Shoot_Mode(Camera_State::Shoot_Mode mode);
		virtual Sony_Capture_Error Set_Recording(bool start);

	private:
		void Retrieve_Decription_File();
		void Handle_Write_HTTP_Request(bool mode_liveview, bool event_thread, const boost::system::error_code& err);
		void Handle_Read_Status_Line(bool mode_liveview, bool event_thread, const boost::system::error_code& err, const size_t bytes_transferred);
		void Handle_Read_Headers(bool mode_liveview, bool event_thread, const boost::system::error_code& err);
		void Handle_Read_Text_Content(bool event_thread, const boost::system::error_code& err);
		void Handle_Read_Common_Header(const boost::system::error_code& err, size_t payload_bytes_already_read, size_t image_bytes_already_read);
		void Handle_Read_Payload_Header(const boost::system::error_code& err, size_t image_bytes_already_read);
		void Handle_Read_Image(const boost::system::error_code& err);
		void Parse_Description();
		void Read_Jpeg_Content();
		void Read_Liveview_Continuously();
		Sony_Capture_Error Send_Options_Command(std::string json_request, bool event_thread = false);
		std::string Build_JSON_Command(const std::string& method, const std::vector<std::string>& params);
		bool Check_Event(std::istream& json_answer, std::string name, std::string expected_value);
		void Check_Events();


		boost::asio::io_service io_service;
		boost::asio::io_service io_service_liveview;
		boost::asio::io_service io_service_event_listener;
		boost::asio::streambuf tcp_request_options;
		boost::asio::streambuf tcp_response_options;
		boost::asio::streambuf tcp_request_liveview;
		boost::asio::streambuf tcp_response_liveview;
		boost::asio::streambuf tcp_request_events;
		boost::asio::streambuf tcp_response_events;
		boost::asio::ip::tcp::socket socket_options;
		boost::asio::ip::tcp::socket socket_liveview;
		boost::asio::ip::tcp::socket socket_event_listener;
		boost::asio::ip::tcp::resolver tcp_resolver;
		SSDP_Client SSDP_Client;
		std::vector<uint8_t> jpeg_image;
		uint8_t liveview_common_header[12];
		uint8_t liveview_payload_header[128];
		std::stringstream text_content;
		std::stringstream events_text_content;
		std::string liveview_url;
		std::string camera_service_url;
		boost::thread liveview_thread;
		boost::thread event_listener_thread;
		boost::mutex liveview_mutex;
		boost::mutex mtx;
		uint8_t* data;
		int timestamp;
		int frame_number;
		size_t file_size;
		bool got_it;
		int current_json_id;
		static const std::map<Camera_State::Shoot_Mode, std::string> shoot_modes_availables;

		Camera_State camera_state;
	};
}//namespace src