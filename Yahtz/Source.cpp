#include <iostream>
#include <ctime>     // For setting srand seed
#include <string> 
#include <Windows.h> // Sleep()
#include <iomanip>   // setw(), setfill(), ...
#include <algorithm> // sort()
#include <fstream>   // High scores

using namespace std;

struct highScore
{
	string HS_Name;
	string HS_Date;
	int HS_Score;
} HS_Data[24], HS_Current[4];

int dice[5],        // Individual dice
	player = 0,     // Current player (player + 1) = current player
	numPlayers = 0, // Number of players for current game. 0 = 1 player. (numPlayers + 1) = Amount of players (4 max)
	dieHeldCount = 0,         // How many dice were held
	roundNum = 1;

int ace[4] = { 0, 0, 0, 0 }, // Global integers for scorekeeping
	two[4] = { 0, 0, 0, 0 },
	three[4] = { 0, 0, 0, 0 },
	four[4] = { 0, 0, 0, 0 },
	five[4] = { 0, 0, 0, 0 },
	six[4] = { 0, 0, 0, 0 },
	chance[4] = { 0, 0, 0, 0 },
	kind_3[4] = { 0, 0, 0, 0 },
	kind_4[4] = { 0, 0, 0, 0 },
	upperTotal[4] = { 0, 0, 0, 0 },
	upperTotalBon[4] = { 0, 0, 0, 0 },
	lowerTotal[4] = { 0, 0, 0, 0 },
	grandTotal[4] = { 0, 0, 0, 0 },
    bonusTotal[4] = { 0, 0, 0, 0 };

bool bonus[4][3] = { 0, 0, 0,   // P1 // Global boolean variables for 
					 0, 0, 0,   // P2 // keeping track of bonus yahtzees
					 0, 0, 0,   // P3
					 0, 0, 0 }, // P4
	 endGame = 0,			    // Changes state when all players have played all turns
	 isFirstRoll = 0,           //
	 isHighScore[4] = { 0, 0, 0, 0 }; // Flags for wheter or not [Player] achieved a high score

signed char smStr[4] = { 0, 0, 0, 0 }, // Global chars for tri-state scorekeeping
			lgStr[4] = { 0, 0, 0, 0 },
			yahtz[4] = { 0, 0, 0, 0 },
			fHouse[4] = { 0, 0, 0, 0 };

string choose = "",
	   playerName[4] = { "", "", "", "" };

// Not my code -- can't figure out my own method for maximizing window
void MaximizeWindow()
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	SMALL_RECT rc;
	rc.Left = rc.Top = 0;
	rc.Right = (short)(min(info.dwMaximumWindowSize.X, info.dwSize.X) - 1);
	rc.Bottom = (short)(min(info.dwMaximumWindowSize.Y, info.dwSize.Y) - 1);
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), true, &rc);
}

// Function for if or not 35 point bonus
string isBonus_63(int utValue)
{
	if (utValue >= 63 && endGame) return "   YES!!   ";
	else if (utValue < 63 && endGame) return "   NO...   ";
	else return "           ";
}

// Calculates total scores
void calcScore(void)
{
	// bonusTotal separate for output to scoresheet
	bonusTotal[player] = (bonus[player][0] ? 100 : 0)  // If first bonus exists, return 100, else 0
		+ (bonus[player][1] ? 100 : 0)  // If second, add 100, else 0
		+ (bonus[player][2] ? 100 : 0); // If third, add 100, else 0
	upperTotal[player] =
		(ace[player] == -1 ? 0 : ace[player])
		+ (two[player] == -1 ? 0 : two[player])
		+ (three[player] == -1 ? 0 : three[player]) // Calculate upper section total
		+ (four[player] == -1 ? 0 : four[player])   // Also check for crossed out (-1)
		+ (five[player] == -1 ? 0 : five[player])
		+ (six[player] == -1 ? 0 : six[player]);
	if (upperTotal[player] >= 63) // If upper section total is >= 63, add 35 bonus
	{
		upperTotalBon[player] = upperTotal[player] + 35;
	}
	else upperTotalBon[player] = upperTotal[player];
	lowerTotal[player] =
		(kind_3[player] == -1 ? 0 : kind_3[player])
		+ (kind_4[player] == -1 ? 0 : kind_4[player])
		+ (fHouse[player] != -1 && fHouse[player] ? 25 : 0)
		+ (smStr[player] != -1 && smStr[player] ? 30 : 0)
		+ (lgStr[player] != -1 && lgStr[player] ? 40 : 0)
		+ (yahtz[player] != -1 && yahtz[player] ? 50 : 0)
		+ (chance[player] == -1 ? 0 : chance[player])
		+ bonusTotal[player];
	grandTotal[player] =
		upperTotalBon[player]
		+ lowerTotal[player];
}

// If a player chooses to cross out a box, this function is called
bool cross(int box)
{
	switch (box) {
	case 1:
		if (ace[player]) return 0;
		else
		{
			ace[player] = -1;
			return 1;
		}
	case 2:
		if (two[player]) return 0;
		else
		{
			two[player] = -1;
			return 1;
		}
	case 3:
		if (three[player]) return 0;
		else
		{
			three[player] = -1;
			return 1;
		}
	case 4:
		if (four[player]) return 0;
		else
		{
			four[player] = -1;
			return 1;
		}
	case 5:
		if (five[player]) return 0;
		else
		{
			five[player] = -1;
			return 1;
		}
	case 6:
		if (six[player]) return 0;
		else
		{
			six[player] = -1;
			return 1;
		}
	case 10:
		if (kind_3[player]) return 0;
		else
		{
			kind_3[player] = -1;
			return 1;
		}
	case 11:
		if (kind_4[player])	return 0;
		else
		{
			kind_4[player] = -1;
			return 1;
		}
	case 12:
		if (fHouse[player]) return 0;
		else
		{
			fHouse[player] = -1;
			return 1;
		}
	case 13:
		if (smStr[player]) return 0;
		else
		{
			smStr[player] = -1;
			return 1;
		}
	case 14:
		if (lgStr[player]) return 0;
		else
		{
			lgStr[player] = -1;
			return 1;
		}
	case 15:
		if (yahtz[player]) return 0;
		else
		{
			yahtz[player] = -1;
			return 1;
		}
	case 16:
		if (chance[player])	return 0;
		else
		{
			chance[player] = -1;
			return 1;
		}
	default:
		return 0;
	}
}

