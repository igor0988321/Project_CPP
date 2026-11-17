#include "DataTypes.h"
#include <cmath>
#include <sstream>

namespace TestingSystem
{
	namespace Models
	{

		std::string escapeField(const std::string& input)
		{
			std::string res;
			res.reserve(input.size());
			for (char ch : input)
			{
				if (ch == '\\')
				{
					res += "\\\\";
				}
				else if (ch == '|')
				{
					res += "\\p";
				}
				else if (ch == '\n')
				{
					res += "\\n";
				}
				else if (ch == '\r')
				{

				}
				else
				{
					res += ch;
				}
			}
			return res;
		}


		std::string unescapeField(const std::string& input)
		{
			std::string res;
			res.reserve(input.size());
			for (std::size_t i = 0; i < input.size(); ++i)
			{
				if (input[i] == '\\' && i + 1 < input.size())
				{
					char next = input[i + 1];

					if (next == '\\')
					{
						res += '\\';
						++i;
					}
					else if (next == 'p')
					{
						res += '|';
						++i;
					}
					else if (next == 'n')
					{
						res += '\n';
						++i;
					}
					else
					{
						res += input[i];
					}
				}
				else
				{
					res += input[i];
				}
				
			}
			return res;
		}

		std::vector<std::string> splitLine(const std::string& line, char del)
		{
			std::vector<std::string> tok;
			std::string current;
			for (char ch : line)
			{
				if (ch == del)
				{
					tok.push_back(current);
					current.clear();
				}
				else
				{
					current += ch;
				}
			}
			tok.push_back(current);
			return tok;
		}


		//USer
		std::string User::serialize() const
		{
			return escapeField(loginHash) + "|" +
				escapeField(passwordHash) + "|" +
				escapeField(fullname) + "|" +
				escapeField(address) + "|" +
				escapeField(phone) + "|" +
				std::to_string(static_cast<int>(role));
		}

		bool User::deserialize(const std::string& line, User& outUser)
		{
			std::vector<std::string> parts = splitLine(line, '|');
			if (parts.size() < 6)
			{
				return false;
			}
			outUser.loginHash = unescapeField(parts[0]);
			outUser.passwordHash = unescapeField(parts[1]);
			outUser.fullname = unescapeField(parts[2]);
			outUser.address = unescapeField(parts[3]);
			outUser.phone = unescapeField(parts[4]);
			int roleInt = std::stoi(parts[5]);
			outUser.role = (roleInt == 0) ? Userrole::Admin : Userrole::Guest;
			return true;
		}


		//Test
		void Test::clearQuestions()
		{
			questions.clear();
			questions.shrink_to_fit();
		}


		//Question
		bool Question::isCorrect(std::int32_t selectIndex) const
		{
			return selectIndex == correctOption;
		}

		std::string Question::serialize() const
		{
			std::string res = escapeField(text) + "|" +
				std::to_string(correctOption) + "|" +
				std::to_string(options.size());
			for (const std::string& opt : options)
			{
				res += "|" + escapeField(opt);
			}
			return res;
		}

		bool Question::deserialize(const std::string& line, Question& outQuestion)
		{
			std::vector<std::string> part = splitLine(line, '|');
			if (part.size() < 3)
			{
				return false;
			}

			outQuestion.text = unescapeField(part[0]);
			outQuestion.correctOption = static_cast<std::int32_t>(std::stoi(part[1]));
			std::size_t optCount = static_cast<std::size_t>(std::stoull(part[2]));

			if (part.size() < 3 + optCount)
			{
				return false;
			}
			outQuestion.options.clear();
			outQuestion.options.reserve(optCount);
			for (std::size_t i = 0; i < optCount; ++i)
			{
				outQuestion.options.push_back(unescapeField(part[3 + i]));
			}
			return true;
		}

		//Header
		std::string TestHeader::serialize() const
		{
			return escapeField(testID) + "|" +
				escapeField(categoryName) + "|" +
				escapeField(testName) + "|" +
				std::to_string(questionCount);
		}

		bool TestHeader::deserialize(const std::string& line, TestHeader& outHeader)
		{
			std::vector<std::string> part = splitLine(line, '|');
			if (part.size() < 4)
			{
				return false;
			}
			outHeader.testID = unescapeField(part[0]);
			outHeader.categoryName = unescapeField(part[1]);
			outHeader.testName = unescapeField(part[2]);
			outHeader.questionCount = static_cast<std::size_t>(std::stoull(part[3]));
			return true;
		}

		//Test Res

		void TestResult::calculateMetrics()
		{
			if (totalQuestions <= 0)
			{
				percentage = 0.0;
				grade12 = 1;
				return;
			}
			percentage = (static_cast<double>(correctAnswers) / static_cast<double>(totalQuestions)) * 100.0;
			int rawGrade = static_cast<int>(std::round((static_cast<double>(correctAnswers) / static_cast<double>(totalQuestions)) * 12.0));
			if (rawGrade < 1)
			{
				rawGrade = 1;
			}
			if (rawGrade > 12)
			{
				rawGrade = 12;
			}
			grade12 = rawGrade;
		}

		std::string TestResult::serialize() const
		{
			return escapeField(userLoginHash) + "|" +
				escapeField(userFullName) + "|" +
				escapeField(categoryName) + "|" +
				escapeField(testName) + "|" +
				std::to_string(correctAnswers) + "|" +
				std::to_string(totalQuestions) + "|" +
				std::to_string(percentage) + "|" +
				std::to_string(grade12) + "|" +
				escapeField(dateTime);
		}


		bool TestResult::deserialize(const std::string& line, TestResult& outResult) {
			std::vector<std::string> parts = splitLine(line, '|');
			if (parts.size() < 9) {
				return false;
			}
			outResult.userLoginHash = unescapeField(parts[0]);
			outResult.userFullName = unescapeField(parts[1]);
			outResult.categoryName = unescapeField(parts[2]);
			outResult.testName = unescapeField(parts[3]);
			outResult.correctAnswers = static_cast<std::int32_t>(std::stoi(parts[4]));
			outResult.totalQuestions = static_cast<std::int32_t>(std::stoi(parts[5]));
			outResult.percentage = std::stod(parts[6]);
			outResult.grade12 = static_cast<std::int32_t>(std::stoi(parts[7]));
			outResult.dateTime = unescapeField(parts[8]);
			return true;
		}

		//Test progress

		std::string TestProgress::serialize() const {
			return escapeField(userLoginHash) + "|" +
				escapeField(testID) + "|" +
				escapeField(categoryName) + "|" +
				escapeField(testName) + "|" +
				std::to_string(currentQuestionIndex) + "|" +
				std::to_string(correctAnswersSoFar);
		}

		bool TestProgress::deserialize(const std::string& line, TestProgress& outProgress) {
			std::vector<std::string> parts = splitLine(line, '|');
			if (parts.size() < 6) {
				return false;
			}
			outProgress.userLoginHash = unescapeField(parts[0]);
			outProgress.testID = unescapeField(parts[1]);
			outProgress.categoryName = unescapeField(parts[2]);
			outProgress.testName = unescapeField(parts[3]);
			outProgress.currentQuestionIndex = static_cast<std::size_t>(std::stoull(parts[4]));
			outProgress.correctAnswersSoFar = static_cast<std::int32_t>(std::stoi(parts[5]));
			return true;
		}
	}
}