#include "conversion.h"

MTRand_int32 rng(time(NULL));

// typedef maps
typedef map<string, int> map1d;
typedef map<string, map<string, int> > map2d;
typedef map<string, map<string, map<string, int> > > map3d;
//typedef iterators
typedef map<string, int>::const_iterator cit1;
typedef map2d::const_iterator cit2;
typedef map3d::const_iterator cit3;

// read all MIDI files
void readAllFiles(Converter& converter, vector<PianoRoll>& allPianoRolls) {
	cout << "Reading MIDI Files." << endl;
	// read all MIDI files into a vector of PianoRolls
	int i = 1, fileCount = 0;
	while (true) {
		string file = "input" + to_string(i) + ".mid";
		PianoRoll input = converter.ConvertToText(file);
		if (input.getTicks() == -1)
			break;
		else {
			fileCount++;
			allPianoRolls.push_back(input);
		}
		i++;
	}
	cout << fileCount << " Files Read Successfully." << endl;
}
// create frequency map from vector of piano rolls
void createNoteMap(map3d& noteData, const vector<PianoRoll>& allPianoRolls) {
	// loop through all piano rolls
	for (uint i = 0; i < allPianoRolls.size(); i++) {
		vector<string> currNotes = allPianoRolls[i].getNotes();
		// loop through notes in this roll
		for (uint j = 2; j < currNotes.size(); j++) {
			// increment this occurance
			noteData[currNotes[j - 2]][currNotes[j - 1]][currNotes[j]]++;
		}
		// THIS SKIPS LAST TWO NOTES IN EACH PIANO ROLL
	}
	cout << "Note Frequency Map Created." << endl;
}
// generate a song based on the user's requested length
bool generateSong(int length, vector<string>& songNotes, const map3d& noteData) {

	// initialize variables
	string curr = "", next1 = "", next2 = "";
	cit3 iter1; cit2 iter2; cit1 iter3;
	vector<string> storage;

	// randomly select starting note WITHOUT weighting based on occurance
	for (iter1 = noteData.begin(); iter1 != noteData.end(); iter1++)
		storage.push_back(iter1->first);
	curr = storage[rng() % storage.size()];
	storage.clear();

	// randomly select next1 and next2 (weighted)
	iter1 = noteData.find(curr);
	if (iter1 == noteData.end()) return false;
	// add all notes from 2nd dimension to storage
	for (iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		for (iter3 = iter2->second.begin(); iter3 != iter2->second.end(); iter3++)
			for (int i = 0; i < iter3->second; i++)
				storage.push_back(iter2->first);
	// randomly select next1
	next1 = storage[rng() % storage.size()];
	storage.clear();
	// find next1 in map2d of curr again
	iter2 = iter1->second.find(next1);
	if (iter2 == iter1->second.end()) return false;
	// add all notes from 1st dimension to storage
	for (iter3 = iter2->second.begin(); iter3 != iter2->second.end(); iter3++)
		for (int i = 0; i < iter3->second; i++)
			storage.push_back(iter3->first);
	// randomly select next2
	next2 = storage[rng() % storage.size()];
	storage.clear();

	// add curr to song and advance to next note
	songNotes.push_back(curr);
	curr = next1;
	next1 = next2;

	// BEGIN LOOP for as many notes as the user requested
	for (int noteCount = 1; noteCount < length; noteCount++) {
		// add current note to song
		songNotes.push_back(curr);
		// find current note in map3d
		iter1 = noteData.find(curr);
		if (iter1 == noteData.end()) return false;
		// find next1 in this map2d
		iter2 = iter1->second.find(next1);
		if (iter2 == iter1->second.end()) return false;
		// generate vector for random choice for next2
		for (iter3 = iter2->second.begin(); iter3 != iter2->second.end(); iter3++)
			for (int i = 0; i < iter3->second; i++)
				storage.push_back(iter3->first);
		// randomly choose next2
		next2 = storage[rng() % storage.size()];
		storage.clear();
		// advance to next word
		curr = next1;
		next1 = next2;
	}
	return true;
}

int main() {

	// initialize containers and conversion object
	Converter converter;
	vector<PianoRoll> allPianoRolls;
	map3d noteData;
	string userInput;

	// create note map
	readAllFiles(converter, allPianoRolls);
	createNoteMap(noteData, allPianoRolls);

	// loop preparation
	cout << endl << "User input required. Available commands:";
	cout << endl << "   \"generate <length>\" to make song" << endl << "   \"quit\" to quit." << endl << "  ~ ";
	
	// main loop
	while (cin >> userInput) {
		// generate command
		if (userInput == "generate") {
			int length;
			vector<string> songNotes;
			// get length of song to generate
			cin >> length;
			cout << "    Generating song of length " << length << "." << endl;
			// generate song and create PianoRoll object using the result
			bool success = generateSong(length, songNotes, noteData);
			if (!success)
				cout << "    Not entirely successful. Generated song of length " << songNotes.size() << ". Continuing." << endl;
			PianoRoll song(0, songNotes);
			// send to MIDI file using ConvertToMidi
			converter.ConvertToMidi(song);
		}
		// quit command
		else if (userInput == "quit") {
			cout << "    Quitting." << endl;
			break;
		}
		// error handling
		else 
			cout << "    Unrecognized command \"" << userInput << "\"." << endl;
		cout << "  ~ ";
	}
	return 0;
}

