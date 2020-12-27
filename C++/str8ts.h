#pragma once

#include <chrono>
#include <random>
#include <vector>
#include <iostream>

#ifndef DEBUG
	#define DEBUG 1
#endif

#define debugPrint(message) if (DEBUG) std::cout << message


enum Mode
{
	USER, BLACK, KNOWN, BLACKKNOWN
};

struct Field
{
	Mode mode;
	int number;
};

class Random
{
public:
	Random();
	int randInt(const int start, const int end);
	void shuffle(std::vector<int>::iterator begin, std::vector<int>::iterator end);
private:
	std::random_device rd;
	std::mt19937 mt;
};

constexpr int MAX_RECURSION_DEPTH_CREATE = 300000;
constexpr int MAX_RECURSION_DEPTH_SOLVE = 800000;
constexpr int FIND_KNOWN_BLACK_FIELDS_RETRIES = 1000;
constexpr int ENCODING_VERSION = 1;
constexpr char BASE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"; // 64 characters used for the base64url encoding

void generate(std::vector<Field>& game, int generatorCount = 0, int numberGeneratorCount = 0);
bool checkIfNumberIsAllowed(const int position, const int number, const std::vector<Field>& game);
bool backtrackCreate(int step, std::vector<Field>& game, int& recursionDepth);
int backtrackSolve(int step, std::vector<Field>& game, int& recursionDepth);
void generateAdditionalKnownNumbers(std::vector<int>& additionalKnownNumbers, const std::vector<Field>& game, int size = 20);

double passedTime(const std::chrono::high_resolution_clock::time_point startTime);
std::string gameToString(const std::vector<Field>& game);
std::string encodeGame(const std::vector<Field>& game, const std::vector<int>& reducedDifficulty = {});

Random mRandom;
std::chrono::high_resolution_clock::time_point benchmarkTimeStart;