void scoresheet() 
{
	const char VERT_BAR = 179, FILL = 219, HORZ_BAR = 196,
		TL_COR = 218, TR_COR = 191, BL_COR = 192, BR_COR = 217,
		VBAR_R = 195, VBAR_L = 180, VBAR_RD = 198, VBAR_LD = 181,         // Shapes for drawing scoresheet
		HORZ_DBAR = 205, HBAR_DN = 194, HBAR_UP = 193, CBAR_SINGLE = 197,
		CBAR_DBL_H = 216, HBAR_DBL_UP = 207, HBAR_DBL_DN = 209;

	system("cls");

	if (endGame) // Calculate scores
	{
		calcScore();
	}

	cout << TL_COR << setw(36) << setfill(HORZ_BAR) << TR_COR << endl;
	cout << VERT_BAR << setfill(' ') << setw(8) << "Player " << (player + 1) << " : " << left << setw(22) << playerName[player] << setw(2) << right << VERT_BAR << endl; // Total whitespace = 36

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << HBAR_DN << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "                  Aces " << VERT_BAR << setfill(' ') << setw(8)
		<< ((ace[player] && ace[player] != -1) ? to_string(ace[player]) : "") // Clean these up with a function?
		<< ((ace[player] == -1) ? "\b\b\b\b\b-----   " : "   ")
		<< VERT_BAR << " [1]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "                  Twos " << VERT_BAR << setfill(' ') << setw(8)
		<< ((two[player] && two[player] != -1) ? to_string(two[player]) : "")
		<< ((two[player] == -1) ? "\b\b\b\b\b-----   " : "   ")
		<< VERT_BAR << " [2]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "                Threes " << VERT_BAR << setfill(' ') << setw(8)
		<< ((three[player] && three[player] != -1) ? to_string(three[player]) : "")
		<< ((three[player] == -1) ? "\b\b\b\b\b-----   " : "   ")
		<< VERT_BAR << " [3]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "                 Fours " << VERT_BAR << setfill(' ') << setw(8)
		<< ((four[player] && four[player] != -1) ? to_string(four[player]) : "")
		<< ((four[player] == -1) ? "\b\b\b\b\b-----   " : "   ")
		<< VERT_BAR << " [4]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "                 Fives " << VERT_BAR << setfill(' ') << setw(8)
		<< ((five[player] && five[player] != -1) ? to_string(five[player]) : "")
		<< ((five[player] == -1) ? "\b\b\b\b\b-----   " : "   ")
		<< VERT_BAR << " [5]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "                 Sixes " << VERT_BAR << setfill(' ') << setw(8)
		<< ((six[player] && six[player] != -1) ? to_string(six[player]) : "")
		<< ((six[player] == -1) ? "\b\b\b\b\b-----   " : "   ")
		<< VERT_BAR << " [6]" << endl;

	cout << VBAR_RD << setw(24) << setfill(HORZ_DBAR) << CBAR_DBL_H << setw(12) << VBAR_LD << endl;
	cout << VERT_BAR << "                 TOTAL " << VERT_BAR << setfill(' ') << setw(8)
		<< (upperTotal[player] ? to_string(upperTotal[player]) : "")
		<< "   " << VERT_BAR << endl;

	cout << VBAR_RD << setw(24) << setfill(HORZ_DBAR) << CBAR_DBL_H << setw(12) << VBAR_LD << endl;
	cout << VERT_BAR << "    Total at least 63? " << VERT_BAR
		<< isBonus_63(upperTotal[player])
		<< VERT_BAR << endl;

	cout << VBAR_RD << setw(24) << setfill(HORZ_DBAR) << CBAR_DBL_H << setw(12) << VBAR_LD << endl;
	cout << VERT_BAR << "           UPPER TOTAL " << VERT_BAR << setfill(' ') << setw(8)
		<< (upperTotalBon[player] ? to_string(upperTotalBon[player]) : "")
		<< "   " << VERT_BAR << endl;

	cout << VBAR_RD << setw(24) << setfill(HORZ_DBAR) << HBAR_DBL_UP << setw(12) << VBAR_LD << endl;
	cout << VBAR_RD << setw(24) << setfill(HORZ_DBAR) << HBAR_DBL_DN << setw(12) << VBAR_LD << endl;
	cout << VERT_BAR << " 3 of a kind (add all) " << VERT_BAR << setfill(' ') << setw(8)
		<< ((kind_3[player] && kind_3[player] != -1) ? to_string(kind_3[player]) : "")
		<< ((kind_3[player] == -1) ? "\b\b\b\b\b-----   " : "   ")
		<< VERT_BAR << " [10]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << " 4 of a kind (add all) " << VERT_BAR << setfill(' ') << setw(8)
		<< ((kind_4[player] && kind_4[player] != -1) ? to_string(kind_4[player]) : "")
		<< ((kind_4[player] == -1) ? "\b\b\b\b\b-----   " : "   ")
		<< VERT_BAR << " [11]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "   Full House (25 pts) " << VERT_BAR << setfill(' ') << setw(8)
		<< ((fHouse[player] && fHouse[player] != -1) ? "25" : " ")
		<< ((fHouse[player] == -1) ? "\b\b\b\b\b-----   " : "   ") // Full house (signed char)
		<< VERT_BAR << " [12]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << " Sm. Straight (30 pts) " << VERT_BAR << setfill(' ') << setw(8)
		<< ((smStr[player] && smStr[player] != -1) ? "30" : " ")
		<< ((smStr[player] == -1) ? "\b\b\b\b\b-----   " : "   ") // Small straight (signed char)
		<< VERT_BAR << " [13]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << " Lg. Striaght (40 pts) " << VERT_BAR << setfill(' ') << setw(8)
		<< ((lgStr[player] && lgStr[player] != -1) ? "40" : " ")
		<< ((lgStr[player] == -1) ? "\b\b\b\b\b-----   " : "   ") // Large straight (signed char)
		<< VERT_BAR << " [14]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "      YAHTZEE (50 pts) " << VERT_BAR << setfill(' ') << setw(8)
		<< ((yahtz[player] && yahtz[player] != -1) ? "50" : " ")
		<< ((yahtz[player] == -1) ? "\b\b\b\b\b-----   " : "   ") // Yahtzee (signed char)
		<< VERT_BAR << " [15]" << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "      Chance (add all) " << VERT_BAR << setfill(' ') << setw(8)
		<< ((chance[player] && chance[player] != -1) ? to_string(chance[player]) : "")
		<< ((chance[player] == -1) ? "\b\b\b\b\b-----   " : "   ")
		<< VERT_BAR << " [16]" << endl;


	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(4) << HBAR_DN << setw(4) << HBAR_DN << setw(4) << VBAR_L << endl;
	cout << VERT_BAR << "         BONUS YAHTZEE " << VERT_BAR
		<< " " << ((bonus[player][0]) ? "X" : " ") << " " << VERT_BAR          // First bonus exists or not (bool)
		<< " " << ((bonus[player][1]) ? "X" : " ") << " " << VERT_BAR          // Second. . .
		<< " " << ((bonus[player][2]) ? "X" : " ") << " " << VERT_BAR << " [20]" << endl; // Third . . .

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(4) << HBAR_UP << setw(4) << HBAR_UP << setw(4) << VBAR_L << endl;

	cout << VERT_BAR << "100 pts for each BONUS " << VERT_BAR << setfill(' ') << setw(9)
		<< (bonusTotal[player] ? to_string(bonusTotal[player]) : "")
		<< "  " << VERT_BAR << endl;

	cout << VBAR_RD << setw(24) << setfill(HORZ_DBAR) << HBAR_DBL_UP << setw(12) << VBAR_LD << endl;
	cout << VBAR_RD << setw(24) << setfill(HORZ_DBAR) << HBAR_DBL_DN << setw(12) << VBAR_LD << endl;
	cout << VERT_BAR << "   LOWER SECTION TOTAL " << VERT_BAR << setfill(' ') << setw(8)
		<< (lowerTotal[player] ? to_string(lowerTotal[player]) : "")
		<< "   " << VERT_BAR << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "   UPPER SECTION TOTAL " << VERT_BAR << setfill(' ') << setw(8)
		<< (upperTotalBon[player] ? to_string(upperTotalBon[player]) : "")
		<< "   " << VERT_BAR << endl;

	cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
	cout << VERT_BAR << "           GRAND TOTAL " << VERT_BAR << setfill(' ') << setw(8)
		<< (grandTotal[player] ? to_string(grandTotal[player]) : "")
		<< "   " << VERT_BAR << endl;

	cout << BL_COR << setw(24) << setfill(HORZ_BAR) << HBAR_UP << setw(12) << BR_COR << endl;
}

