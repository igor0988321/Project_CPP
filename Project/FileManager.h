#pragma once
#include "DataTypes.h"
#include <string>
#include <vector>



namespace TestingSystem
{
	namespace Storage
	{

		class FileManager
		{
			static const std::string Data;
			static const std::string Tests;
			static const std::string Users_File;
			static const std::string Index_File;
			static const std::string Results_File;
			static const std::string Progress_File;


			static std::string getTest(const std::string& testID);
			static bool rewriteUsers(const std::vector<Models::User>& users);
			static bool rewriteTests(const std::vector<Models::TestHeader>& headers);

		public:

			static void initStorage();


			//Users

			static bool isAdmin();
			static bool isLogin(const std::string& loginHash);
			static bool findUser(const std::string& loginHash, Models::User& User);
			static bool addUSer(const Models::User& user);
			static bool updateUser(const Models::User& updateUser);
			static bool deleteUSer(const std::string& loginHash);
			static std::vector<Models::User> loadAllGuests();


			//Test

			static std::vector<Models::CategoryInfo> loadCategori();
			static bool loadTest(const std::string& testID, Models::Test& Test);
			static bool saveNewTest(const Models::Test& test);
			static bool appendQuestion(const std::string& testID, const Models::Question& question);
			static bool deleteTest(const std::string& testID);

			static bool exportCategory(const std::string& categoryName, const std::string& exportFile);
			static bool importCategory(const std::string& importFile);

			//result

			static bool appendRes(const Models::TestResult& result);
			static std::vector<Models::TestResult> loadUserResults(const std::string& userLoginHash);
			static std::vector<Models::TestResult> loadResultsByCategory(const std::string& categoryName);
			static std::vector<Models::TestResult> loadResultsByTest(const std::string& categoryName, const std::string& testName);
			static std::vector<Models::TestResult> loadAllResults();
			static bool exportStatics(const std::vector<Models::TestResult>& result, const std::string& file, const std::string& report);

			// pause

			static bool saveProgres(const Models::TestProgress& progress);
			static bool loadProgres(const std::string& userHash, const std::string& testID, Models::TestProgress& Progress);
			static std::vector<Models::TestProgress> loadAllProgres(const std::string& userHash);
			static bool removeProgres(const std::string& userHash, const std::string& testID);
		};
	}
}
