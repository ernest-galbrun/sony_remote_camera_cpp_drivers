#pragma once

#include <vector>
#include <iostream>

#include <boost/thread.hpp>

namespace src {

	struct Camera_State {
		boost::mutex mtx;
		std::vector<std::string> availableApiList;
		bool idle;
		struct ZoomInformationType {
			double zoom_position;
			double zoom_number_box;
			double zoom_index_current_box;
			double zoom_position_current_box;
		} zoom_information;
		bool liveview;
		int liveview_orientation;
		enum Beep_Mode {BEEP_OFF,BEEP_ON,BEEP_SHUTTER} beep_mode;
		enum Movie_Quality {HQ,STD,VGA,SSLOW} movie_quality;
		struct Still_Size {
			bool availabity;
			std::string current_aspect;
			std::string current_size;
		} still_size;
		bool steady_mode;
		int view_angle;
		enum Exposure_Mode {INTELLIGENT_AUTO, APERTURE, SHUTTER} exposure_mode;
		enum Postview_Image_Size {ORIGINAL, TWO_M} postview_image_size;
		enum Shoot_Mode {STILL, MOVIE, AUDIO, INTERVALSTILL} shoot_mode;
		double current_exposure_compensation;
		enum Flash_Mode {FLASH_OFF, FLASH_AUTO, FLASH_ON};
		double f_number;
		int iso_speed_rate;
		std::string shutter_speed;

	
		void Update_State(std::istream& json_answer);
	};

}