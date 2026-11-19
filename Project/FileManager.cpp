#include "FileManager.h"
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <algorithm>


namespace TestingSystem
{
	namespace Storage
	{
        const std::string FileManager::Data = "data";
        const std::string FileManager::Tests = "data/tests";
        const std::string FileManager::Users_File = "data/users.db";
        const std::string FileManager::Index_File= "data/tests_index.db";
        const std::string FileManager::Results_File = "data/results.db";
        const std::string FileManager::Progress_File = "data/progress.db";



        void FileManager::initStorage()
        {
            std::filesystem::create_directories(Data);
            std::filesystem::create_directories(Tests);

            const std::string files[] = { Users_File, Index_File, Results_File, Progress_File };
            for (const std::string& path : files) {
                if (!std::filesystem::exists(path)) {
                    std::ofstream out(path);
                }
            }
        }

        std::string FileManager::getTest(const std::string& testID)
        {
            return Tests + "/" + testID + ".db";
        }

        //user

        bool FileManager::isAdmin()
        {
            std::ifstream in(Users_File);
            if (!in.is_open())
            {
                return false;
            }
            std::string line;
            Models::User user;
            while (std::getline(in, line))
            {
                if (line.empty())
                {
                    continue;
                }
                if (Models::User::deserialize(line, user))
                {
                    if (user.role == Models::Userrole::Admin)
                    {
                        return true;
                    }
                }
            }
        }


        bool FileManager::isLogin(const std::string& loginHash)
        {
            Models::User d;
            return findUser(loginHash, d);
        }


