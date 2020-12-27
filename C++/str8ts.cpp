#include "str8ts.h"
#include <algorithm>
#include <bitset>
#include <ctime>
#include <iterator>
#include <string>

Random::Random()
{
	mt = std::mt19937(rd());
}
int Random::randInt(const int start, const int end)
{
	std::uniform_int_distribution<int> dist = std::uniform_int_distribution<int>(start, end);
	return dist(mt);
}
void Random::shuffle(std::vector<int>::iterator begin, std::vector<int>::iterator end)
{
	std::shuffle(begin, end, mt);
}

double passedTime(const std::chrono::high_resolution_clock::time_point startTime)
{
	return ((std::chrono::duration<double>)(std::chrono::high_resolution_clock::now() - startTime)).count();
}

/// <summary>
/// Encode and format the game in a human-readable format
/// </summary>
/// <param name="game">The game to be encoded</param>
/// <returns>The game in a human-readable format</returns>
std::string gameToString(const std::vector<Field>& game)
{
	std::string result = "";
	for (int i = 0; i < 81; i++)
	{
		if (i % 9 == 0)
			result += "   ";
		switch (game[i].mode)
		{
		case BLACK:
			result += "    ";
			break;
		case BLACKKNOWN:
			result += "[" + std::to_string(game[i].number) + "] ";
			break;
		case KNOWN:
			result += " " + std::to_string(game[i].number) + "  ";
			break;
		default:
			result += "(" + std::to_string(game[i].number) + ") ";
		}
		if ((i + 1) % 9 == 0)
			result += "\n";
	}
	return result;
}

/// <summary>
/// Encode / encrypt a Str8ts game as a url base64url string
/// (c.f. rfc464)
/// </summary>
/// <param name="game">Game field to be encoded</param>
/// <param name="additionalKnownNumbers">Position of numbers which can be shown to the player to reduce the difficulty</param>
/// <returns>The game as a base64url encoded string including the encoding version number</returns>
std::string encodeGame(const std::vector<Field>& game, const std::vector<int>& additionalKnownNumbers)
{
	// Encode game as binary
	std::string binary = std::bitset<8>(ENCODING_VERSION).to_string(); // Include encoding version number
	for (int i = 0; i < 81; i++)
	{
		int black = (game[i].mode == BLACK || game[i].mode == BLACKKNOWN) ? 1 : 0;
		int known = (game[i].mode == USER || game[i].mode == BLACK) ? 0 : 1;
		binary += std::to_string(black) + std::to_string(known) + std::bitset<4>(game[i].number - 1LL).to_string();
	}
	for (int i = 0; i < additionalKnownNumbers.size(); i++)
	{
		binary += std::bitset<7>(additionalKnownNumbers[i]).to_string();
	}

	// Encode binary data as base 64
	std::string result = "";
	for (int i = 0; i < binary.length() + 5; i += 6)
	{	
		std::string bits;
		bits = binary.substr(i, std::min<int>(6, binary.length() - i));
		while (bits.length() < 6)
			bits += "0";
		result += BASE[stoi(bits, nullptr, 2)];
	}
	return result;
}

/// <summary>
/// Recursively filles a given basic Str8ts game field with numbers
/// </summary>
/// <param name="step">Step / position of the game to be filled with a number</param>
/// <param name="game">Game to be filled with numbers</param>
/// <param name="recursionDepth">Depth of the backtracking recursion</param>
/// <returns></returns>
bool backtrackCreate(int step, std::vector<Field> &game, int & recursionDepth)
{
	if (recursionDepth > MAX_RECURSION_DEPTH_CREATE) // If recursion depth is to large return and start from the beginning
		return false;
	recursionDepth++;
	if (game[step].mode == BLACK)
		return backtrackCreate(step + 1, game, recursionDepth);
	std::vector<int> numbers = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	mRandom.shuffle(numbers.begin(), numbers.end());
	for (int i = 0; i < 9; i++) // Try all nine shuffled numbers on after another
	{
		if (checkIfNumberIsAllowed(step, numbers[i], game))
		{
			game[step].number = numbers[i]; // Number is valid and stored in the game array
			bool state = false;
			if (step == 80 || backtrackCreate(step + 1, game, recursionDepth)) // If it is the last field return, otherwise start algorithm for next field
				return true;
			else // If algorithm for next field failed, step back and try next number or also return to previous step
				game[step].number = 0;
		}
	}
	return false;
}

