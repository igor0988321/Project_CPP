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

	}
}