// Controls score placement and checks that score is being placed in a valid box
void endTurn(void) 
{                  
	int boxSelect = 100, boxTotal = 0, n = 0, i;
	bool isValid = 0;
	scoresheet();
	cout << "\rFinal roll results . . .      " << endl;
	cout << "Roll:      [" << dice[0] << "] " << "[" << dice[1] << "] " << "[" << dice[2] << "] " << "[" << dice[3] << "] " << "[" << dice[4] << "]" << endl;
	cout << "Enter an integer for the [Box] in which to count this roll, "
		<< "\n or enter [0] to cross out a box:";
	// Process selection
	while (!isValid) // Loops until a valid selection is made and completed
	{
		int crossBox = 0; 
		char Y_N = ' ';   
		cin >> boxSelect; 
		if (boxSelect <= 6 || boxSelect <= 16 && boxSelect >= 10 || boxSelect == 20) // if input is valid
		{
			cout << endl << "You chose box [" << boxSelect << "]" << endl << endl;
			switch (boxSelect) {
			case 0:
				cout << "Are you sure you want to cross out a box? (Y/N)";
				cin >> Y_N;
				Y_N = tolower(Y_N);
				if (Y_N == 'y')
				{
					cout << "Which box would you like to cross out? ";
					cin >> crossBox;
					if (cross(crossBox))
					{
						cout << "Box [" << crossBox << "] has been cross out";
						isValid = 1;
						break;
					}
					else
					{
						cout << "Invalid selection. Please make a new selection: ";
						break;
					}
				}
				else
				{
					cout << "You chose to not cross out a box.\nPlease make a new selection: ";
					break;

				}
			case 1:
				if (count(dice, dice + 5, 1))
				{
					if (ace[player])
					{
						cout << "Box [1] has already been used. Please choose another: ";
						break;
					}
					while (n <= 4)
					{
						if (dice[n] == 1)
							boxTotal++;
						n++;
					}

					ace[player] = boxTotal;
					isValid = 1;
					break;
				}
				else
				{
					cout << "You can't do that! Your roll contains no ones. Please choose again: ";
					break;
				}
			case 2:
				if (count(dice, dice + 5, 2))
				{
					if (two[player])
					{
						cout << "Box [2] has already been used. Please choose another: ";
						break;
					}
					while (n <= 4)
					{
						if (dice[n] == 2)
							boxTotal += 2;
						n++;
					}
					two[player] = boxTotal;
					isValid = 1;
					break;
				}
				else
				{
					cout << "You can't do that! Your roll contains no twos. Please choose again: ";
					break;
				}
			case 3:
				if (count(dice, dice + 5, 3))
				{
					if (three[player])
					{
						cout << "Box [3] has already been used. Please choose another: ";
						break;
					}
					while (n <= 4)
					{
						if (dice[n] == 3)
							boxTotal += 3;
						n++;
					}
					three[player] = boxTotal;
					isValid = 1;
					break;
				}
				else
				{
					cout << "You can't do that! Your roll contains no threes. Please choose again: ";
					break;
				}
			case 4:
				if (count(dice, dice + 5, 4))
				{
					if (four[player])
					{
						cout << "Box [4] has already been used. Please choose another: ";
						break;
					}
					while (n <= 4)
					{
						if (dice[n] == 4)
							boxTotal += 4;
						n++;
					}
					four[player] = boxTotal;
					isValid = 1;
					break;
				}
				else
				{
					cout << "You can't do that! Your roll contains no fours. Please choose again: ";
					break;
				}
			case 5:
				if (count(dice, dice + 5, 5))
				{
					if (five[player])
					{
						cout << "Box [5] has already been used. Please choose another: ";
						break;
					}
					while (n <= 4)
					{
						if (dice[n] == 5)
							boxTotal += 5;
						n++;
					}
					five[player] = boxTotal;
					isValid = 1;
					break;
				}
				else
				{
					cout << "You can't do that! Your roll contains no fives. Please choose again: ";
					break;
				}
			case 6: // Sixes
				if (count(dice, dice + 5, 6))
				{
					if (six[player])
					{
						cout << "Box [6] has already been used. Please choose another: ";
						break;
					}
					while (n <= 4)
					{
						if (dice[n] == 6)
							boxTotal += 6;
						n++;
					}
					six[player] = boxTotal;
					isValid = 1;
					break;
				}
				else
				{
					cout << "You can't do that! Your roll contains no sixes. Please choose again: ";
					break;
				}
			case 10: // 3 of a kind
				for (i = 1; !(count(dice, dice + 5, i) >= 3) && i <= 7; i++);
				if (i <= 6) // Since i was <= 5, roll contains 3 of a kind
				{
					if (kind_3[player])
					{
						cout << "Three of a kind [10] has already been used. Please choose another box: ";
						break;
					}
					while (n <= 4)
					{
						boxTotal += dice[n];
						n++;
					}
					kind_3[player] = boxTotal;
					isValid = 1;
					break;
				}
				else
				{
					cout << "You can't do that! You do not have three of a kind. Please choose another box: ";
					break;
				}
			case 11: // 4 of a kind
				for (i = 1; !(count(dice, dice + 5, i) >= 4) && i <= 7; i++);
				if (i <= 6) // Since i was <= 5, roll contains 4 of a kind
				{
					if (kind_4[player])
					{
						cout << "Four of a kind [11] has already been used. Please choose another box: ";
						break;
					}
					while (n <= 4)
					{
						boxTotal += dice[n];
						n++;
					}
					kind_4[player] = boxTotal;
					isValid = 1;
					break;
				}
				else
				{
					cout << "You can't do that! You do not have four of a kind. Please choose another box: ";
					break;
				}
			case 12: // Full house
				sort(dice, dice + 5);
				for (i = 1; !(count(dice, dice + 5, i) == 3 || count(dice, dice + 5, i) == 2) && i < 7; i++);
				if (i < 7)
				{
					if (count(dice, dice + 5, i) == 3)
					{
						for (i; !(count(dice, dice + 5, i) == 2) && i < 7; i++);
						if (i < 7)
						{
							if (fHouse[player])
							{
								cout << "Full house [12] has already been used. Please choose another box: ";
								break;
							}
							fHouse[player] = 1;
							isValid = 1;
							break;
						}
						{
							cout << "You can't do that! You don't have a Full House. Please choose another box: ";
							break;
						}
					}
					if (count(dice, dice + 5, i) == 2)
					{
						for (i; !(count(dice, dice + 5, i) == 3) && i < 7; i++);
						if (i < 7)
						{
							if (fHouse[player])
							{
								cout << "Full house [12] has already been used. Please choose another box: ";
								break;
							}
							fHouse[player] = 1;
							isValid = 1;
							break;
						}
						{
							cout << "You can't do that! You don't have a Full House. Please choose another box: ";
							break;
						}
					}
				}
				else
				{
					cout << "You can't do that! You don't have a Full House. Please choose another box: ";
					break;
				}
			case 13: // Small straight
				if ((count(dice, dice + 5, 1) && count(dice, dice + 5, 2) && count(dice, dice + 5, 3) && count(dice, dice + 5, 4))
					|| (count(dice, dice + 5, 2) && count(dice, dice + 5, 3) && count(dice, dice + 5, 4) && count(dice, dice + 5, 5))
					|| (count(dice, dice + 5, 3) && count(dice, dice + 5, 4) && count(dice, dice + 5, 5) && count(dice, dice + 5, 6))
					)
				{

					if (smStr[player])
					{
						cout << "Small straight [13] has already been used. Please choose another box: ";
						break;
					}
					smStr[player] = 1;
					isValid = 1;
					break;
				}
				else
				{
					cout << "You can't do that! You don't have a Small Straight. Please enter another box: ";
					break;
				}
			case 14: // Large straight
				sort(dice, dice + 5); // Sort for comparison
				for (i = 0; dice[i] == (dice[i + 1] - 1) && i < 4; i++);
				if (i == 4)
				{
					if (lgStr[player])
					{
						cout << "Large straight [14] has already been used. Please choose another box: ";
						break;
					}
					lgStr[player] = 1;
					isValid = 1;
					break;
				}
				else
				{
					cout << "You can't do that! You don't have a Large Straight. Please choose another box: ";
					break;
				}
			case 15: // Yahtzee
				for (i = 1; !(count(dice, dice + 5, i) == 5) && i < 7; i++);
				if (i < 7)
				{
					if (yahtz[player])
					{
						cout << "Yahtzee [15] has already been used.\nWould you like to use one of your bonus yahtzee slots?\nIf you choose yes, you must also cross out a box. (Y/N)";
						cin >> Y_N;
						Y_N = tolower(Y_N);
						if (Y_N == 'y')
						{
							if (!bonus[player][0])
							{
								bonus[player][0] = 1;
								cout << endl << "The first Yahtzee bonus box has been used." << endl;
								cout << "Which box would you like to cross out? ";
								cin >> crossBox;
								while (true)
								{

									if (cross(crossBox))
									{
										cout << "Box [" << crossBox << "] has been crossed out";
										break;
									}
									else
									{
										cout << "Invalid selection. Please choose another box to cross out: ";
									}
								}
								isValid = 1;
								break;
							}
							if (!bonus[player][1])
							{
								bonus[player][1] = 1;
								cout << endl << "The second Yahtzee bonus box has been used." << endl;
								cout << "Which box would you like to cross out? ";
								cin >> crossBox;
								while (true)
								{

									if (cross(crossBox))
									{
										cout << "Box [" << crossBox << "] has been crossed out";
										break;
									}
									else
									{
										cout << "Invalid selection. Please choose another box to cross out: ";
										crossBox = 0;
										cin >> crossBox;
									}
								}
								isValid = 1;
								break;
							}
							if (!bonus[player][2])
							{
								bonus[player][2] = 1;
								cout << endl << "The last Yahtzee bonus box has been used." << endl;
								cout << "Which box would you like to cross out? ";
								cin >> crossBox;
								while (true)
								{

									if (cross(crossBox))
									{
										cout << "Box [" << crossBox << "] has been crossed out";
										break;
									}
									else
									{
										cout << "Invalid selection. Please choose another box to cross out: ";
										crossBox = 0;
										cin >> crossBox;
									}
								}
								isValid = 1;
								break;
							}
							else
							{
								cout << " Sorry, you've used all your bonus Yahtzee boxes.\nPlease choose another box, or enter [0] to cross out a box: ";
								break;
							}

						}
						else
						{
							cout << "You chose to not cross out a box.\nPlease make a new selection: ";
							break;
						}
					}
					yahtz[player] = 1;
					isValid = 1;
					break;
				}
				else
				{
					cout << "Nice try! You don't have a Yahtzee, do you? Tsk Tsk. Enter another box: ";
					break;
				}
			case 16:
				if (chance[player])
				{
					cout << "Chance [16] has already been used. Please choose another box: ";
					break;
				}
				while (n <= 4)
				{
					boxTotal += dice[n];
					n++;
				}
				chance[player] = boxTotal;
				isValid = 1;
				break;
			case 20:
				for (i = 1; !(count(dice, dice + 5, i) == 5) && i < 7; i++);
				if (i < 7)
				{
					if (yahtz[player])
					{
						cout << "If you use a Bonus Yahtzee box, you must cross out another box.\nAre you sure? (Y/N) ";
						cin >> Y_N;
						Y_N = tolower(Y_N);
						if (Y_N == 'y')
						{
							if (!bonus[player][0])
							{
								bonus[player][0] = 1;
								cout << endl << "The first Yahtzee bonus box has been used." << endl;
								cout << "Which box would you like to cross out? ";
								cin >> crossBox;
								while (true)
								{

									if (cross(crossBox))
									{
										cout << "Box [" << crossBox << "] has been crossed out";
										break;
									}
									else
									{
										cout << "Invalid selection. Please choose another box to cross out: ";
									}
								}
								isValid = 1;
								break;
							}
							if (!bonus[player][1])
							{
								bonus[player][1] = 1;
								cout << endl << "The second Yahtzee bonus box has been used." << endl;
								cout << "Which box would you like to cross out? ";
								cin >> crossBox;
								while (true)
								{

									if (cross(crossBox))
									{
										cout << "Box [" << crossBox << "] has been crossed out";
										break;
									}
									else
									{
										cout << "Invalid selection. Please choose another box to cross out: ";
										crossBox = 0;
										cin >> crossBox;
									}
								}
								isValid = 1;
								break;
							}
							if (!bonus[player][2])
							{
								bonus[player][2] = 1;
								cout << endl << "The last Yahtzee bonus box has been used." << endl;
								cout << "Which box would you like to cross out? ";
								cin >> crossBox;
								while (true)
								{

									if (cross(crossBox))
									{
										cout << "Box [" << crossBox << "] has been crossed out";
										break;
									}
									else
									{
										cout << "Invalid selection. Please choose another box to cross out: ";
										crossBox = 0;
										cin >> crossBox;
									}
								}
								isValid = 1;
								break;
							}
							else
							{
								cout << " Sorry, you've used all your bonus Yahtzee boxes.\nPlease choose another box, or enter [0] to cross out a box: ";
								break;
							}
						}
						else
						{
							cout << "You chose to not cross out a box.\nPlease make a new selection: ";
							break;

						}
					}
					else
					{
						cout << "You must use your Yahtzee box before using Bonus Yahtzee boxes.\nPlease try another box: ";
						break;
					}
				}
				else
				{
					cout << "Nice try! You don't have a Yahtzee, do you? Tsk Tsk. Enter another box: ";
					break;
				}
			default:
				cout << "[" << boxSelect << "] is not a valid choice. Please enter another box: ";
			}
		}
		else // if box number input is NOT valid
		{
			cin.clear();
			cin.ignore(INT_MAX, '\n');
			cout << "Invalid entry. Please enter another box: ";
		}
	}
}

