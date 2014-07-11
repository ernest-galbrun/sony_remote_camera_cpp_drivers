#include <boost/asio.hpp>
#include <exception>

#ifdef SONY_CAPTURE_EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif

class DLL ssdp_failure : public std::ios_base::failure {
public:
	ssdp_failure(std::string s) :std::ios_base::failure(s){};
};

class DLL SSDP_Client
{
public:
	SSDP_Client() = delete;
	SSDP_Client(boost::asio::io_service& io_service, std::string my_own_ip);



private:
	const static boost::asio::ip::address multicast_address;
	const static int multicast_port;

	void Handle_Send_Discovery_Request(const boost::system::error_code& error);
	void Handle_Read_Header(const boost::system::error_code& err, size_t bytes_recvd);

	boost::asio::ip::udp::endpoint endpoint;
	boost::asio::ip::udp::socket socket;
	std::string description_url;
	std::string multicast_search_request;
	enum { max_length = 1024 };
	char data_[max_length];
};