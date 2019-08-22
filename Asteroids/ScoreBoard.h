#pragma once
#include <string>
#include <vector>

class ScoreBoard
{
public:
	ScoreBoard();
	~ScoreBoard();

public:
	const std::vector<std::pair<std::string, int>>* GetHighScores() const;
	void AddScore(std::string name, int score);

private:
	typedef std::vector<std::pair<std::string, int>> ScoreList; 
	
	ScoreList* highScores_;
};

