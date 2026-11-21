#pragma once
#include "DataTypes.h"
#include <string>
#include <vector>


namespace TestingSystem
{
	namespace Engine
	{
		class Test
		{
			static std::string getTime();
			static std::string generateTestID(const std::string& category, const std::string& name);
		public:
			static std::vector<Models::CategoryInfo> getCategories();

			static std::vector<Models::TestProgress> getProgress(const std::string& userLogin);

			static bool Testsession(const std::string& userLogin, const std::string& testID, Models::Test& Test, Models::TestProgress& Progress);

			static Models::TestResult finishTest(const Models::User& user, Models::Test& test, std::int32_t Answers);

			static bool pauseTest(const Models::User& user, Models::Test& test, std::size_t QuestionIndex, std::int32_t answer);

			static bool createTest(const std::string& category, const std::string& testName, const std::vector<Models::Question>& questions, std::string& TestID);

			static bool addQuestion(const std::string& testID, const Models::Question& question);

			static bool removeTest(const std::string& testID);

			static bool exportCategory(const std::string& category, const std::string& file);

			static bool importCategory(const std::string& file);

			static std::vector<Models::TestResult> getHistory(const std::string& Login);

			static std::vector<Models::TestResult> getStatsAll();

			static std::vector<Models::TestResult> getStatsCategory(const std::string& category);
			
			static std::vector<Models::TestResult> getStatsTest(const std::string& category, const std::string& test);

			static std::vector<Models::TestResult> getStatUser(const std::string& userLogin);
			
			static bool saveFile(const std::vector<Models::TestResult>& res, const std::string& file, const std::string& title);


		};
	}
}