/// <summary>
/// Check if a number is allowed at a certain position in the partially complete game according to the general Str8ts rules
/// </summary>
/// <param name="position">Position for the number</param>
/// <param name="number">Number to be checked</param>
/// <param name="game">Partially complete game field for which the number should be checked</param>
/// <returns>If the number is allowed</returns>
bool checkIfNumberIsAllowed(const int position, const int number, const std::vector<Field>& game)
{
	bool invalid = false;
	// Check if number already exists in same row
	int index = position - position % 9; // Arrayindex of start of row
	for (int horizontal = index; horizontal < index + 9; horizontal++)
	{
		if (game[horizontal].number == number)
		{
			invalid = true;
			break; // Number already exists in row so skip the for loop and afterwards continue with next number
		}
	}
	if (invalid)
		return false;

	// Check if number already exists in same column
	for (int horizontal = position % 9; horizontal < 81; horizontal += 9)
	{
		if (game[horizontal].number == number)
		{
			invalid = true;
			break; // Number already exists in column so skip the for loop and afterwards continue with next number
		}
	}
	if (invalid)
		return false;

	// Check row space
	// Check if new number together with existing numbers can form a sequence in the available (horizontal) fields
	int minimum = number;
	int maximum = number;
	int minIndex = position;
	int maxIndex = position;
	while (minIndex % 9 > 0 && game[minIndex - 1LL].mode == USER) // Find left end of field row
	{
		minIndex--;
		if (game[minIndex].number > 0 && game[minIndex].number < minimum)
			minimum = game[minIndex].number;
		if (game[minIndex].number > maximum)
			maximum = game[minIndex].number;
	}
	while (maxIndex % 9 < 8 && game[maxIndex + 1LL].mode == USER) // Find right end of field row
	{
		maxIndex++;
		if (game[maxIndex].number > 0 && game[maxIndex].number < minimum)
			minimum = game[maxIndex].number;
		if (game[maxIndex].number > maximum)
			maximum = game[maxIndex].number;
	}
	if (maximum - minimum > maxIndex - minIndex)
		return false;

	// Check col space
	// Check if new number together with existing numbers can form a sequence in the available (vertical) fields
	minimum = number;
	maximum = number;
	minIndex = position;
	maxIndex = position;
	while (minIndex > 8 && game[minIndex - 9LL].mode == USER) // Find upper end of field column
	{
		minIndex -= 9;
		if (game[minIndex].number > 0 && game[minIndex].number < minimum)
			minimum = game[minIndex].number;
		if (game[minIndex].number > maximum)
			maximum = game[minIndex].number;
	}
	while (maxIndex < 72 && game[maxIndex + 9LL].mode == USER) // Find lower end of field column
	{
		maxIndex += 9;
		if (game[maxIndex].number > 0 && game[maxIndex].number < minimum)
			minimum = game[maxIndex].number;
		if (game[maxIndex].number > maximum)
			maximum = game[maxIndex].number;
	}
	if (maximum - minimum > maxIndex / 9 - minIndex / 9)
		return false;
	return true;
}

