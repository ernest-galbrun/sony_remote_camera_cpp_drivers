#define BOOST_TEST_MODULE sony_camera
#include <boost/test/unit_test.hpp>
//#include <boost/test/execution_monitor.hpp>
#include <memory>
#include <thread>
#include <iostream>
#include <fstream>
#include "../sony_capture/sony_remote_camera.h"



using namespace std;
using namespace src;

BOOST_AUTO_TEST_SUITE( dsc_qx10 )


BOOST_AUTO_TEST_CASE(liveview) {
    Sony_Remote_Camera_Interface* s;
	BOOST_REQUIRE(s = GetSonyRemoteCamera("10.0.1.1"));
	BOOST_CHECK(s->Launch_Liveview() == SC_NO_ERROR);
	uint8_t* img;
	size_t size;
	int ts, fn;
	auto err = s->Get_Last_JPeg_Image(img, size, fn, ts);
	BOOST_CHECK(err == SC_NO_NEW_DATA_AVAILABLE);
	this_thread::sleep_for(chrono::milliseconds(2000));
	err = s->Get_Last_JPeg_Image(img, size, fn, ts);
	BOOST_REQUIRE(err == SC_NO_ERROR);
	vector<uint8_t> start_marker = { 0xff, 0xd8 };
	vector<uint8_t> end_marker = { 0xff, 0xd9 };
	auto start = search(img, img + size, start_marker.begin(), start_marker.end());
	auto end = search(img, img + size, end_marker.begin(), end_marker.end());
	BOOST_CHECK(start != img + size);
	BOOST_CHECK(end != img + size);
	this_thread::sleep_for(chrono::milliseconds(200));
	free(img);
	err = s->Get_Last_JPeg_Image(img, size, fn, ts);
	BOOST_REQUIRE(err == SC_NO_ERROR);
	start = search(img, img + size, start_marker.begin(), start_marker.end());
	end = search(img, img + size, end_marker.begin(), end_marker.end());
	BOOST_CHECK(start != img + size);
	BOOST_CHECK(end != img + size);
	free(img);
}

BOOST_AUTO_TEST_CASE(camera_mode_movie){
    Sony_Remote_Camera_Interface* s;
	BOOST_REQUIRE(s = GetSonyRemoteCamera("10.0.1.1"));
	BOOST_CHECK(s->Launch_Liveview() == SC_NO_ERROR);
	BOOST_CHECK(s->Set_Shoot_Mode(Camera_State::MOVIE) == SC_NO_ERROR);
	uint8_t* img;
	size_t size;
	int ts, fn;
	decltype(s->Get_Last_JPeg_Image(img, size, fn, ts)) err;
	int i = 0;
	BOOST_CHECK(!s->Set_Recording(true));
	for (int i=0;i<100;++i){
		//err = s->Get_Last_JPeg_Image(img, size, fn, ts);
		this_thread::sleep_for(chrono::milliseconds(200));
		/*stringstream filename;
		filename << "image" << i << ".jpg";
		std::ofstream image_file(filename.str(), ios::out | ios::binary | ios::trunc);
		image_file.write((char*)img, size);
		image_file.close();*/
	}
	BOOST_CHECK(!s->Set_Recording(false));
}


BOOST_AUTO_TEST_CASE(wrong_ip) {
	Sony_Remote_Camera_Interface* s;
	s = GetSonyRemoteCamera("193.54.20.196");
	BOOST_CHECK(s == 0);
}

BOOST_AUTO_TEST_SUITE_END()