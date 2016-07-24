#include "siggen.hpp"

//std::string sentence = "$$MIKEL-3,1469352038,Flight7,02:19:59,37.3466186523,-121.946205139,32.0,0.0,3.95110321045,0.106474421918,0.420691430569,0.804084181786,0.0,0.0,24.1094760895,100874.882812,32.3646697998,1.39783334732,43*ED19";
std::string sentence = "$$EAD-PLS-001";

float frequencies[] = {1500.0, 2000.0};

void populateBuffer(char* buf, int startIndex, int length, int bit) {
	int sample = 0;

	for (unsigned int i = startIndex; i < (startIndex + length); i++) {
		sample = (int)(2.0 * 32768.0 * sin(2 * M_PI * frequencies[bit] * ((float) i / length)));
		
		buf[4 * i] = sample & 0xff;
		buf[4 * i+1] = (sample >> 8) & 0xff;
		buf[4 * i+2] = sample & 0xff;
		buf[4 * i+3] = (sample >> 8) & 0xff;
	}
}

int main(int argc, char* argv[]) {

	ao_device *device;
	ao_sample_format format;
	int driver;
	char* buffer;
	int buf_size;

	std::cout << "libao example program" << std::endl;

	ao_initialize();

	int count = 0;
	ao_info** driver_info = ao_driver_info_list(&count);

	for(unsigned int d = 0; d < count; d++) {
		ao_info* info = driver_info[d];
		std::cout << d << ": [" << info->type << "] - " << info->short_name << std::endl;
	}

	std::cout << "Driver Count: " << count << std::endl;

	driver = ao_default_driver_id();

	memset(&format, 0, sizeof(format));
	format.bits = 16;
	format.channels = 2;
	format.rate = 44100;
	format.byte_format = AO_FMT_LITTLE;


	// RTTY Info
	int len = sentence.size();
	std::cout << "Byte Count: " << len << std::endl;
	
	int startBitCount = 1;
	int endBitCount = 2;
	int asciiMode = 8;
	int baudRate = 50;

	int symbolCount = (len * asciiMode) + (len * startBitCount) + (len * endBitCount);
	std::cout << "Symbol Count: " << symbolCount << std::endl;

	float milliSecondsSymbol = (1.0f / (baudRate / 1000.0f));
	std::cout << "ms/symbol: " << milliSecondsSymbol << std::endl;

	float sentenceLengthMilliseconds = milliSecondsSymbol * symbolCount;
	std::cout << "Total Sentence Length: " << sentenceLengthMilliseconds / 1000 << std::endl;

	int bufferSize = sentenceLengthMilliseconds * format.rate;
	// End RTTY Info

	device = ao_open_live(driver, &format, NULL);

	if (device == NULL) {
		std::cerr << "Error Opening Device" << std::endl;
		return -1;
	}

	buf_size = format.bits / 8 * format.channels * (format.rate * (sentenceLengthMilliseconds / 1000));
	buffer = (char*) calloc(buf_size, sizeof(char));

	int samplesPerBit = (format.rate / 1000) * milliSecondsSymbol;
	int index = 0;

	for(unsigned int c = 0; c < len; c++) {
		char ch = sentence[c];
		for (unsigned int b = 0; b < asciiMode; b++) {
			int bit = (c >> b) & 0x01;

			populateBuffer(buffer, index, samplesPerBit, bit);
			index += samplesPerBit;
		}
	}

	ao_play(device, buffer, buf_size);
	
	ao_close(device);
	ao_shutdown();

	return 0;
}