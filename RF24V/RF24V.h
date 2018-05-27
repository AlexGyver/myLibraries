#ifndef __RF24V_H__
#define __RF24V_H__
class RF24;
class RF24V{
public:
	RF24V(RF24& _radio, byte radioNum);
	void begin();
	void volume(bool upDn);
	void setVolume(char vol);
	void transfer();
	void reader();
	void broadcast(byte radioID);
    uint64_t getAddress(byte addressNo);
private:
	RF24& radio;
	void timerStart();};
void TX();
void RX();
#endif
