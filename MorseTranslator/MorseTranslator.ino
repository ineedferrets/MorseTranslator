#include <ArduinoSTL.h>
#include <eeprom.h>
#include "LiquidCrystal.h"
#include "BinaryTree.h"

// initialise lcd library (does not connect to lcd)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int lcdPin = 6;

// store which pins are for input button, piezo buzzer. and delete button
const int inputButtPin = 7;
const int buzzPin = 8;
const int delButtPin = 13;

// divisor helps prevent stack overflow for measuring millis(), timeUnit is the length of a dot and space length
// is number of timeUnits until new character
const float divisor = 10000.0f;
const float timeUnit = 80.0f;
const int spaceLength = 8;

// values for checking when buttons are being pressed and were last pressed (only accessed by their relative
// functions)
bool inputPrevPressed = false;
bool delPrevPressed = false;
float lastChange = 0;

// storing current morse code inputted
std::vector<char> morseCode = std::vector<char>();

// storing the translated characters
std::vector<char> translation = std::vector<char>();

// custom structure for storing all morse values
BinaryTree morseTree;

// reference to functions
void initialiseLCDScreen();
void initialisePins();
void buildMorseTree();
bool checkInputButtPressed(std::vector<char> &morseInput);
void morseToChar(std::vector<char> morseInput, std::vector<char> &translationInput);
bool checkDelButtPressed(std::vector<char> &inputTranslation);
void resetMorseInput(std::vector<char> &morseInput);
void printScreen(std::vector<char> morseInput, std::vector<char> translationInput, LiquidCrystal &lcdInput);



void setup() {
	initialiseLCDScreen();
	initialisePins();
	buildMorseTree();
	Serial.begin(9600);
}

void loop() {
	if (checkInputButtPressed(morseCode) && morseCode.back() != ' ' && morseCode.back() != NULL && morseCode.size() <= 4) {
		printScreen(morseCode, translation, lcd);
	}
	else if (morseCode.size() == 4 || morseCode.back() == ' ') {
		morseToChar(morseCode, translation);
		resetMorseInput(morseCode);
		printScreen(morseCode, translation, lcd);
	}
	else if(checkDelButtPressed(translation)) {
		printScreen(morseCode, translation, lcd);
	}
	delay(10);
}

// setup connection with screen
void initialiseLCDScreen() {
	lcd.begin(16, 2);
	lcd.cursor();
	pinMode(lcdPin, OUTPUT);
	digitalWrite(lcdPin, HIGH);
}

// setup which pins are input and output
void initialisePins() {
	pinMode(buzzPin, OUTPUT);
	pinMode(inputButtPin, INPUT);
	pinMode(delButtPin, INPUT);
}

// build the binary tree of all morse values
void buildMorseTree() {
	node *root = new node(NULL);
	morseTree = BinaryTree(root);

	// generate all nodes from left to right
	{
		node *letterH = new node('h');
		node *letterV = new node('v');

		node *letterS = new node('s');
		letterS->left = letterH;
		letterS->right = letterV;

		node *letterF = new node('f');

		node *letterU = new node('u');
		letterU->left = letterF;

		node *letterI = new node('i');
		letterI->left = letterS;
		letterI->right = letterU;

		node *letterL = new node('l');

		node *letterR = new node('r');
		letterR->left = letterL;

		node *letterP = new node('p');
		node *letterJ = new node('j');

		node *letterW = new node('w');
		letterW->left = letterP;
		letterW->right = letterJ;

		node *letterA = new node('a');
		letterA->left = letterR;
		letterA->right = letterW;

		node *letterE = new node('e');
		letterE->left = letterI;
		letterE->right = letterA;

		node *letterB = new node('b');
		node *letterX = new node('x');

		node *letterD = new node('d');
		letterD->left = letterB;
		letterD->right = letterX;

		node *letterC = new node('c');
		node *letterY = new node('y');

		node *letterK = new node('k');
		letterK->left = letterC;
		letterK->right = letterY;

		node *letterN = new node('n');
		letterN->left = letterD;
		letterN->right = letterK;

		node *letterZ = new node('z');
		node *letterQ = new node('q');

		node *letterG = new node('g');
		letterG->left = letterZ;
		letterG->right = letterQ;

		node *letterO = new node('o');

		node *letterM = new node('m');
		letterM->left = letterG;
		letterM->right = letterO;

		node *letterT = new node('t');
		letterT->left = letterN;
		letterT->right = letterM;

		root->left = letterE;
		root->right = letterT;
	}
}

// continuously checks whether the input button is pressed/not pressed and for how long
bool checkInputButtPressed(std::vector<char> &inputMorse) {
	bool inputNowPressed = digitalRead(inputButtPin);
	bool inputReturn = false;

	if (inputPrevPressed) {
		if (!inputNowPressed) {
			noTone(buzzPin);
			if (millis() - lastChange*divisor <= 3 * timeUnit)
				inputMorse.push_back('.');
			else
				inputMorse.push_back('-');
			inputPrevPressed = false;
			lastChange = millis() / divisor;
			inputReturn = true;
		}
	}
	else if (!inputPrevPressed) {
		if (!inputNowPressed) {

			if (millis() - lastChange*divisor >= spaceLength * timeUnit && inputMorse.size() != 0) {
				inputMorse.push_back(' ');
				inputReturn = true;
			}
		}
		else if (inputNowPressed) {
			tone(buzzPin, 340);
			inputPrevPressed = true;
			lastChange = millis() / divisor;
		}
	}
	return inputReturn;
}

// uses the combination of dots and dashes to navigate the binary tree (dot is left, dash is right)
void morseToChar(std::vector<char> morseInput, std::vector<char> &translationInput) {
	node *morseLetter = morseTree.getRoot();

	for (int i = 0; i < morseInput.size(); i++) {
		char dotDash = morseInput[i];
		switch (dotDash) {
		case '.':
			if (morseLetter->left == NULL)
				translationInput.push_back('?');
			morseLetter = morseLetter->left;
			break;

		case '-':
			if (morseLetter->right == NULL)
				translationInput.push_back('?');
			morseLetter = morseLetter->right;
			break;

		default:
			break;
		}
	}
	return translationInput.push_back(morseLetter->key_value);
}

// checks whether the delete button is pressed/not pressed and how long for
bool checkDelButtPressed(std::vector<char> &inputTranslation) {
	bool delButtPressed = digitalRead(delButtPin);

	if (delButtPressed && !delPrevPressed) {
		inputTranslation.pop_back();
		delPrevPressed = true;
		tone(buzzPin, 50);
		return true;
	}
	if (!delButtPressed && delPrevPressed) {
		delPrevPressed = false;
		noTone(buzzPin);
	}
	return false;
}

// removes morse input on top line
void resetMorseInput(std::vector<char> &morseInput) {
	morseInput = std::vector<char>();
}

// clears the screen and prints new page
void printScreen(std::vector<char> morseInput, std::vector<char> translationInput, LiquidCrystal &lcdInput) {
	lcdInput.noCursor();
	lcdInput.clear();

	int textEndPos = translationInput.size();
	lcdInput.setCursor(0, 1);
	int screenDifference = 0;
	if (textEndPos > 16) {
		screenDifference = textEndPos - 16;
		for (int i = screenDifference; i < textEndPos; i++)
			lcdInput.print(translationInput[i]);
	}
	else {
		for (int i = 0; i < textEndPos; i++)
			lcdInput.print(translationInput[i]);
	}

	lcdInput.setCursor(0, 0);
	for (int i = 0; i < morseInput.size(); i++)
		lcdInput.print(morseInput[i]);

	lcdInput.cursor();
}