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
	catch (std::exception) {
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
