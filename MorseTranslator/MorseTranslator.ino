#include <ArduinoSTL.h>
#include <LiquidCrystal.h>
#include "BinaryTree.h"

// initialise lcd library (does not connect to lcd)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// store which pins are for input button, piezo buzzer. and delete button
const int inputButtPin = 7;
const int buzzPin = 8;
const int delButtPin = 13;

// divisor helps prevent stack overflow for measuring millis(), timeUnit is the length of a dot and space length
// is number of timeUnits until new character
const float divisor = 10000.0f;
const float timeUnit = 80.0f;
const int spaceLength = 8;

// values for checking when buttons are being pressed and were last pressed
bool inputPrevPressed = false;
bool delPrevPressed = false;
float lastChange = 0;

// tracking cursor and screen position
int cursorPosition[2] = { 0, 0 };
int screenColRightPosition = 16;

// storing current morse code inputted
char morseCode[4];
int morseCharNum = 0;

// storing the translated characters
std::vector<char> translation = std::vector<char>();

// custom structure for storing all morse values
BinaryTree morseTree;

void setup() {
	initialiseLCDScreen();
	initialisePins();
	buildMorseTree();
	Serial.begin(9600);
}

void loop() {
	char dotDash = checkInputButtPressed();

	if (dotDash != NULL && morseCharNum < 4) {
		lcd.print(dotDash);
		morseCode[morseCharNum] = dotDash;
		morseCharNum++;
	}
	else if (morseCharNum == 4 || dotDash == ' ') {
		char morseChar = morseToChar();
		translation.push_back(morseChar);

		cursorPosition[0] = translation.size() - 1;
		cursorPosition[1] = 1;
		lcd.setCursor(cursorPosition[0], cursorPosition[1]);
		lcd.write(morseChar);

		resetMorseInput();
	}
	else
		checkDelButtonPressed();
	
	checkScreenEdge();

	delay(10);
}

// setup connection with screen
void initialiseLCDScreen() {
	lcd.begin(16, 2);
	lcd.cursor();
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
char checkInputButtPressed() {
	char output = NULL;
	bool inputNowPressed = digitalRead(inputButtPin);

	if (inputPrevPressed) {
		if (!inputNowPressed) {
			noTone(buzzPin);
			if (millis() - lastChange*divisor <= 3 * timeUnit)
				output = '.';
			else
				output = '-';
			cursorPosition[0] += 1;
			inputPrevPressed = false;
			lastChange = millis() / divisor;
		}
	}
	else if (!inputPrevPressed) {
		if (!inputNowPressed) {

			if (millis() - lastChange*divisor >= spaceLength * timeUnit && morseCharNum != 0) {
				output = ' ';
			}
		}
		else if (inputNowPressed) {
			tone(buzzPin, 340);
			inputPrevPressed = true;
			lastChange = millis() / divisor;
		}
	}

	return output;
}

// uses the combination of dots and dashes to navigate the binary tree (dot is left, dash is right)
char morseToChar() {
	node *morseLetter = morseTree.getRoot();

	for (int i = 0; i < sizeof(morseCode); i++) {
		char dotDash = morseCode[i];
		if (dotDash == ' ' || dotDash == NULL)
			return morseLetter->key_value;
		else {
			switch (dotDash) {
			case '.':
				if (morseLetter->left == NULL)
					return '?';
				morseLetter = morseLetter->left;
				break;

			case '-':
				if (morseLetter->right == NULL)
					return '?';
				morseLetter = morseLetter->right;
				break;

			default:
				break;
			}
		}
	}

	return morseLetter->key_value;
}

// checks whether the delete button is pressed/not pressed and how long for
void checkDelButtonPressed() {
	bool delButtPressed = digitalRead(delButtPin);

	if (delButtPressed && !delPrevPressed) {
		int endChar = translation.size() - 1;
		cursorPosition[0] = std::max(0, endChar);
		cursorPosition[1] = 1;
		lcd.setCursor(cursorPosition[0], cursorPosition[1]);
		lcd.print(" ");
		resetCursor();
		translation.pop_back();
		delPrevPressed = true;
		tone(buzzPin, 60);
	}

	if (!delButtPressed && delPrevPressed) {
		delPrevPressed = false;
		noTone(buzzPin);
	}
}

// check whether the translation has gone beyond the screen edge and moves it accordingly
void checkScreenEdge() {
	if (translation.size() > screenColRightPosition) {
		screenColRightPosition += 1;
		lcd.scrollDisplayLeft();
		resetCursor();
	}
	else if (translation.size() < screenColRightPosition && screenColRightPosition > 16) {
		screenColRightPosition -= 1;
		lcd.scrollDisplayRight();
		resetCursor();
	}
}

// set cursor to the top left of the screen
void resetCursor() {
	cursorPosition[0] = screenColRightPosition - 16;
	cursorPosition[1] = 0;
	lcd.setCursor(cursorPosition[0], cursorPosition[1]);
}

// removes morse input on top line
void resetMorseInput() {
	resetCursor();
	cursorPosition[0] += morseCharNum;
	cursorPosition[1] = 0;

	for (int i = 0; i < morseCharNum; i++) {
		cursorPosition[0] = std::max(0, cursorPosition[0] - 1);
		lcd.setCursor(cursorPosition[0], cursorPosition[1]);
		lcd.print(" ");
		lcd.setCursor(cursorPosition[0], cursorPosition[1]);
		morseCode[i] = NULL;
	}
	morseCharNum = 0;
}
