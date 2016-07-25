#include "siggen.hpp"

//std::string input = "$$EAD-PLS-00,123,12:00:00,53.194,-2.902,1000";
std::string input = "$$TEST";

float freqOffset = 1500.0f;
float freqShift = 1000.0f;

float freq[] = {
	freqOffset * M_PI * 2.0f,
	(freqOffset + freqShift) * M_PI * 2.0f
};

float phase = 0.0f;
float amplitude = 1.0f;

// RTTY Info
int len = 0;	
int loopCount = 1;
int startBitCount = 1;
int stopBitCount = 2;
int asciiMode = 8;
int baudRate = 50;

float milliSecondsSymbol = (1000.0f / baudRate);
// End RTTY Info

int pushBit(std::vector<int16_t>* buffer, int index, int samplesPerBit, int bit) {
	
	float delta = freq[bit] / 44100; 

	for (unsigned int s = 0; s < samplesPerBit; s++) {
		buffer->push_back(cos(phase) * (32768 * amplitude));
		phase += delta;
		index++;
	}

	while(phase > M_PI * 2) phase -= M_PI * 2;

	return index;
}

int init() {
	ao_initialize();
	driver = ao_default_driver_id();
	std::cout << "Default Driver: " << driver << std::endl;
	
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

	return 0;
}

uint16_t crc_xmodem_update (uint16_t crc, uint8_t data) {
	int i;

	crc = crc ^ ((uint16_t)data << 8);
	for (i=0; i<8; i++)
	{
		if (crc & 0x8000)
			crc = (crc << 1) ^ 0x1021;
		else
			crc <<= 1;
	}
	 
	return crc;
}

std::string checksum(const std::string input) {
	size_t i;
	uint16_t crc;
	uint8_t c;
 
	crc = 0xFFFF;
 
	// Calculate checksum ignoring the first two $s
	for (i = 2; i < input.size(); i++)
	{
		c = input[i];
		crc = crc_xmodem_update(crc, c);
	}
 	
 	char buf[6];
	sprintf (buf, "%u", crc);

	char chcksum[10];
	memset(chcksum, '\0', sizeof(chcksum));

	snprintf(chcksum, sizeof(chcksum), "*%04X\n", crc);

	return std::string(chcksum, sizeof(chcksum));
}

void generate() {
	std::string hash = checksum(input);
	std::string sentence = input + hash;
	std::cout << sentence << std::endl;

	len = sentence.size();

	int index = 0;
	int startBitPreambleCount = 20;
	int samplesPerBit = (format.rate / 1000) * milliSecondsSymbol;

	// Preamble
	for(unsigned int s = 0; s < startBitPreambleCount; s++) {
		index = pushBit(&newBuffer, index, samplesPerBit, 1);
	}

	index = 0;
	for(unsigned int c = 0; c < len; c++) {
		char ch = sentence[c];

		// Start Bit
		for(unsigned int b = 0; b < startBitCount; b++) {
			index = pushBit(&newBuffer, index, samplesPerBit, 0);	
		}

		// Character Bit
		for (unsigned int b = 0; b < asciiMode; b++) {
			int bit = (ch >> b) & 0x01;
			index = pushBit(&newBuffer, index, samplesPerBit, bit);
		}
		
		// Stop Bit
		for(unsigned int b = 0; b < stopBitCount; b++) {
			index = pushBit(&newBuffer, index, samplesPerBit, 1);	
		}
	}	
}

void play() {
	// Play Buffer n Times
	for(unsigned int l = 0; l < loopCount; l++) {	
		ao_play(device, (char*) &newBuffer[0], newBuffer.size() * sizeof(int16_t));
	}
}

void help() {
	std::cout << "Orbital Technologies RTTY Signal Generator" << std::endl;
	std::cout << "\t--message \"sentence\": Message to transmit" << std::endl;
	std::cout << "\t[--list] : List Audio Output Devices" << std::endl;
	std::cout << "\t[--baud n]: Set Baud Rate" << std::endl;
	std::cout << "\t[--stop n]: Stop Bits" << std::endl;
	std::cout << "\t[--start n]: Start Bits" << std::endl;
	std::cout << "\t[--loop n]: Loop Signal n times" << std::endl;
}

int main(int argc, char* argv[]) {
	// if (argc == 1) {
	// 	help();
	// 	return 0;
	// }

	if(init() < 0) {
		return -1;
	}

	int count = 0;
	ao_info** info = ao_driver_info_list(&count);

	for(unsigned d = 0; d < count; d++ ) {
		ao_info* driver = info[d];
		std::cout << d << ": [" << driver->type << "] - " << driver->short_name << std::endl;
	}

	generate();
	play();

	ao_close(device);
	ao_shutdown();
	return 0;
}