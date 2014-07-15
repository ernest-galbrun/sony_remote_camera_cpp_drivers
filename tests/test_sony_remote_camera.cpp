#define BOOST_TEST_MODULE sony_camera
#include <boost/test/unit_test.hpp>
#include <memory>
#include <thread>
#include "../sony_capture/sony_remote_camera.h"

using namespace std;

//shared_ptr<Sony_Remote_Camera_Implementation> create_camera(string s) {
//	return shared_ptr<Sony_Remote_Camera_Implementation>(new Sony_Remote_Camera_Implementation(s));
//}

BOOST_AUTO_TEST_CASE(camera_dsc_qx10) {
	//BOOST_CHECK(!GetSonyRemoteCamera("193.54.20.196"));
	shared_ptr<Sony_Remote_Camera_Interface> s;
	BOOST_CHECK(s = GetSonyRemoteCamera("10.0.1.1"));
	BOOST_CHECK(s->Retrieve_Decription_File() == Sony_Remote_Camera_Interface::SC_NO_ERROR);
	BOOST_CHECK(s->Launch_Liveview() == Sony_Remote_Camera_Interface::SC_NO_ERROR);
	auto img = s->GetLastJPegImage();
	vector<uint8_t> start_marker = { 0xff, 0xd8 };
	vector<uint8_t> end_marker = { 0xff, 0xd9 };
	auto start = search(img.begin(), img.end(), start_marker.begin(), start_marker.end());
	auto end = search(img.begin(), img.end(), end_marker.begin(), end_marker.end());
	BOOST_CHECK(start != img.end());
	BOOST_CHECK(end != img.end());
	this_thread::sleep_for(chrono::milliseconds(200)); 
	img = s->GetLastJPegImage();
	start = search(img.begin(), img.end(), start_marker.begin(), start_marker.end());
	end = search(img.begin(), img.end(), end_marker.begin(), end_marker.end());
	BOOST_CHECK(start != img.end());
	BOOST_CHECK(end != img.end());
}