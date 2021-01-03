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
	USER, KNOWN, BLACK, BLACKKNOWN
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
constexpr int FIND_KNOWN_BLACK_FIELDS_RETRIES = 2000;
constexpr int ENCODING_VERSION = 2;
constexpr int DEFAULT_DIFFICULTY = 3;
constexpr char BASE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"; // 64 characters used for the base64url encoding

void generate(std::vector<Field>& game, int difficulty, int generatorCount = 0, int numberGeneratorCount = 0);
bool checkIfNumberIsAllowed(const int position, const int number, const std::vector<Field>& game);
bool backtrackCreate(int step, std::vector<Field>& game, int& recursionDepth);
int backtrackSolve(int step, std::vector<Field>& game, int& recursionDepth);

double passedTime(const std::chrono::high_resolution_clock::time_point startTime);
std::string gameToString(const std::vector<Field>& game);
std::string encodeGame(const std::vector<Field>& game);

Random mRandom;
std::chrono::high_resolution_clock::time_point benchmarkTimeStart;
