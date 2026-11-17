#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace  TestingSystem
{
	namespace Models
	{
		enum class Userrole : std::uint8_t
		{
			Admin = 0,
			Guest = 1
		};

		std::string escapeField(const std::string& input);
		std::string unescapeField(const std::string& input);
		std::vector<std::string> splitLine(const std::string& line, char del = '|');

		//профіль користувача
		struct User
		{
			std::string loginHash;
			std::string passwordHash;
			std::string fullname;
			std::string address;
			std::string phone;
			Userrole role = Userrole::Guest;

			std::string serialize() const;
			static bool deserialize(const std::string& line, User& outUser);
		};

		//питання тестуї
		struct Question
		{
			std::string text;
			std::vector<std::string> options;
			std::int32_t correctOption = 0;

			bool isCorrect(std::int32_t selected) const;
			std::string serialize() const;
			static bool deserialize(const std::string& line, Question& outQuestion);
		};

		//заголовок тексту
		struct TestHeader
		{
			std::string testID;
			std::string categoryName;
			std::string testName;
			std::size_t questionCount = 0;

			std::string serialize() const;
			static bool deserialize(const std::string& line, TestHeader& outHeader);
		};

		//тест
		struct Test
		{
			TestHeader header;
			std::vector<Question> questions;

			void clearQuestions();
		};

		//категорія
		struct CategoryUnfo
		{
			std::string name;
			std::vector<TestHeader> tests;
		};

		//Результат 
		struct TestResult
		{

			std::string userLoginHash;
			std::string userFullName;
			std::string categoryName;
			std::string testName;
			std::int32_t correctAnswers = 0;
			std::int32_t totalQuestions = 0;
			double percentage = 0.0;
			std::int32_t grade12 = 1;
			std::string dataTime;

			void calculateMetrics();
			std::string serialize() const;
			static bool eserialize(const std::string& line, TestResult& outResult);
		};

		//незавершений тест
		struct TestProgress
		{
			std::string userLoginHash;
			std::string testID;
			std::string categoryName;
			std::string testName;
			std::size_t currentQuestionIndex = 0;
			std::int32_t correctAnswerSoFar = 0;

			std::string serialize() const;
			static bool deserialize(const std::string& line, TestProgress& outProgress);
		};
	}
}