void roll(int a) // Dice roll function // Output random number until 20n iterations, then pick value
{
	int b = a * 20; // Show held die					// The integer pass to the function indicates the 
	while (a <= 4)  // Loop until fifth die is rolled   // number of HELD die. If int_a is 2, 
	{													// only the last three die are rolled.
		cout << "[" << ((b < 20) ? rand() % 6 + 1 : dice[0]) << "] "
			<< "[" << ((b < 40) ? rand() % 6 + 1 : dice[1]) << "] "
			<< "[" << ((b < 60) ? rand() % 6 + 1 : dice[2]) << "] "
			<< "[" << ((b < 80) ? rand() % 6 + 1 : dice[3]) << "] "
			<< "[" << ((b < 100) ? rand() % 6 + 1 : dice[4]) << "]\r";
		b++;
		Sleep(35);     // Throttle output speed for visual awesomeness

		if (!(b % 20)) // Evaluate true every 20th iteration (b % 20 == 0)
		{
			dice[a] = rand() % 6 + 1; // Set the value of the ath die
			a++;                      // Increment counter
		}
	}
	sort(dice, dice + 5);             // Sort the die in ascending order so that the placement verification
}									  // algorithms can work with them.

void choice(string q) // Function to parse, store, and echo which die were held 
{					  // Sets number of die held (hc)
	dieHeldCount = 0;           // Reset hold counter
	int dt[11];       // Temporary die value holder
	if (q.find('1') <= q.size()) // Block 1 for detecting and parsing hold input
	{
		++dieHeldCount;                     // Found "1", so increment hold counter
		dt[dieHeldCount] = dice[0];         // Store temporary copy of first die's value at current hc value
		dt[dieHeldCount + 5] = 1;           // Store the die number for output
	}
	if (q.find('2') <= q.size()) // Block 2 for detecting and parsing hold input
	{							  // Same as first block + 1
		++dieHeldCount;
		dt[dieHeldCount] = dice[1];
		dt[dieHeldCount + 5] = 2;
	}
	if (q.find('3') <= q.size()) // Block 3 for detecting and parsing hold input
	{							  // Same as first block + 2
		++dieHeldCount;
		dt[dieHeldCount] = dice[2];
		dt[dieHeldCount + 5] = 3;
	}
	if (q.find('4') <= q.size()) // Block 4 for detecting and parsing hold input
	{							  // Same as first block + 3
		++dieHeldCount;
		dt[dieHeldCount] = dice[3];
		dt[dieHeldCount + 5] = 4;
	}
	if (q.find('5') <= q.size()) // Block 5 for detecting and parsing hold input
	{							  // Same as first block + 4
		++dieHeldCount;
		dt[dieHeldCount] = dice[4];
		dt[dieHeldCount + 5] = 5;
	}
	if (dieHeldCount == 0)
	{
		// No die held
		cout << endl << "You chose not to hold any die." << endl;
	}
	else if (dieHeldCount == 1)
	{
		cout << endl << "You chose to hold die number " << dt[6] << "." << endl;
		cout << "Your held die: [" << dt[1] << "]" << endl;
		dice[0] = dt[1];
	}
	else if (dieHeldCount == 2)
	{
		// Two die held
		cout << endl << "You chose to hold die numbers " << dt[6] << " and " << dt[7] << endl;
		cout << "Your held die: [" << dt[1] << "] [" << dt[2] << "]" << endl;
		dice[0] = dt[1];
		dice[1] = dt[2];
	}
	else if (dieHeldCount == 3)
	{
		// Three die held
		cout << endl << "You chose to hold die numbers " << dt[6] << ", " << dt[7] << ", and " << dt[8] << "." << endl;
		cout << "Your held die: [" << dt[1] << "] [" << dt[2] << "] [" << dt[3] << "]" << endl;
		dice[0] = dt[1];
		dice[1] = dt[2];
		dice[2] = dt[3];
	}
	else if (dieHeldCount == 4)
	{
		// Four die held
		cout << endl << "You chose to hold die numbers " << dt[6] << ", " << dt[7] << ", " << dt[8] << ", and " << dt[9] << "." << endl;
		cout << "Your held die: [" << dt[1] << "] [" << dt[2] << "] [" << dt[3] << "] [" << dt[4] << "]" << endl;
		dice[0] = dt[1];
		dice[1] = dt[2];
		dice[2] = dt[3];
		dice[3] = dt[4];
	}
	else if (dieHeldCount == 5)
	{
		// Five die held
		cout << endl << "You chose to hold die numbers " << dt[6] << ", " << dt[7] << ", " << dt[8] << ", " << dt[9] << ", and " << dt[10] << "." << endl;
		cout << "Your held die: [" << dt[1] << "] [" << dt[2] << "] [" << dt[3] << "] [" << dt[4] << "] [" << dt[5] << "]" << endl;
		dice[0] = dt[1];
		dice[1] = dt[2];
		dice[2] = dt[3];
		dice[3] = dt[4];
		dice[4] = dt[5];
	}
}

