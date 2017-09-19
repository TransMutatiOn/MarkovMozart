#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdint>
#include <ctime>
#include <vector>
#include "mtrand.h"
using namespace std;
typedef unsigned int uint;
typedef uint8_t byte;

extern MTRand_int32 rng;

int decodeVLQ(const vector<uint8_t>& input);
void encodeVLQ(int value, vector<uint8_t>& output);
bool atTrack(const vector<byte>::iterator iter);
string generateName(MTRand_int32& rng);

class PianoRoll {
public:
	PianoRoll() { ticks_per_16th = -1; }
	PianoRoll(int given_ticks, const vector<string>& given_notes);
	PianoRoll(const PianoRoll& other);
	PianoRoll operator=(const PianoRoll other) { return PianoRoll(other); }
	const vector<string>& getNotes() const { return notes; }
	const int getTicks() const { return ticks_per_16th; }
private:
	int ticks_per_16th;
	vector<string> notes;
};

class Converter {
public:
	Converter();
	const string& operator [](int value) const { return toText.find(value)->second; }
	const int operator [](string value) const { return toMIDI.find(value)->second; }
	PianoRoll ConvertToText(const string& midifile);
	string ConvertToMidi(const PianoRoll& data);
private:
	map<int, string> toText;
	map<string, int> toMIDI;
	map<string, string> readableNotes;
};