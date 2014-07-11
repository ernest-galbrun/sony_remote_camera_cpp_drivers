#include "sony_remote_camera.h"


sony_remote_camera::sony_remote_camera(std::string my_own_ip_address) 
	:SSDP_Client(io_service,my_own_ip_address)
{
}

sony_remote_camera::~sony_remote_camera()
{
}