/// <summary>
/// Generate a new Str8ts game
/// </summary>
/// <param name="game">Field where the new game is stored</param>
/// <param name="generatorCount">Number of previous retries to generate the black game fields</param>
/// <param name="numberGeneratorCount">Number of previous retries to generate all numbers</param>
void generate(std::vector<Field> &game, int generatorCount, int numberGeneratorCount)
{
	//Increase number of retries to generate a new game
	generatorCount++;

	// Initialize game field
	game.assign(81, Field());

	// Generate black fields
	int totalBlackFields = 19 + mRandom.randInt(0, 3);
	int count = 0;
	bool symmetric = mRandom.randInt(0, 1);
	while (count < totalBlackFields)
	{
		int position = mRandom.randInt(0, 80);
		if (game[position].mode != BLACK)
		{
			game[position].mode = BLACK;
			count++;
			if (symmetric)
			{
				int reversePosition = 80 - position;
				game[reversePosition].mode = BLACK;
				count++;
			}
		}
	}

	// Check black fields
	for (int row = 0; row < 9; row++)
	{
		int singleFieldsColumn = 0;
		int singleFieldsRow = 0;
		for (int col = 0; col < 9; col++)
		{
			// To check if there are unanted clusters of black fields or lone white fields all eight fields around the current field are examined
			int horizontalNeighbors = 0;
			int verticalNeighbors = 0;
			int secondOrderNeighbors = 0;
			if (row > 0 && game[(row - 1LL) * 9 + col].mode == BLACK)
				horizontalNeighbors++;
			if (row < 8 && game[(row + 1LL) * 9 + col].mode == BLACK)
				horizontalNeighbors++;
			if (col > 0 && game[row * 9LL + (col - 1LL)].mode == BLACK)
				verticalNeighbors++;
			if (col < 8 && game[row * 9LL + (col + 1LL)].mode == BLACK)
				verticalNeighbors++;
			if (row > 0 && col > 0 && game[(row - 1LL) * 9 + (col - 1LL)].mode == BLACK)
				secondOrderNeighbors++;
			if (row > 0 && col < 8 && game[(row - 1LL) * 9 + (col + 1LL)].mode == BLACK)
				secondOrderNeighbors++;
			if (row < 8 && col > 0 && game[(row + 1LL) * 9 + (col - 1LL)].mode == BLACK)
				secondOrderNeighbors++;
			if (row < 8 && col < 8 && game[(row + 1LL) * 9 + (col + 1LL)].mode == BLACK)
				secondOrderNeighbors++;
			if (horizontalNeighbors > 1)
				singleFieldsRow++;
			if (verticalNeighbors > 1)
				singleFieldsColumn++;
			if (horizontalNeighbors + verticalNeighbors + secondOrderNeighbors > 3) // Restart game generation because of too many black fields
				return generate(game, generatorCount);
		}
		if (singleFieldsColumn > 1 || singleFieldsRow > 1)
			return generate(game, generatorCount);
	}

	// Generate numbers 
	int recursionDepthCreate = 0;
	if (!backtrackCreate(0, game, recursionDepthCreate)) // If algorithm fails restart game generation (with increased numberGeneratorCount)
		return generate(game, generatorCount, numberGeneratorCount + 1);
	debugPrint("BACKTRACKING CREATOR: Field retries: " << generatorCount << ", Number retries: " << numberGeneratorCount  << ", Depth: " << recursionDepthCreate << " (Time: " << passedTime(benchmarkTimeStart) << " seconds)" << std::endl);

	// Generate known black fields
	int knownBlackFieldCount = 0;
	int maxKnownBlackFields = 3 + mRandom.randInt(0, 2);
	int findBlackFieldsRetries = 0;
	while (knownBlackFieldCount < maxKnownBlackFields || findBlackFieldsRetries > FIND_KNOWN_BLACK_FIELDS_RETRIES) // Retry until enough black fields were found or max number of retries
	{
		findBlackFieldsRetries++;
		int i = mRandom.randInt(0, 80);
		if (game[i].mode == BLACK)
		{
			int col = i % 9;
			int row = i / 9;
			for (int num = 1; num < 10; num++) // Check which number does not exists in the same row and column
			{
				bool exists = false;
				for (int s = 0; s < 9; s++)
				{
					int horizontal = 9 * row + s;
					int vertical = col + s * 9;
					if (game[horizontal].number == num || game[vertical].number == num)
					{
						exists = true;
						break;
					}
				}
				if (!exists) // Number does not exist and is stored
				{
					game[i].mode = BLACKKNOWN;
					game[i].number = num;
					knownBlackFieldCount++;
					break;
				}
			}
		}
	}

	debugPrint("GAME WITH ALL NUMBERS:" << std::endl);
	debugPrint(gameToString(game));
	
	// Remove numbers to generate user input fields
	bool solvable = true;
	int countRemovedNumbers = 0;
	std::vector<Field> reducedGame(game);
	int recursionDepthSolve = 0;
	while (solvable)
	{
		int i = mRandom.randInt(0, 80);
		bool wasNumberRemoved = false;
		recursionDepthSolve = 0;
		for (int j = 0; j < 81; j++)
		{
			if (reducedGame[(i + j) % 81].mode == USER && reducedGame[(i + j) % 81].number != 0)
			{
				int removedNumber = reducedGame[(i + j) % 81].number;
				reducedGame[(i + j) % 81].number = 0;
				int solutionCount = backtrackSolve(0, reducedGame, recursionDepthSolve);
				if (solutionCount == 1)
				{
					countRemovedNumbers++;
					wasNumberRemoved = true;
					break;
				}
				else
				{
					reducedGame[(i + j) % 81].number = removedNumber;
				}
			}
		}
		debugPrint("REMOVED NUMBERS: " << countRemovedNumbers << " (Time: " << passedTime(benchmarkTimeStart) << " seconds)" << "\r");
		if (!wasNumberRemoved)
		{
			debugPrint(std::endl);
			solvable = false;
		}
	}
	for (int i = 0; i < 81; i++)
	{
		if (reducedGame[i].mode == USER && reducedGame[i].number > 0)
		{
			game[i].mode = KNOWN;
		}
	}
}

