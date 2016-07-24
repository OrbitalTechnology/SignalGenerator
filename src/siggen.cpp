#include "siggen.hpp"

//std::string sentence = "$$MIKEL-3,1469352038,Flight7,02:19:59,37.3466186523,-121.946205139,32.0,0.0,3.95110321045,0.106474421918,0.420691430569,0.804084181786,0.0,0.0,24.1094760895,100874.882812,32.3646697998,1.39783334732,43*ED19";
//std::string sentence = "$$EAD-PLS-001";

char message[] = {"\0\0\0\0\0"};

float frequencies[] = {1500.0, 2000.0};
float freqL = 1500.0f;
float freqH = 2000.0f;

float freq[] = {
	freqL * M_PI * 2.0f,
	freqH * M_PI * 2.0f
};

float phase = 0.0f;
int amplitude = 32768;

int newFunction(int16_t* buffer, int index, int samplesPerBit, int bit) {
	float delta = freq[bit] / 44100; 

	for (unsigned int s = 0; s < samplesPerBit; s++) {
		buffer[index] = cos(phase) * amplitude;
		phase += delta;
		index++;
	}

	return index;
}


int main(int argc, char* argv[]) {

	std::cout << "libao example program" << std::endl;

	ao_initialize();
	driver = ao_default_driver_id();

	memset(&format, 0, sizeof(format));
	format.bits = 16;
	format.channels = 1;
	format.rate = 44100;
	format.byte_format = AO_FMT_LITTLE;

	device = ao_open_live(driver, &format, NULL);

	if (device == NULL) {
		std::cerr << "Error Opening Device" << std::endl;
		return -1;
	}

	// RTTY Info
	int len = 5;//sentence.size();
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

	buf_size = format.bits / 8 * format.channels * (format.rate * (sentenceLengthMilliseconds / 1000));
	buffer = (int16_t*) calloc(buf_size, sizeof(int16_t));

	int samplesPerBit = (format.rate / 1000) * milliSecondsSymbol;
	int index = 0;

	for(unsigned int c = 0; c < len; c++) {
		char ch = message[c];

		index = newFunction(buffer, index, samplesPerBit, 0);
		for (unsigned int b = 0; b < asciiMode; b++) {
			int bit = (c >> b) & 0x01;

			index = newFunction(buffer, index, samplesPerBit, bit);
		}
		index = newFunction(buffer, index, samplesPerBit, 1);
		index = newFunction(buffer, index, samplesPerBit, 2);
	}

	while (true) {
		ao_play(device, (char*)buffer, buf_size);
	}
	
	ao_close(device);
	ao_shutdown();

	return 0;
}