void playerturn(void) 
{
	srand(time(NULL));     // Set seed for randomization
	scoresheet();          // Show scoresheet for player
	roll(0);               // Dice rolling function // See roll() for input explanation
	// First roll result
	cout << "\rYou rolled . . .            " << endl;
	cout << "Roll:      [" << dice[0] << "] " << "[" << dice[1] << "] " << "[" << dice[2] << "] " << "[" << dice[3] << "] " << "[" << dice[4] << "]" << endl;
	cout << "            |   |   |   |   |" << endl;
	cout << "Die number: 1   2   3   4   5" << endl << endl;
	cout << "Which die would you like to hold? ";
	if (isFirstRoll) // To avoid that pesky \n haunting the input stream after one cycle through playerturn()
	{
		cin.ignore();    // Ignore one character
	}
	else isFirstRoll = 1;
	getline(cin, choose);
	choice(choose);           // Parse die selections and output result
	if (dieHeldCount == 5)
	{
		endTurn();            // All die were held, so call endTurn() then exit playerturn
		return;
	}
	roll(dieHeldCount);                 // Call roll() with number of die held
	// Second roll result
	cout << "\rSecond roll . . .           " << endl;
	cout << "Roll:      [" << dice[0] << "] " << "[" << dice[1] << "] " << "[" << dice[2] << "] " << "[" << dice[3] << "] " << "[" << dice[4] << "]" << endl;
	cout << "            |   |   |   |   |" << endl;
	cout << "Die number: 1   2   3   4   5" << endl << endl;
	cout << "Which die would you like to hold? ";
	choose.clear(); // Delete contents of choose. Not sure if needed. Don't remember why it was added
	getline(cin, choose);
	choice(choose);        // Parse die selections and output result
	if (dieHeldCount == 5)
	{
		endTurn();  // All die were held, so call endTurn() then exit playerturn
		return;
	}
	roll(dieHeldCount); // Call roll() again
	// Final roll result
	cout << "\rFinal roll results . . .      " << endl;
	cout << "Roll:      [" << dice[0] << "] " << "[" << dice[1] << "] " << "[" << dice[2] << "] " << "[" << dice[3] << "] " << "[" << dice[4] << "]" << endl;
	endTurn(); // Final roll is done. Player to place score
}

