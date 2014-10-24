sony_remote_camera_cpp_drivers
==============================

C++ drivers for sony remote camera

This implements a C++ class for the use of sony remote cameras based on the REST API provided by sony.
https://developer.sony.com/develop/cameras/

It includes a discovery protocol over ssdp, the http implementation using boost/asio, and the json parsing.

It has only been tested on a sony QX10 for the moment.
