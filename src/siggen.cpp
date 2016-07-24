#include "siggen.hpp"

int main(int argc, char* argv[]) {

	ao_device *device;
	ao_sample_format format;
	int default_driver;
	char* buffer;
	int buf_size;
	int sample;
	float freq = 440.0;
	int i;

	std::cout << "libao example program" << std::endl;

	ao_initialize();

	default_driver = ao_default_driver_id();

	memset(&format, 0, sizeof(format));
	format.bits = 16;
	format.channels = 2;
	format.rate = 44100;
	format.byte_format = AO_FMT_LITTLE;

	return 0;
}