#include "camera_state.h"


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace src;
using namespace std;

void Camera_State::Update_State(std::istream& json_answer){
	using boost::property_tree::ptree;
	ptree pt;
	read_json(json_answer, pt);
	try {
		auto v2 = pt.get_child("error"); 
		for (ptree::value_type& v3 : v2.get_child("")) {
			//	for (ptree::value_type &v3 : pt.get_child("")){
			//results.push_back(v3.second);
			auto s = v3.second.data();
			//	}
		}
		return;
	}
	catch (std::exception){

	}
	auto v2 = pt.get_child("result");
	//auto v9 = pt.get<string>("currentShootMode");
	//auto v3 = v2.get_child("");
	vector<ptree> results;
	for (ptree::value_type& v3 : v2.get_child("")) {
		//	for (ptree::value_type &v3 : pt.get_child("")){
		results.push_back(v3.second);
		auto s = v3.second.data();
		//	}
	}
	auto v4 = results[0].get_child("names");
	vector<string> api;
	for (auto apinames : v4){
		api.push_back(apinames.second.data());
	}
	availableApiList = api;
	idle = (results[1].get<string>("cameraStatus") == "IDLE");
	zoom_information.zoom_index_current_box = results[2].get<double>("zoomIndexCurrentBox");
	zoom_information.zoom_number_box = results[2].get<double>("zoomNumberBox");
	zoom_information.zoom_position = results[2].get<double>("zoomPosition");
	zoom_information.zoom_position_current_box = results[2].get<double>("zoomPositionCurrentBox");
	liveview = results[3].get<bool>("liveviewStatus");
	liveview_orientation = results[4].get<int>("liveviewOrientation");
	string beep_mode = results[11].get<string>("currentBeepMode");
	Camera_State::Beep_Mode m;
	if (beep_mode == "Off") {
		m = Camera_State::BEEP_OFF;
	}
	else if (beep_mode == "On") {
		m = Camera_State::BEEP_ON;
	}
	else if (beep_mode == "Shutter Only"){
		m = Camera_State::BEEP_SHUTTER;
	}
	beep_mode = m;
	string ssm = results[21].get<string>("currentShootMode");
	Camera_State::Shoot_Mode sm;
	if (ssm == "still"){
		sm = Camera_State::STILL;
	}
	else if (ssm == "movie"){
		sm = Camera_State::MOVIE;
	}
	else if (ssm == "audio"){
		sm = Camera_State::AUDIO;
	}
	else if (ssm == "intervalstill"){
		sm = Camera_State::INTERVALSTILL;
	}
	shoot_mode = sm;
}