/// <summary>
/// Count solutions for a Str8ts game via an iterative backtracking
/// </summary>
/// <param name="step">Backtracking iteration step</param>
/// <param name="game">Game to be solved</param>
/// <returns>Number of solutions</returns>
int backtrackSolve(int step, std::vector<Field> &game, int &recursionDepth)
{
	if (step == 81)
		return 1;
	if (recursionDepth > MAX_RECURSION_DEPTH_SOLVE) // If recursion depth is to large return and start from the beginning
		return false;
	recursionDepth++;
	if (game[step].mode != USER || game[step].number != 0)
		return backtrackSolve(step + 1, game, recursionDepth);
	std::vector<int> numbers = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	int solutionCount = 0;
	for (int i = 0; i < 9; i++) // Find solutions for all nine possible numbers
	{
		if (checkIfNumberIsAllowed(step, numbers[i], game))
		{
			game[step].number = numbers[i];
			solutionCount += backtrackSolve(step + 1, game, recursionDepth);
		}
	}
	game[step].number = 0;
	return solutionCount;
}

/// <summary>
/// Generate additional known numbers based on a given game field that can be shown to the user to reduce difficulty
/// </summary>
/// <param name="additionalKnownNumbers">Vector where the additional numbers are stored</param>
/// <param name="game">Game for which the additional numbers are generated</param>
/// <param name="game">Number of additional numbers to generate</param>
void generateAdditionalKnownNumbers(std::vector<int>& additionalKnownNumbers, const std::vector<Field>& game, int size)
{
	additionalKnownNumbers.reserve(size);
	int countAdditionalNumbers = 0;
	while (additionalKnownNumbers.size() < size)
	{
		int i = mRandom.randInt(0, 80);
		bool unknownValuesAvailable = false;
		for (int j = 0; j < 81; j++)
		{
			if (game[(i + j) % 81].mode == USER)
			{
				countAdditionalNumbers++;
				additionalKnownNumbers.push_back((i + j) % 81);
				unknownValuesAvailable = true;
				break;
			}
		}
		if (!unknownValuesAvailable)
		{
			debugPrint("WARNING: There are probably too less USER values");
			break;
		}
	}
}

int main()
{
	std::vector<Field> game;
	benchmarkTimeStart = std::chrono::high_resolution_clock::now();
	generate(game);
	std::vector<int> additionalKnownNumbers;
	generateAdditionalKnownNumbers(additionalKnownNumbers, game);

	debugPrint("FINAL GAME: (Time: " << passedTime(benchmarkTimeStart) << " seconds)" << std::endl << gameToString(game));
	debugPrint("ENCODED GAME: ");

	std::string base64 = encodeGame(game, additionalKnownNumbers);
	std::cout << base64 << std::endl;
}
