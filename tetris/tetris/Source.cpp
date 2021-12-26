#include <iostream>
using namespace std;

#include <Windows.h>
#include <thread>
#include <vector>

// -- function declarations --
int main();
std::size_t rotate(std::size_t x, std::size_t y, int r);
bool doesPieceFit(unsigned char tetrominoIndex, int rotation, std::size_t topLeftX, std::size_t topLeftY);

// -- variables --
// tetromino shapes
wstring tetrominos[7];
// dimensions of playing field
unsigned int playingFieldWidth{12};
unsigned int playingFieldHeight{18};
// playing field
unsigned char *playingField{nullptr};
// screen dimensions
unsigned int screenWidth{80};
unsigned int screenHeight{30};
// possible characters for displays 
wstring displayCharacters{L" ABCDEFG=#"};
unsigned char shift{ 2 };



// for given x and y and rotatation, return appropriate index in a tetrominos string
std::size_t rotate(std::size_t x, std::size_t y, int r) {
	switch (r % 4) {
		// no rotation
		case 0: return y * 4 + x;
		// 90 deg rotation cw
		case 1: return 12 + y - (x * 4);
		// 180 deg cw
		case 2: return 15 - (y * 4) - x;
		// 270 deg cw
		case 3: return 3 - y + (x * 4);
	}
	return 0;
}

// given the type of tetromino piece, rotation index, and coorindates of the top left square of the tetromino, return if it fits on the field 
bool doesPieceFit(unsigned char tetrominoIndex, int rotation, std::size_t topLeftX, std::size_t topLeftY) {
	for (std::size_t i{}; i < 4; ++i) {
		for (std::size_t j{}; j < 4; ++j) {
			// index of cell in tetromino after rotation
			std::size_t indexInTetromino{ rotate(j, i, rotation) };
			// index of cell in the field 
			std::size_t indexInField{(topLeftY + i) * playingFieldWidth + (topLeftX + j)};
			// for any cell in the tetromino, if a physical cell in the tetromino coincides with a physical cell in the
			// playing field, return false
			// for any cell in the tetromino, if the cell is physical and is out of bounds of the playing field, return false
			if (topLeftY + i >= 0 && topLeftY + i < playingFieldHeight) {
				if (topLeftX + j >= 0 && topLeftX + j < playingFieldWidth) {
					if (tetrominos[tetrominoIndex][indexInTetromino] != L'.' && playingField[indexInField] != 0) {
						return false;
					}
				} else if (tetrominos[tetrominoIndex][indexInTetromino] != L'.') {
					return false;
				}
			} else if (tetrominos[tetrominoIndex][indexInTetromino] != L'.') {
				return false;
			}
		}
	}
	return true;
}