// Function for helping the sorting of high scores
bool sortByScore(const highScore &a, const highScore &b) {
	return a.HS_Score > b.HS_Score;
}

// Returns a string containing the current date in MM/DD/YYYY format
string scoreDate(void)
{
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "%m/%d/%Y", timeinfo);
	return buffer;
}

void highScoreCalc(void)
{ 
	ifstream scoreTable;
	scoreTable.open("scoreTable.ytz");
	if (!scoreTable.good())
	{
		ofstream scoreCreate("scoreTable.ytz");
		for (int i = 0; i < 20; i++)
		{
			scoreCreate << "Player 0 01/01/2000" << endl;
		}
		scoreCreate.close();
		scoreTable.open("scoreTable.ytz"); //Declared inside block causes problem.
		system("pause");
	}

	for (int n = 0; n < 20; n++)
	{
		scoreTable >> HS_Data[n].HS_Name;
		scoreTable >> HS_Data[n].HS_Score;
		scoreTable >> HS_Data[n].HS_Date;
	}
	scoreTable.close();
	for (int i = 0; i <= numPlayers; i++)
	{
		if (grandTotal[i] > HS_Data[19].HS_Score)
		{
			HS_Current[i].HS_Name = playerName[i];
			HS_Current[i].HS_Date = scoreDate();
			HS_Current[i].HS_Score = grandTotal[i];
			HS_Data[i + 20] = HS_Current[i];
			isHighScore[i] = 1;
		}
	}


	sort(HS_Data, HS_Data + 24, sortByScore);
	ofstream scoreWrite("scoreTable.ytz");
	for (int i = 0; i < 20; i++)
	{
		scoreWrite << setfill (' ') << left << setw(22) << HS_Data[i].HS_Name << ' '
				   << right << setw(5) << HS_Data[i].HS_Score << ' '
				   << setw(10) << HS_Data[i].HS_Date << endl;
	}
	scoreWrite.close();
}

