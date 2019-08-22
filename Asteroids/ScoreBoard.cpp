#include "ScoreBoard.h"
#include <fstream>
#include <algorithm>

ScoreBoard::ScoreBoard()
{
	highScores_ = new ScoreList();

	std::ifstream fin("HighScores.txt");

	if (fin.good())
	{
		try
		{
			std::string line, name;
			int newScore;
			while (std::getline(fin, line))
			{
				name = line.substr(0, line.find(":") - 1);
				newScore = std::stoi(line.substr(line.find(":") + 1, line.length() - 1));

				highScores_->push_back(std::make_pair(name, newScore));
			}
		}
		catch (std::exception e)
		{

		}
	}
}

ScoreBoard::~ScoreBoard()
{
	highScores_->clear();
	delete highScores_;
}

const ScoreBoard::ScoreList* ScoreBoard::GetHighScores() const
{
	return highScores_;
}

bool sortbysec(const std::pair<std::string, int>& a,
	const std::pair<std::string, int>& b)
{
	return (a.second > b.second);
}

void ScoreBoard::AddScore(std::string name, int score)
{
	highScores_->push_back(std::make_pair(name, score));

	std::sort(highScores_->begin(), highScores_->end(), sortbysec);

	std::ofstream fout("HighScores.txt");

	int numScores = 0;
	for (auto score = highScores_->begin(); score != highScores_->end() && numScores < 10; score++, numScores++)
	{
		fout << score->first << " : " << score->second << std::endl;
	}

	fout.close();
}

