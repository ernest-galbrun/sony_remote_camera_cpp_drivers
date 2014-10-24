#pragma once
//#include "SSDP_Client.h"
#include "camera_state.h"
#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include <boost\asio.hpp>
#include <boost/thread.hpp>

#ifdef SONY_CAPTURE_EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif

namespace src {
	DLL enum Sony_Capture_Error {
		SC_NO_ERROR = 0,
		SC_ERROR,
		SC_NO_NEW_DATA_AVAILABLE,
		SC_WRONG_PARAMETER
	};

	class Sony_Remote_Camera_Interface {
	public:
		virtual ~Sony_Remote_Camera_Interface(){};
		//virtual Sony_Capture_Error Retrieve_Decription_File() = 0;
		virtual Sony_Capture_Error Launch_Liveview() = 0;
		// Get Liveview image along with its size, frame number and timestamp.
		// It is your responsability to free the memory pointed to data when you have finished using it
        // returns SC_NO_NEW_DATA_AVAILABLE and dont modify the inpupts if this image has already been sent
		virtual Sony_Capture_Error Get_Last_JPeg_Image(uint8_t*& data, size_t& size, int& frame_number, int& timestamp) = 0;
		virtual Sony_Capture_Error Set_Shoot_Mode(Camera_State::Shoot_Mode mode) = 0;
		virtual Sony_Capture_Error Set_Recording(bool start) = 0;
	};

	DLL Sony_Remote_Camera_Interface* GetSonyRemoteCamera(std::string my_own_ip);

}