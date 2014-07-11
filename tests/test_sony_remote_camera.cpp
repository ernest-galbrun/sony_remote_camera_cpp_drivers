#define BOOST_TEST_MODULE sony_camera
#include <boost/test/unit_test.hpp>
#include <memory>
#include "../sony_capture/sony_remote_camera.h"

using namespace std;

shared_ptr<sony_remote_camera> create_camera(string s) {
	return shared_ptr<sony_remote_camera>(new sony_remote_camera(s));
}

BOOST_AUTO_TEST_CASE(camera_dsc_qx10) {
	BOOST_CHECK_THROW(create_camera("0.0.0.0"), ssdp_failure);
	shared_ptr<sony_remote_camera> s;
	BOOST_REQUIRE_NO_THROW(s = create_camera("10.0.1.1"));
}