#pragma once
#include "SSDP_Client.h"
#include <string>
#include <boost\asio.hpp>

#ifdef SONY_CAPTURE_EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif


class DLL sony_remote_camera
{
public:
	sony_remote_camera() = delete;
	sony_remote_camera(std::string my_own_ip_address);
	~sony_remote_camera();
	
private:
	boost::asio::io_service io_service;
	SSDP_Client SSDP_Client;
};