        bool FileManager::findUser(const std::string& loginHash, Models::User& User)
        {
            std::ifstream in(Users_File);
            if (!in.is_open())
            {
                return false;
            }

            std::string line;
            Models::User curr;
            while (std::getline(in, line))
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::User::deserialize(line, curr))
                {
                    if (curr.loginHash == loginHash)
                    {
                        User = curr;
                        return true;
                    }
                }
            }
            return false;
        }



        bool FileManager::addUSer(const Models::User& user)
        {
            if (isLogin(user.loginHash))
            {
                return false;
            }

            std::ofstream out(Users_File, std::ios::app);
            if (!out.is_open())
            {
                return false;
            }

            out << user.serialize() << "\n";
            return true;
        }


        bool FileManager::rewriteUsers(const std::vector<Models::User>& users)
        {
            std::ofstream out(Users_File, std::ios::trunc);
            if (!out.is_open())
            {
                return false;
            }

            for (const Models::User& u : users)
            {
                out << u.serialize() << "\n";
            }
            return true;
        }


        bool FileManager::updateUser(const Models::User& updatedUser) 
        {
            std::ifstream in(Users_File);
            if (!in.is_open())
            {
                return false;
            }

            std::vector<Models::User> allUsers;
            std::string line;
            Models::User current;
            bool found = false;

            while (std::getline(in, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::User::deserialize(line, current))
                {
                    if (current.loginHash == updatedUser.loginHash) 
                    {
                        allUsers.push_back(updatedUser);
                        found = true;
                    }
                    else 
                    {
                        allUsers.push_back(current);
                    }
                }
            }
            in.close();

            if (!found)
            {
                return false;
            }
            return rewriteUsers(allUsers);
        }

        bool FileManager::deleteUSer(const std::string& loginHash)
        {
            std::ifstream in(Users_File);
            if (!in.is_open())
            {
                return false;
            }

            std::vector<Models::User> remainingUsers;
            std::string line;
            Models::User current;
            bool deleted = false;

            while (std::getline(in, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::User::deserialize(line, current)) 
                {
                    if (current.loginHash == loginHash && current.role != Models::Userrole::Admin) 
                    {
                        deleted = true;
                    }
                    else 
                    {
                        remainingUsers.push_back(current);
                    }
                }
            }
            in.close();

            if (!deleted)
            {
                return false;
            }

            return rewriteUsers(remainingUsers);
        }

        std::vector<Models::User> FileManager::loadAllGuests() 
        {
            std::vector<Models::User> guests;
            std::ifstream in(Users_File);
            if (!in.is_open())
            {
                return guests;
            }

            std::string line;
            Models::User current;
            while (std::getline(in, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::User::deserialize(line, current)) 
                {
                    if (current.role == Models::Userrole::Guest) 
                    {
                        guests.push_back(current);
                    }
                }
            }
            return guests;
        }

        //test

        std::vector<Models::CategoryInfo> FileManager::loadCategori() 
        {
            std::vector<Models::CategoryInfo> categories;
            std::ifstream in(Index_File);
            if (!in.is_open())
            {
                return categories;
            }


            std::string line;
            Models::TestHeader header;
            while (std::getline(in, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestHeader::deserialize(line, header)) 
                {
                    bool catFound = false;

                    for (Models::CategoryInfo& cat : categories)
                    {
                        if (cat.name == header.categoryName) 
                        {
                            cat.tests.push_back(header);
                            catFound = true;
                            break;
                        }
                    }
                    if (!catFound) 
                    {
                        Models::CategoryInfo newCat;
                        newCat.name = header.categoryName;
                        newCat.tests.push_back(header);
                        categories.push_back(newCat);
                    }
                }
            }
            return categories;
        }

        bool FileManager::loadTest(const std::string& testId, Models::Test& outTest) 
        {
            
            std::ifstream idxIn(Index_File);
            if (!idxIn.is_open())
            {
                return false;
            }

            std::string line;
            Models::TestHeader header;
            bool headerFound = false;

            while (std::getline(idxIn, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestHeader::deserialize(line, header)) {

                    if (header.testID == testId) 
                    {
                        outTest.header = header;
                        headerFound = true;
                        break;
                    }
                }
            }
            idxIn.close();
            if (!headerFound)
            {
                return false;
            }


            
            std::ifstream testIn(getTest(testId));
            if (!testIn.is_open())
            {
                return false;
            }


            outTest.questions.clear();
            outTest.questions.reserve(outTest.header.questionCount);
            Models::Question q;
            while (std::getline(testIn, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::Question::deserialize(line, q)) 
                {
                    outTest.questions.push_back(q);
                }
            }
            return true;
        }

        bool FileManager::rewriteTests(const std::vector<Models::TestHeader>& headers) 
        {
            std::ofstream out(Index_File, std::ios::trunc);
            if (!out.is_open())
            {
                return false;
            }


            for (const Models::TestHeader& h : headers) 
            {
                out << h.serialize() << "\n";
            }
            return true;
        }

        bool FileManager::saveNewTest(const Models::Test& test) 
        {
            Models::TestHeader header = test.header;
            header.questionCount = test.questions.size();

            std::ofstream testOut(getTest(header.testID), std::ios::trunc);
            if (!testOut.is_open())
            {
                return false;
            }


            for (const Models::Question& q : test.questions) 
            {
                testOut << q.serialize() << "\n";
            }
            testOut.close();

            
            std::ifstream idxIn(Index_File);
            std::vector<Models::TestHeader> headers;
            std::string line;
            Models::TestHeader current;
            bool updated = false;

            while (std::getline(idxIn, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestHeader::deserialize(line, current)) 
                {
                    if (current.testID == header.testID) 
                    {
                        headers.push_back(header);
                        updated = true;
                    }
                    else 
                    {
                        headers.push_back(current);
                    }
                }
            }
            idxIn.close();

            if (!updated) 
            {
                headers.push_back(header);
            }
            return rewriteTests(headers);
        }

        bool FileManager::appendQuestion(const std::string& testId, const Models::Question& question) 
        {
            std::ofstream testOut(getTest(testId), std::ios::app);
            if (!testOut.is_open())
            {
                return false;
            }


            testOut << question.serialize() << "\n";
            testOut.close();

           
            std::ifstream idxIn(Index_File);
            std::vector<Models::TestHeader> headers;
            std::string line;
            Models::TestHeader current;
            bool found = false;

            while (std::getline(idxIn, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestHeader::deserialize(line, current)) 
                {
                    if (current.testID == testId) 
                    {
                        current.questionCount += 1;
                        found = true;
                    }
                    headers.push_back(current);
                }
            }
            idxIn.close();

            if (!found)
            {
                return false;
            }

            return rewriteTests(headers);
        }

        bool FileManager::deleteTest(const std::string& testId) {
            std::ifstream idxIn(Index_File);
            if (!idxIn.is_open())
            {
                return false;
            }


            std::vector<Models::TestHeader> headers;
            std::string line;
            Models::TestHeader current;
            bool removed = false;

            while (std::getline(idxIn, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestHeader::deserialize(line, current)) 
                {
                    if (current.testID == testId) 
                    {
                        removed = true;
                    }
                    else 
                    {
                        headers.push_back(current);
                    }
                }
            }
            idxIn.close();

            if (!removed)
            {
                return false;
            }
            std::filesystem::remove(getTest(testId));
            return rewriteTests(headers);
        }

        bool FileManager::exportCategory(const std::string& categoryName, const std::string& exportFilePath)
        {
            std::vector<Models::CategoryInfo> categories = loadCategori();
            const Models::CategoryInfo* targetCat = nullptr;

            for (const Models::CategoryInfo& cat : categories) 
            {
                if (cat.name == categoryName) 
                {
                    targetCat = &cat;
                    break;
                }
            }
            if (targetCat == nullptr)
            {
                return false;
            }


            std::ofstream out(exportFilePath, std::ios::trunc);
            if (!out.is_open())
            {
                return false;
            }


            out << "CATEGORY|" << Models::escapeField(targetCat->name) << "|" << targetCat->tests.size() << "\n";

            for (const Models::TestHeader& h : targetCat->tests) 
            {
                Models::Test fullTest;
                if (loadTest(h.testID, fullTest)) 
                {
                    out << "TEST|" << fullTest.header.serialize() << "\n";
                    for (const Models::Question& q : fullTest.questions) 
                    {
                        out << "QUESTION|" << q.serialize() << "\n";
                    }
                    fullTest.clearQuestions(); 
                }
            }
            return true;
        }

        bool FileManager::importCategory(const std::string& importFilePath) 
        {
            std::ifstream in(importFilePath);
            if (!in.is_open())
            {
                return false;
            }


            std::string line;
            Models::Test currentTest;
            bool hasActiveTest = false;

            while (std::getline(in, line))
            {
                if (line.empty())
                {
                    continue;
                }


                if (line.rfind("TEST|", 0) == 0) 
                {
                    if (hasActiveTest)
                    {
                        saveNewTest(currentTest);
                        currentTest.clearQuestions();
                    }
                    std::string payload = line.substr(5);
                    if (Models::TestHeader::deserialize(payload, currentTest.header)) 
                    {
                        currentTest.questions.clear();
                        hasActiveTest = true;
                    }
                }
                else if (line.rfind("QUESTION|", 0) == 0 && hasActiveTest)
                {
                    std::string payload = line.substr(9);
                    Models::Question q;
                    if (Models::Question::deserialize(payload, q))
                    {
                        currentTest.questions.push_back(q);
                    }
                }
            }

            if (hasActiveTest) 
            {
                saveNewTest(currentTest);
                currentTest.clearQuestions();
            }
            return true;
        }

        // res

        bool FileManager::appendRes(const Models::TestResult& result) 
        {
            std::ofstream out(Results_File, std::ios::app);
            if (!out.is_open())
            {
                return false;
            }

            out << result.serialize() << "\n";
            return true;
        }

        std::vector<Models::TestResult> FileManager::loadUserResults(const std::string& userLoginHash) 
        {
            std::vector<Models::TestResult> userResults;
            std::ifstream in(Results_File);
            if (!in.is_open())
            {
                return userResults;
            }


            std::string line;
            Models::TestResult current;
            while (std::getline(in, line))
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestResult::deserialize(line, current)) 
                {
                    if (current.userLoginHash == userLoginHash) 
                    {
                        userResults.push_back(current); 
                    }
                }
            }
            return userResults;
        }

        std::vector<Models::TestResult> FileManager::loadResultsByCategory(const std::string& categoryName) 
        {
            std::vector<Models::TestResult> filtered;
            std::ifstream in(Results_File);
            if (!in.is_open())
            {
                return filtered;
            }

            std::string line;
            Models::TestResult current;
            while (std::getline(in, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestResult::deserialize(line, current)) 
                {
                    if (current.categoryName == categoryName) 
                    {
                        filtered.push_back(current);
                    }
                }
            }
            return filtered;
        }

        std::vector<Models::TestResult> FileManager::loadResultsByTest(const std::string& categoryName, const std::string& testName) 
        {
            std::vector<Models::TestResult> filtered;
            std::ifstream in(Results_File);
            if (!in.is_open())
            {
                return filtered;
            }

            std::string line;
            Models::TestResult current;
            while (std::getline(in, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestResult::deserialize(line, current)) 
                {
                    if (current.categoryName == categoryName && current.testName == testName) 
                    {
                        filtered.push_back(current);
                    }
                }
            }
            return filtered;
        }

        std::vector<Models::TestResult> FileManager::loadAllResults() 
        {
            std::vector<Models::TestResult> all;
            std::ifstream in(Results_File);
            if (!in.is_open())
            {
                return all;
            }

            std::string line;
            Models::TestResult current;
            while (std::getline(in, line))
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestResult::deserialize(line, current)) 
                {
                    all.push_back(current);
                }
            }
            return all;
        }

        bool FileManager::exportStatics(const std::vector<Models::TestResult>& results, const std::string& filePath, const std::string& reportTitle) 
        {
            std::ofstream out(filePath, std::ios::trunc);
            if (!out.is_open())
            {
                return false;
            }

            out << "====================================================================\n";
            out << "ЗВІТ СТАТИСТИКИ ТЕСТУВАННЯ: " << reportTitle << "\n";
            out << "====================================================================\n\n";

            if (results.empty()) 
            {
                out << "Записи за вказаним фільтром відсутні.\n";
                return true;
            }

            for (std::size_t i = 0; i < results.size(); ++i)
            {
                const Models::TestResult& r = results[i];
                out << (i + 1) << ". Користувач: " << r.userFullName << "\n"
                    << "   Категорія:  " << r.categoryName << " -> Тест: " << r.testName << "\n"
                    << "   Результат:  " << r.correctAnswers << " / " << r.totalQuestions
                    << " (" << std::fixed << std::setprecision(1) << r.percentage << "%)"
                    << " | Оцінка (12-бальна): " << r.grade12 << "\n"
                    << "   Дата:       " << r.dateTime << "\n"
                    << "--------------------------------------------------------------------\n";
            }
            return true;
        }

        //progress

        bool FileManager::saveProgres(const Models::TestProgress& progress)
        {
            std::ifstream in(Progress_File);
            std::vector<Models::TestProgress> allProgress;
            std::string line;
            Models::TestProgress current;
            bool updated = false;

            if (in.is_open()) 
            {
                while (std::getline(in, line)) 
                {
                    if (line.empty())
                    {
                        continue;
                    }

                    if (Models::TestProgress::deserialize(line, current)) 
                    {
                        if (current.userLoginHash == progress.userLoginHash && current.testID == progress.testID)
                        {
                            allProgress.push_back(progress);
                            updated = true;
                        }
                        else 
                        {
                            allProgress.push_back(current);
                        }
                    }
                }
                in.close();
            }

            if (!updated) 
            {
                allProgress.push_back(progress);
            }

            std::ofstream out(Progress_File, std::ios::trunc);
            if (!out.is_open())
            {
                return false;
            }

            for (const Models::TestProgress& p : allProgress)
             {
                out << p.serialize() << "\n";
            }
            return true;
        }

        bool FileManager::loadProgres(const std::string& userLoginHash, const std::string& testId, Models::TestProgress& outProgress)
        {
            std::ifstream in(Progress_File);
            if (!in.is_open())
            {
                return false;
            }

            std::string line;
            Models::TestProgress current;
            while (std::getline(in, line))
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestProgress::deserialize(line, current)) 
                {
                    if (current.userLoginHash == userLoginHash && current.testID == testId)
                    {
                        outProgress = current;
                        return true;
                    }
                }
            }
            return false;
        }

        std::vector<Models::TestProgress> FileManager::loadAllProgres(const std::string& userLoginHash) 
        {
            std::vector<Models::TestProgress> list;
            std::ifstream in(Progress_File);
            if (!in.is_open())
            {
                return list;
            }


            std::string line;
            Models::TestProgress current;
            while (std::getline(in, line))
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestProgress::deserialize(line, current)) 
                {
                    if (current.userLoginHash == userLoginHash) 
                    {
                        list.push_back(current);
                    }
                }
            }
            return list;
        }

        bool FileManager::removeProgres(const std::string& userLoginHash, const std::string& testId) 
        {
            std::ifstream in(Progress_File);
            if (!in.is_open())
            {
                return false;
            }


            std::vector<Models::TestProgress> remaining;
            std::string line;
            Models::TestProgress current;
            bool removed = false;

            while (std::getline(in, line)) 
            {
                if (line.empty())
                {
                    continue;
                }

                if (Models::TestProgress::deserialize(line, current)) 
                {
                    if (current.userLoginHash == userLoginHash && current.testID == testId) 
                    {
                        removed = true;
                    }
                    else 
                    {
                        remaining.push_back(current);
                    }
                }
            }
            in.close();

            if (!removed)
            {
                return false;
            }


            std::ofstream out(Progress_File, std::ios::trunc);
            if (!out.is_open())
            {
                return false;
            }

            for (const Models::TestProgress& p : remaining) 
            {
                out << p.serialize() << "\n";
            }
            return true;
        }
	}
}