#include "Test.h"
#include "FileManager.h"
#include "md5.h"
#include <ctime>
#include <cstdio>


namespace TestingSystem {
    namespace Engine {

        std::string Test::getTime() 
        {
            std::time_t now = std::time(nullptr);
            std::tm localTime{};
#ifdef _WIN32
            localtime_s(&localTime, &now);
#else
            localtime_r(&now, &localTime);
#endif
            char buffer[32];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
            return std::string(buffer);
        }

        std::string Test::generateTestID(const std::string& categoryName, const std::string& testName) 
        {
            std::string seed = categoryName + "_" + testName + "_" + std::to_string(std::time(nullptr));
            return md5(seed).substr(0, 12);
        }

        std::vector<Models::CategoryInfo> Test::getCategories()
        {
            return Storage::FileManager::loadCategori();
        }

        std::vector<Models::TestProgress> Test::getProgress(const std::string& userLoginHash) 
        {
            return Storage::FileManager::loadAllProgres(userLoginHash);
        }


        bool Test::Testsession(const std::string& userLoginHash, const std::string& testId,  Models::Test& outTest,  Models::TestProgress& outProgress )
        {
            if (!Storage::FileManager::loadTest(testId, outTest)) 
            {
                return false;
            }

           
            if (!Storage::FileManager::loadProgres(userLoginHash, testId, outProgress))
            {
                outProgress.userLoginHash = userLoginHash;
                outProgress.testID = testId;
                outProgress.categoryName = outTest.header.categoryName;
                outProgress.testName = outTest.header.testName;
                outProgress.currentQuestionIndex = 0;
                outProgress.correctAnswersSoFar = 0;
            }

            if (outProgress.currentQuestionIndex >= outTest.questions.size()) 
            {
                outProgress.currentQuestionIndex = 0;
                outProgress.correctAnswersSoFar = 0;
            }

            return true;
        }

        Models::TestResult Test::finishTest(const Models::User& user, Models::Test& test,  std::int32_t correctAnswers ) 
        {
            Models::TestResult result;
            result.userLoginHash = user.loginHash;
            result.userFullName = user.fullname;
            result.categoryName = test.header.categoryName;
            result.testName = test.header.testName;
            result.correctAnswers = correctAnswers;
            result.totalQuestions = static_cast<std::int32_t>(test.questions.size());
            result.dateTime = getTime();

            result.calculateMetrics();

          
            Storage::FileManager::appendRes(result);
            Storage::FileManager::removeProgres(user.loginHash, test.header.testID);

            test.clearQuestions();

            return result;
        }

        bool Test::pauseTest( const Models::User& user,  Models::Test& test,  std::size_t currentQuestionIndex,  std::int32_t correctAnswersSoFar) 
        {
            Models::TestProgress progress;
            progress.userLoginHash = user.loginHash;
            progress.testID = test.header.testID;
            progress.categoryName = test.header.categoryName;
            progress.testName = test.header.testName;
            progress.currentQuestionIndex = currentQuestionIndex;
            progress.correctAnswersSoFar = correctAnswersSoFar;

            bool saved = Storage::FileManager::saveProgres(progress);

            test.clearQuestions();

            return saved;
        }


        bool Test::createTest( const std::string& categoryName,  const std::string& testName,   const std::vector<Models::Question>& questions, std::string& outGeneratedTestId ) 
        {
            if (categoryName.empty() || testName.empty() || questions.empty()) 
            {
                return false;
            }

            Models::Test newTest;
            outGeneratedTestId = generateTestID(categoryName, testName);
            newTest.header.testID = outGeneratedTestId;
            newTest.header.categoryName = categoryName;
            newTest.header.testName = testName;
            newTest.header.questionCount = questions.size();
            newTest.questions = questions;

            bool success = Storage::FileManager::saveNewTest(newTest);
            newTest.clearQuestions();
            return success;
        }

        bool Test::addQuestion( const std::string& testId,  const Models::Question& question ) 
        {
            if (testId.empty() || question.text.empty() || question.options.size() < 2) 
            {
                return false;
            }
            return Storage::FileManager::appendQuestion(testId, question);
        }

        bool Test::removeTest(const std::string& testId) 
        {
            return Storage::FileManager::deleteTest(testId);
        }

        bool Test::exportCategory(const std::string& categoryName, const std::string& filePath) 
        {
            return Storage::FileManager::exportCategory(categoryName, filePath);
        }

        bool Test::importCategory(const std::string& filePath) 
        {
            return Storage::FileManager::importCategory(filePath);
        }

       

        std::vector<Models::TestResult> Test::getHistory(const std::string& userLoginHash)
        {
            return Storage::FileManager::loadUserResults(userLoginHash);
        }

        std::vector<Models::TestResult> Test::getStatsAll() 
        {
            return Storage::FileManager::loadAllResults();
        }

        std::vector<Models::TestResult> Test::getStatsCategory(const std::string& categoryName) 
        {
            return Storage::FileManager::loadResultsByCategory(categoryName);
        }

        std::vector<Models::TestResult> Test::getStatsTest(const std::string& categoryName, const std::string& testName) 
        {
            return Storage::FileManager::loadResultsByTest(categoryName, testName);
        }

        std::vector<Models::TestResult> Test::getStatUser(const std::string& userLoginHash) 
        {
            return Storage::FileManager::loadUserResults(userLoginHash);
        }

        bool Test::saveFile(const std::vector<Models::TestResult>& results,  const std::string& filePath,  const std::string& reportTitle )
        {
            return Storage::FileManager::exportStatics(results, filePath, reportTitle);
        }

    } 
} 