int main() {

	// Initialize game assets
	tetrominos[0].append(L"..E.");
	tetrominos[0].append(L"..E.");
	tetrominos[0].append(L"..E.");
	tetrominos[0].append(L"..E.");

	tetrominos[1].append(L"..E.");
	tetrominos[1].append(L".EE.");
	tetrominos[1].append(L".E..");
	tetrominos[1].append(L"....");

	tetrominos[2].append(L".E..");
	tetrominos[2].append(L".EE.");
	tetrominos[2].append(L"..E.");
	tetrominos[2].append(L"....");

	tetrominos[3].append(L"....");
	tetrominos[3].append(L".EE.");
	tetrominos[3].append(L".EE.");
	tetrominos[3].append(L"....");

	tetrominos[4].append(L"..E.");
	tetrominos[4].append(L".EE.");
	tetrominos[4].append(L"..E.");
	tetrominos[4].append(L"....");

	tetrominos[5].append(L"....");
	tetrominos[5].append(L".EE.");
	tetrominos[5].append(L"..E.");
	tetrominos[5].append(L"..E.");

	tetrominos[6].append(L"....");
	tetrominos[6].append(L".EE.");
	tetrominos[6].append(L".E..");
	tetrominos[6].append(L".E..");

	// initialize playing field
	playingField = new unsigned char[playingFieldWidth*playingFieldHeight];

	// create the border on playing field
	for (std::size_t i{}; i < playingFieldHeight; ++i) {
		for (std::size_t j{}; j < playingFieldWidth; ++j) {
			playingField[i * playingFieldWidth + j] = (j == 0 || j == playingFieldWidth - 1 || i == playingFieldHeight - 1) ? 9 : 0;

		}
	}

	// create screen buffer
	wchar_t *screen = new wchar_t[screenWidth*screenHeight];
	for (std::size_t i = 0; i < screenWidth*screenHeight; i++) {
		screen[i] = L' ';
	}
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// -- game logic variables -- 
	bool gameOver{false};
	unsigned char currentTetromino{1};
	int currentRotation{0};
	std::size_t currentTopLeftX{playingFieldWidth / 2};
	std::size_t currentTopLeftY{0};
	// array of whether each key is pressed
	bool keysArray[5]{};
	// to fix rotate key being too reactive
	bool rotateHold{false};
	// how long until the piece is forced down (this simulates "gravity")
	unsigned int countForForceDown{15};
	unsigned int forceDownCounter{0};
	bool forceDown{false};
	// store lines that are destroyed
	vector<std::size_t> destroyed_lines{};
	// number of tetrimons placed 
	std::size_t pieceCount{0};
	const std::size_t NUM_PIECES_FOR_DIFFICULTY_INCREASE{10};
	// score
	std::size_t score{0};
	const std::size_t SCORE_EACH_PIECE{25};
	const std::size_t SCORE_EACH_LINE{100};


	// main game loop
	while (!gameOver) {

		// -- timing --
		this_thread::sleep_for(50ms);
		++forceDownCounter;
		if (forceDownCounter == countForForceDown) {
			forceDown = true;
		}

		std::cout << forceDown;

		// -- input --
		for (unsigned char k{}; k < 5; ++k) {                         
			// \x25 is left arrow 
			// \x27 isd right arrow
			// \x28 is down arrow
			keysArray[k] = ((0x8000 & GetAsyncKeyState((unsigned char)("\x25\x27\x28Z\x20"[k]))) != 0);
		}

		// -- logic --
		// left arrow
		if (keysArray[0]) {
			if (doesPieceFit(currentTetromino, currentRotation, currentTopLeftX - 1, currentTopLeftY)) {
				currentTopLeftX -= 1;
			}
		}
		// right arrow
		if (keysArray[1]) {
			if (doesPieceFit(currentTetromino, currentRotation, currentTopLeftX + 1, currentTopLeftY)) {
				currentTopLeftX += 1;
			}
		}
		// down arrow
		if (keysArray[2]) {
			if (doesPieceFit(currentTetromino, currentRotation, currentTopLeftX, currentTopLeftY + 1)) {
				currentTopLeftY += 1;
			}
		}
		// Z key
		if (keysArray[3]) {
			if (doesPieceFit(currentTetromino, currentRotation + 1, currentTopLeftX, currentTopLeftY + 1) && !rotateHold) {
				currentRotation += 1; 
			}
			rotateHold = true;
		} else {
			rotateHold = false;
		}
	
		if (forceDown) {
			// if there is still space for the tetromino to go down
			if (doesPieceFit(currentTetromino, currentRotation, currentTopLeftX, currentTopLeftY + 1)) {
				++currentTopLeftY;
			} else { 
				// lock the current tetromino into the field
				for (std::size_t i{}; i < 4; ++i) {
					for (std::size_t j{}; j < 4; ++j) {
						if (tetrominos[currentTetromino][rotate(j, i, currentRotation)] == L'E') {
							// set cell part of tetromino to appropriate index for getting character from displayCharacters string 
							playingField[(currentTopLeftY + i) * playingFieldWidth + (currentTopLeftX + j)] = currentTetromino + 1;
						}
					}
				}

				// increment pieces placed
				++pieceCount;
				if (pieceCount % NUM_PIECES_FOR_DIFFICULTY_INCREASE == 0 && pieceCount > NUM_PIECES_FOR_DIFFICULTY_INCREASE) {
					--countForForceDown;
				}

				// check for any lines at the bottom
				for (std::size_t i{}; i < 4; ++i) {
					// check the lines that are above the border (we don't want to check the empty lines coinciding with the border)
					if (currentTopLeftY + i < playingFieldHeight - 1) {
						bool fullLine{ true };
						// note the borders take up one cell
						for (std::size_t k{ 1 }; k < playingFieldWidth - 1; ++k) {
							if (playingField[(currentTopLeftY + i) * playingFieldWidth + k] == 0) {
								fullLine = false;
								break;
							}
						}
						// set line to =
						if (fullLine) {
							for (std::size_t j{1}; j < playingFieldWidth - 1; ++j) {
								playingField[(currentTopLeftY + i) * playingFieldWidth + j] = 8 ;
							}
							destroyed_lines.push_back(currentTopLeftY + i);
						}
					}
				}

				// update score
				score += SCORE_EACH_PIECE;
				if (!destroyed_lines.empty()) {
					score += (1 << destroyed_lines.size()) * SCORE_EACH_LINE;
				}


				// set the next piece
				currentTopLeftX = playingFieldWidth / 2;
				currentTopLeftY = 0;
				currentRotation = 0;
				currentTetromino = rand() % 7;
				
				// if the piece doesn't fit:
				gameOver = !doesPieceFit(currentTetromino, currentRotation, currentTopLeftX, currentTopLeftY);
			}

			forceDownCounter = 0;
			forceDown = false;
		}

		// -- render output -- 

		// draw the field
		for (std::size_t i{}; i < playingFieldHeight; ++i) {
			for (std::size_t j{}; j < playingFieldWidth; ++j) {
				// set characters on the screen
				// note we're shifting the playing field for aesthetic reasons
				screen[(i + shift) * screenWidth + j + shift] = displayCharacters[playingField[playingFieldWidth * i + j]];
			}
		}

		// draw the current tetromino piece
		for (std::size_t i{}; i < 4; ++i) {
			for (std::size_t j{}; j < 4; ++j) {
				if (tetrominos[currentTetromino][rotate(j, i, currentRotation)] == L'E') {
					screen[(currentTopLeftY + i + shift) * screenWidth + (currentTopLeftX + j + shift)] = currentTetromino + L'A';
				}
			}
		}

		// draw the score
		swprintf_s(&screen[2 * screenWidth + playingFieldWidth + 6], 16, L"SCORE: %8d", score);

		// if there are lines to be destroyed, 
		if (!destroyed_lines.empty()) {
			// display the frame for drawing the = signs
			WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			// for each of the destroyed lines, move the lines above down one level
			for (std::size_t i{}; i < destroyed_lines.size(); ++i) {
				for (std::size_t y{destroyed_lines[i]}; y > 0; --y) {
					for (std::size_t x{ 1 }; x < playingFieldWidth - 1; ++x) {
						playingField[(y * playingFieldWidth) + x] = playingField[((y - 1)*playingFieldWidth) + x];
					}
				}
			}

			destroyed_lines.clear();
		}

		// display the frame
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);

	}

	// clean up console
	CloseHandle(hConsole);
	system("cls");
	std::cout << "GAME OVER!" << std::endl;
	std::cout << "SCORE: " << score << std::endl; 
	system("pause");



	return 0;

}