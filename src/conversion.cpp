#include "conversion.h"

int decodeVLQ(const vector<byte>& input) {
	int ret = 0;
	for (int i = input.size() - 1, j = 0; i >= 0; i--, j++) {
		if (input[i] > 127) {
			if (j != 0)
				ret += (input[i] - 127) * 128 * j;
			else
				ret += (input[i] - 127);
		}
		else {
			if (j != 0)
				ret += input[i] * 128 * j;
			else
				ret += input[i];
			return ret;
		}
	}
	return ret;
}
void encodeVLQ(int value, vector<byte>& output) {
	while (value > 127) {
		output.push_back(((byte)(value & 127)) | 128);
		value >>= 7;
	}
	output.push_back(((byte)value) & 127);
}
bool atTrack(const vector<byte>::iterator iter) {
	char temp[4] = { (char)*(iter - 4), (char)*(iter - 3), (char)*(iter - 2), (char)*(iter - 1) };
	if (temp[0] == 'M' && temp[1] == 'T' && temp[2] == 'r' && temp[3] == 'k')
		return true;
	return false;
}
string generateName(MTRand_int32& rng) {
	// generates a random song name of the format "<adjective>_<noun>_in_<key signature>"
	vector<string> adj, noun, key;
	// open files
	ifstream f_adj("../text/adjective.txt");
	ifstream f_noun("../text/noun.txt");
	ifstream f_key("../text/key.txt");
	// read inpput
	string input;
	while (f_adj >> input) { adj.push_back(input); }
	while (f_noun >> input) { noun.push_back(input); }
	while (f_key >> input) { key.push_back(input); }
	// close files
	f_adj.close();
	f_noun.close();
	f_key.close();
	// randomly generate file name
	string name = adj[rng() % adj.size()] + "_" + noun[rng() % noun.size()] + "_in_" + key[rng() % key.size()] + ".txt";
	return name;
}

PianoRoll::PianoRoll(int given_ticks, const vector<string>& given_notes) {
	ticks_per_16th = given_ticks;
	notes = vector<string>(given_notes);
}
PianoRoll::PianoRoll(const PianoRoll& other) {
	ticks_per_16th = other.ticks_per_16th;
	notes = vector<string>(other.notes);
}

Converter::Converter() {
	// initialize conversion maps
	toText = map<int, string>();
	toMIDI = map<string, int>();
	// build conversion maps
	for (uint i = 0; i < 128; i++) {
		// find octave and note indexes
		int octave = i / 12;
		int note = i % 12;
		// populate maps with number representing octave and a symbol representing a note
		toText[i] = to_string(octave) + char(note + 33);
		toMIDI[to_string(octave) + char(note + 33)] = i;
		string n = "";
		if (note == 0) n = "C"; else if (note == 1) n = "C#"; else if (note == 2) n = "D";
		else if (note == 3) n = "D#"; else if (note == 4) n = "E"; else if (note == 5) n = "F";
		else if (note == 6) n = "F#"; else if (note == 7) n = "G"; else if (note == 8) n = "G#";
		else if (note == 9) n = "A"; else if (note == 10) n = "A#"; else n = "B";

		readableNotes[to_string(octave) + char(note + 33)] = n + to_string(octave);
	}

}
PianoRoll Converter::ConvertToText(const string& midifile) {
	// open midi file in binary mode
	ifstream midi;
	midi.open("../input/" + midifile, ios::binary);
	if (!midi)
		return PianoRoll();
	// read byte-by-byte and save to vector
	vector<byte> data;
	byte* buff = new byte;
	while (!midi.eof()) {
		midi.read((char*)buff, 1);
		data.push_back(*buff);
	}
	midi.close();
	delete buff;
	cout << "    MIDI file \"" << midifile << "\" found... ";
	// determine file format, number of tracks and note timing
	vector<byte>::iterator
		iter1 = data.begin() + 8,
		iter2 = data.begin() + 10,
		iter3 = data.begin() + 12,
		iter4 = data.begin() + 14;
	int format = decodeVLQ(vector<byte>(iter1, iter2));
	int ntrks = decodeVLQ(vector<byte>(iter2, iter3));
	int timing = decodeVLQ(vector<byte>(iter3, iter4)) / 4;
	// begin parsing note data
	vector<byte>::iterator iter = iter4;
	// data containers
	vector<string> notes;
	while (iter != data.end()) {
		if (atTrack(iter)) {
			// get length
			iter += 4;
			int trackLength = 0;
			if (iter != data.end()) {
				int temp[] = { int(*(iter - 4)), int(*(iter - 3)), int(*(iter - 2)), int(*(iter - 1)) };
				trackLength = (temp[0] << 24) | (temp[1] << 16) | (temp[2] << 8) | temp[3];
			}
		// loop through track
			for (int i = 0; i < trackLength || iter != data.end(); i++, iter++) {
				// save each note when it starts
				if (int(*iter) == 144) {
					if (toText.find(*(iter + 1)) != toText.end()) {
						string formattedNote = toText.find(*(iter + 1))->second;
						notes.push_back(formattedNote);
					}
				}
			}
		}
		else
			iter++;
	}
	cout << "Read Successfully." << endl;
	return PianoRoll(timing, notes);
}
string Converter::ConvertToMidi(const PianoRoll& data) {
	ofstream output;
	string name;
	// find first free filename
	for (int i = 0; i < 1024; i++) {
		name = generateName(rng);
		ifstream test("../output/" + name);
		if (i == 1023) {
			cout << "    NO FREE FILENAMES FOUND." << endl;
			return "";
		}
		if (!test) {
			test.close();
			break;
		}
		test.close();
	}
	output.open("../output/" + name);
	// write pretty notes to file
	for (int i = 0; i < data.getNotes().size(); i++) {
		output << readableNotes.find(data.getNotes()[i])->second;
		if (i % 10 == 0)
			output << endl;
		else
			output << " ";
	}
	cout << "    Masterpiece \"" << name << "\" created." << endl;
	return name;
}