void highScoreTable(void)
{ 
	cout << setfill(' ') << left << setw(22) << "Name"
		<< right << setw(5) << "Score"
		<< setw(10) << "Date" << endl;

	for (int i = 0; i < 20; i++)
	{
		cout << setfill(' ') << left << setw(22) << HS_Data[i].HS_Name << ' '
			 << right << setw(5) << HS_Data[i].HS_Score << ' '
			 << setw(10) << HS_Data[i].HS_Date << endl;

	}
}

void end_Game()
{
	int endChoice = 0, exit = 0;

	const char VERT_BAR = 179, FILL = 219, HORZ_BAR = 196,
		TL_COR = 218, TR_COR = 191, BL_COR = 192, BR_COR = 217,
		VBAR_R = 195, VBAR_L = 180, VBAR_RD = 198, VBAR_LD = 181,         // Shapes for drawing scoresheet
		HORZ_DBAR = 205, HBAR_DN = 194, HBAR_UP = 193, CBAR_SINGLE = 197,
		CBAR_DBL_H = 216, HBAR_DBL_UP = 207, HBAR_DBL_DN = 209;
	for (player = 0; numPlayers >= player; player++)
	{
		calcScore();
	}
	system("cls");
	cout << "The game has ended!\nCalculating the final scores..." << endl << endl;
	Sleep(5000);
	player = 0;
	while (player <= numPlayers)
	{
		cout << TL_COR << setw(36) << setfill(HORZ_BAR) << TR_COR << endl;
		cout << VERT_BAR << setfill(' ') << setw(8) << "Player " << (player + 1) << " : " << left << setw(22) << playerName[player] << setw(2) << right << VERT_BAR << endl; // Total whitespace = 36

		cout << VBAR_RD << setw(24) << setfill(HORZ_DBAR) << HBAR_DBL_DN << setw(12) << VBAR_LD << endl;
		cout << VERT_BAR << "   LOWER SECTION TOTAL " << VERT_BAR << setfill(' ') << setw(8)
			<< (lowerTotal[player] ? to_string(lowerTotal[player]) : "")
			<< "   " << VERT_BAR << endl;

		cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
		cout << VERT_BAR << "   UPPER SECTION TOTAL " << VERT_BAR << setfill(' ') << setw(8)
			<< (upperTotalBon[player] ? to_string(upperTotalBon[player]) : "")
			<< "   " << VERT_BAR << endl;

		cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
		cout << VERT_BAR << "           GRAND TOTAL " << VERT_BAR << setfill(' ') << setw(8)
			<< (grandTotal[player] ? to_string(grandTotal[player]) : "")
			<< "   " << VERT_BAR << endl;

		cout << BL_COR << setw(24) << setfill(HORZ_BAR) << HBAR_UP << setw(12) << BR_COR << endl << endl;

		player++;
	}

	highScoreCalc();

	while (!exit)
	{
		for (int i = 0; i <= numPlayers; i++)
		{
			if (isHighScore[i])
			{
				cout << playerName[i] << " has made the high score list!" << endl << endl;
			}
			
		}
		cout << "Enter player number to view that player's scorecard." << endl;
		cout << "Enter 5 to view a summary of all player's scores." << endl;
		cout << "Enter 6 to view the high scores." << endl;
		cout << "Enter 0 to quit the game." << endl;
		cout << "Please enter your selection: ";
		cin >> endChoice;
		if (!endChoice)
		{
			return;
		}
		if (endChoice == 6)
		{
			highScoreTable();
		}
		else if (endChoice == 5)
		{
			system("cls");
			player = 0;
			while (player <= numPlayers)
			{
				cout << TL_COR << setw(36) << setfill(HORZ_BAR) << TR_COR << endl;
				cout << VERT_BAR << setfill(' ') << setw(18) << (player + 1) << setw(18) << VERT_BAR << endl; // Total whitespace = 36

				cout << VBAR_RD << setw(24) << setfill(HORZ_DBAR) << HBAR_DBL_DN << setw(12) << VBAR_LD << endl;
				cout << VERT_BAR << "   LOWER SECTION TOTAL " << VERT_BAR << setfill(' ') << setw(8)
					<< (lowerTotal[player] ? to_string(lowerTotal[player]) : "")
					<< "   " << VERT_BAR << endl;

				cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
				cout << VERT_BAR << "   UPPER SECTION TOTAL " << VERT_BAR << setfill(' ') << setw(8)
					<< (upperTotalBon[player] ? to_string(upperTotalBon[player]) : "")
					<< "   " << VERT_BAR << endl;

				cout << VBAR_R << setw(24) << setfill(HORZ_BAR) << CBAR_SINGLE << setw(12) << VBAR_L << endl;
				cout << VERT_BAR << "           GRAND TOTAL " << VERT_BAR << setfill(' ') << setw(8)
					<< (grandTotal[player] ? to_string(grandTotal[player]) : "")
					<< "   " << VERT_BAR << endl;

				cout << BL_COR << setw(24) << setfill(HORZ_BAR) << HBAR_UP << setw(12) << BR_COR << endl << endl;

				player++;
			}
		}
		else if (endChoice <= (numPlayers + 1))
		{
			player = (endChoice - 1);
			scoresheet();
		}
		else
		{
			cout << "Invalid selection. Please try again." << endl << endl;
		}
	}
}

int main()
{
	MaximizeWindow();
	cout << "YY    YY    AA      HH    HH TTTTTTTTTT ZZZZZZZZZZ  EEEEE    EEEEE    !!! " << endl;
	cout << " YY  YY    AAAA     HH    HH     TT           ZZZ  EE   EE  EE   EE  !!!!!" << endl;
	cout << "  YYYY    AA  AA    HH    HH     TT          ZZZ   EE  EEE  EE  EEE   !!! " << endl;
	cout << "   YY    AA    AA   HHHHHHHH     TT         ZZZ    EEE      EEE       !!! " << endl;
	cout << "   YY   AAAAAAAAAA  HH    HH     TT        ZZZ      EEEEEE   EEEEEE    !  " << endl;
	cout << "   YY   AA      AA  HH    HH     TT       ZZZ                             " << endl;
	cout << "   YY   AA      AA  HH    HH     TT      ZZZ                          !!! " << endl;
	cout << "                                        ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ !!! " << endl << endl;

	cout << "Welcome to YAHTZEE!\n\nPlease enter the number of players (up to four): ";
	cin >> numPlayers;
	while (numPlayers > 4 || numPlayers < 1)
	{
		if (cin.fail())
		{
			cin.clear();
			cin.ignore(INT_MAX, '\n');
		}
		cout << endl << "Invalid entry. The number of players must be between 1 and 4.\nPlease enter the number of players again: ";
		cin >> numPlayers;
	}
	cin.ignore();
	numPlayers--;
	for (int np = 0; np <= numPlayers; np++)
	{
		cout << endl << "Please enter Player " << (np + 1) << "'s name: ";
		getline(cin, playerName[np]);
		if (playerName[np].size() > 22)
		{
			playerName[np].resize(22);
		}
		cout << " Player " << (np + 1) << "'s name has been set as '" << playerName[np] << "'" << endl;
	}
	cout << endl << endl << "Game is ready! Starting in.. ";
	for (int readyGo = 5; readyGo >= 1; readyGo--)
	{
		cout << readyGo << ".. ";
		Sleep(900);
	}
	while (roundNum <= 13) // Game loop
	{
		cout << endl << endl << "It's now " << (playerName[player]) << "'s turn. [ Player " << (player + 1) << " ]";
		Sleep(3000);

		playerturn();
		scoresheet();
		if (player == numPlayers)
		{
			player = 0;
			roundNum++;
		}
		else
		{
			player++;
		}
	}

	endGame = 1;
	end_Game();

	return 0;
}