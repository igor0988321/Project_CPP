#include "App.h"
#include "DataTypes.h"
#include "FileManager.h"
#include "Manag.h"
#include "Test.h"
#include "ConsoleMenu.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include <windows.h>

namespace TestingSystem {
    namespace UI {

        namespace {

            std::string padRightUtf8(const std::string& str, std::size_t width)
            {
                std::size_t charCount = 0;
                std::size_t bytePos = 0;
                std::size_t cutBytePos = str.size();

                while (bytePos < str.size())
                {
                    unsigned char c = static_cast<unsigned char>(str[bytePos]);
                    std::size_t charLen = 1;
                    if ((c & 0x80) == 0x00) charLen = 1;
                    else if ((c & 0xE0) == 0xC0) charLen = 2;
                    else if ((c & 0xF0) == 0xE0) charLen = 3;
                    else if ((c & 0xF8) == 0xF0) charLen = 4;

                    if (width > 3 && charCount == width - 3)
                    {
                        cutBytePos = bytePos;
                    }
                    if (charCount == width)
                    {
                        return str.substr(0, cutBytePos) + "...";
                    }

                    bytePos += charLen;
                    charCount++;
                }

                return str + std::string(width - charCount, ' ');
            }

            void clearBuffer() 
            {
                std::cin.clear();
                std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            }

            int readIntInRange(const std::string& prompt, int minVal, int maxVal) 
            {
                int value = 0;
                while (true) {
                    std::cout << prompt;
                    if (std::cin >> value && value >= minVal && value <= maxVal) 
                    {
                        clearBuffer();
                        return value;
                    }
                    std::cout << "Помилка! Введіть число від " << minVal << " до " << maxVal << ".\n";
                    clearBuffer();
                }
            }

            
            void seedDefaultTestsIfEmpty() 
            {
                if (!Engine::Test::getCategories().empty()) 
                {
                    return;
                }

                std::string dummyId;

                std::vector<Models::Question> discMath = {
                    { "Скільки підмножин має множина з 3 елементів?", { "3", "6", "8", "9" }, 2 },
                    { "Яка операція позначається символом ^ (кон'юнкція)?", { "Логічне І", "Логічне АБО", "Заперечення", "Імплікація" }, 0 },
                    { "Граф без циклів, який є зв'язним, називається:", { "Мультиграф", "Дерево", "Орієнтований граф", "Повний граф" }, 1 }
                };
                Engine::Test::createTest("Математика", "Дискретна математика", discMath, dummyId);

                std::vector<Models::Question> mathAnalysis = {
                    { "Чому дорівнює похідна від константи C?", { "1", "C", "0", "Нескінченність" }, 2 },
                    { "Яка функція є первісною для cos(x)?", { "-sin(x)", "sin(x)", "tg(x)", "-cos(x)" }, 1 }
                };
                Engine::Test::createTest("Математика", "Математичний аналіз", mathAnalysis, dummyId);

                std::vector<Models::Question> quantumPhysics = {
                    { "Хто ввів поняття кванта енергії?", { "Альберт Ейнштейн", "Макс Планк", "Нільс Бор", "Ернест Резерфорд" }, 1 },
                    { "Як називається квант електромагнітного випромінювання?", { "Електрон", "Протон", "Фотон", "Нейтрон" }, 2 }
                };
                Engine::Test::createTest("Фізика", "Квантова фізика", quantumPhysics, dummyId);

                std::vector<Models::Question> mechanics = {
                    { "Одиниця вимірювання сили в системі СІ:", { "Джоуль", "Ват", "Ньютон", "Паскаль" }, 2 },
                    { "Формула другого закону Ньютона:", { "F = m * a", "E = m * c^2", "A = F * s", "P = m * g" }, 0 }
                };
                Engine::Test::createTest("Фізика", "Механіка", mechanics, dummyId);
            }

           
            void handleFirstRunAdminSetup() 
            {
                system("cls");
                std::cout << "====================================================\n";
                std::cout << "  ПЕРШИЙ ЗАПУСК СИСТЕМИ ТЕСТУВАННЯ\n";
                std::cout << "  Створіть обліковий запис єдиного Адміністратора\n";
                std::cout << "====================================================\n\n";

                while (true) 
                {
                    std::string login, password;
                    std::cout << "Введіть логін Адміністратора: ";
                    std::getline(std::cin, login);
                    std::cout << "Введіть пароль Адміністратора: ";
                    std::getline(std::cin, password);

                    if (Auth::Manag::InitialAdmin(login, password)) 
                    {
                        std::cout << "\nАдміністратора успішно створено! Логін і пароль зашифровано.\n";
                        system("pause");
                        break;
                    }
                    std::cout << "Помилка! Логін і пароль не можуть бути порожніми.\n\n";
                }
            }


            void runInteractiveTest(const Models::User& user, const std::string& testId)
            {
                Models::Test currentTest;
                Models::TestProgress progress;

                if (!Engine::Test::Testsession(user.loginHash, testId, currentTest, progress))
                {
                    std::cout << "Не вдалося завантажити обраний тест.\n";
                    system("pause");
                    return;
                }

                if (currentTest.questions.empty()) 
                {
                    std::cout << "У цьому тесті поки що немає питань.\n";
                    currentTest.clearQuestions();
                    system("pause");
                    return;
                }

                if (progress.currentQuestionIndex > 0)
                {
                    system("cls");
                    std::cout << "Знайдено збережену паузу для тесту \"" << currentTest.header.testName << "\".\n";
                    std::cout << "Продовжуємо з питання №" << (progress.currentQuestionIndex + 1)
                        << " із " << currentTest.questions.size() << ".\n\n";
                    system("pause");
                }

                std::size_t qIdx = progress.currentQuestionIndex;
                std::int32_t correctCount = progress.correctAnswersSoFar;

                while (qIdx < currentTest.questions.size())
                {
                    system("cls");
                    const Models::Question& q = currentTest.questions[qIdx];

                    std::cout << "Категорія: " << currentTest.header.categoryName
                        << " | Тест: " << currentTest.header.testName << "\n";
                    std::cout << "Питання " << (qIdx + 1) << " з " << currentTest.questions.size() << "\n";
                    std::cout << "----------------------------------------------------\n";
                    std::cout << q.text << "\n\n";

                    std::vector<std::string> menuOptions = q.options;
                    menuOptions.push_back("[ ПАУЗА: Зупинити тест і продовжити пізніше ]");

                    int choice = ConsoleMenu::Menu::verticalMenu(
                        "Оберіть відповідь (стрілки Вгору/Вниз + Enter):",
                        menuOptions,
                        ConsoleMenu::Left,
                        10
                    );

                    if (choice == -1 || choice == static_cast<int>(q.options.size())) 
                    {
                        Engine::Test::pauseTest(user, currentTest, qIdx, correctCount);
                        std::cout << "Тест зупинено. Прогрес збережено!\n";
                        system("pause");
                        return;
                    }

                    if (q.isCorrect(choice)) 
                    {
                        correctCount++;
                    }
                    qIdx++;
                }

                Models::TestResult res = Engine::Test::finishTest(user, currentTest, correctCount);

                system("cls");
                std::cout << "====================================================\n";
                std::cout << "  ТЕСТУВАННЯ ЗАВЕРШЕНО!\n";
                std::cout << "====================================================\n";
                std::cout << "Категорія:             " << res.categoryName << "\n";
                std::cout << "Тест:                  " << res.testName << "\n";
                std::cout << "Правильних відповідей: " << res.correctAnswers << " з " << res.totalQuestions << "\n";
                std::cout << "Відсоток успішності:   " << std::fixed << std::setprecision(1) << res.percentage << "%\n";
                std::cout << "Оцінка (12-бальна):    " << res.grade12 << " / 12\n";
                std::cout << "====================================================\n\n";
                system("pause");
            }

            void printResultsList(const std::vector<Models::TestResult>& results, const std::string& title)
            {
                system("cls");
                std::cout << "=== " << title << " ===\n\n";
                if (results.empty())
                {
                    std::cout << "Результатів поки що немає.\n\n";
                    system("pause");
                    return;
                }

                const std::string border = "+----+----------------------+--------------------+------------------------+-----------+--------+---------------------+";
                std::cout << border << "\n";
                std::cout << "| " << padRightUtf8("№", 2)
                    << " | " << padRightUtf8("Ім'я (ПІБ)", 20)
                    << " | " << padRightUtf8("Категорія", 18)
                    << " | " << padRightUtf8("Тест", 22)
                    << " | " << padRightUtf8("Результат", 9)
                    << " | " << padRightUtf8("Оцінка", 6)
                    << " | " << padRightUtf8("Дата та час", 19) << " |\n";
                std::cout << border << "\n";

                for (std::size_t i = 0; i < results.size(); ++i)
                {
                    const Models::TestResult& r = results[i];
                    std::string resStr = std::to_string(r.correctAnswers) + "/" + std::to_string(r.totalQuestions);
                    std::string gradeStr = std::to_string(r.grade12) + "/12";

                    std::cout << "| " << padRightUtf8(std::to_string(i + 1), 2)
                        << " | " << padRightUtf8(r.userFullName, 20)
                        << " | " << padRightUtf8(r.categoryName, 18)
                        << " | " << padRightUtf8(r.testName, 22)
                        << " | " << padRightUtf8(resStr, 9)
                        << " | " << padRightUtf8(gradeStr, 6)
                        << " | " << padRightUtf8(r.dateTime, 19) << " |\n";
                }
                std::cout << border << "\n\n";
                system("pause");
            }


            void runGuestSession(const Models::User& guest) 
            {
                while (true)
                {
                    system("cls");
                    std::vector<std::string> items = {
                        "Пройти нове тестування",
                        "Продовжити незавершений тест",
                        "Переглянути мої попередні результати",
                        "Налаштування консолі",
                        "Вийти з акаунту"
                    };

                    int choice = ConsoleMenu::Menu::verticalMenu(
                        "КАБІНЕТ ТЕСТОВАНОГО: " + guest.fullname,
                        items,
                        ConsoleMenu::Center,
                        8
                    );

                    if (choice == -1 || choice == 4)
                        break;

                    if (choice == 0) {
                        std::vector<Models::CategoryInfo> categories = Engine::Test::getCategories();
                        if (categories.empty()) 
                        {
                            std::cout << "Категорії тестів поки що відсутні.\n";
                            system("pause");
                            continue;
                        }

                        std::vector<std::string> catNames;
                        for (const Models::CategoryInfo& c : categories) catNames.push_back(c.name);
                        catNames.push_back("<- Назад");

                        int catIdx = ConsoleMenu::Menu::verticalMenu("Оберіть розділ знань:", catNames, ConsoleMenu::Center, 8);
                        if (catIdx == -1 || catIdx == static_cast<int>(categories.size())) 
                            continue;

                        const Models::CategoryInfo& selectedCat = categories[catIdx];
                        std::vector<std::string> testNames;
                        for (const Models::TestHeader& th : selectedCat.tests) 
                        {
                            testNames.push_back(th.testName + " (питань: " + std::to_string(th.questionCount) + ")");
                        }
                        testNames.push_back("<- Назад");

                        int testIdx = ConsoleMenu::Menu::verticalMenu("Розділ: " + selectedCat.name + " | Оберіть тест:", testNames, ConsoleMenu::Center, 8);
                        if (testIdx == -1 || testIdx == static_cast<int>(selectedCat.tests.size())) 
                            continue;

                        runInteractiveTest(guest, selectedCat.tests[testIdx].testID);
                    }
                    else if (choice == 1) 
                    {
                        std::vector<Models::TestProgress> pausedList = Engine::Test::getProgress(guest.loginHash);
                        if (pausedList.empty()) 
                        {
                            std::cout << "У вас немає призупинених тестів.\n";
                            system("pause");
                            continue;
                        }

                        std::vector<std::string> pausedTitles;
                        for (const Models::TestProgress& p : pausedList) 
                        {
                            pausedTitles.push_back(p.categoryName + " -> " + p.testName +
                                " (питання №" + std::to_string(p.currentQuestionIndex + 1) + ")");
                        }
                        pausedTitles.push_back("<- Назад");

                        int pIdx = ConsoleMenu::Menu::verticalMenu("Незавершені тести:", pausedTitles, ConsoleMenu::Center, 8);
                        if (pIdx == -1 || pIdx == static_cast<int>(pausedList.size())) 
                            continue;

                        runInteractiveTest(guest, pausedList[pIdx].testID);
                    }
                    else if (choice == 2)
                    {
                        std::vector<Models::TestResult> myHistory = Engine::Test::getHistory(guest.loginHash);
                        printResultsList(myHistory, "МОЇ РЕЗУЛЬТАТИ ТЕСТУВАННЯ");
                    }
                    else if (choice == 3) 
                    {
                        ConsoleMenu::Menu::consoleSettings();
                    }
                }
            }

            
            void adminManageUsers()
            {
                while (true) 
                {
                    system("cls");
                    std::vector<std::string> items = {
                        "Список усіх гостей",
                        "Створити нового користувача",
                        "Модифікувати дані користувача",
                        "Видалити користувача",
                        "<- Назад"
                    };

                    int choice = ConsoleMenu::Menu::verticalMenu("УПРАВЛІННЯ КОРИСТУВАЧАМИ", items, ConsoleMenu::Center, 8);
                    if (choice == -1 || choice == 4) 
                        break;

                    if (choice == 0) {
                        std::vector<Models::User> guests = Auth::Manag::getAll();
                        system("cls");
                        std::cout << "=== ЗАРЕЄСТРОВАНІ КОРИСТУВАЧІ ===\n\n";
                        if (guests.empty()) 
                        {
                            std::cout << "Список порожній.\n";
                        }
                        else {
                            for (std::size_t i = 0; i < guests.size(); ++i) 
                            {
                                std::cout << (i + 1) << ". ПІБ: " << guests[i].fullname
                                    << " | Адреса: " << guests[i].address
                                    << " | Тел: " << guests[i].phone << "\n";
                            }
                        }
                        std::cout << "\n";
                        system("pause");
                    }
                    else if (choice == 1) 
                    {
                        system("cls");
                        std::string login, pass, name, addr, phone, err;
                        std::cout << "=== СТВОРЕННЯ КОРИСТУВАЧА ===\n";
                        std::cout << "Логін: "; std::getline(std::cin, login);
                        std::cout << "Пароль: "; std::getline(std::cin, pass);
                        std::cout << "ПІБ: "; std::getline(std::cin, name);
                        std::cout << "Домашня адреса: "; std::getline(std::cin, addr);
                        std::cout << "Телефон: "; std::getline(std::cin, phone);

                        if (Auth::Manag::registerGuest(login, pass, name, addr, phone, err)) 
                        {
                            std::cout << "\nКористувача успішно створено!\n";
                        }
                        else 
                        {
                            std::cout << "\nПомилка: " << err << "\n";
                        }
                        system("pause");
                    }
                    else if (choice == 2) 
                    {
                        std::vector<Models::User> guests = Auth::Manag::getAll();
                        if (guests.empty()) 
                        {
                            std::cout << "Немає користувачів для редагування.\n";
                            system("pause");
                            continue;
                        }
                        std::vector<std::string> names;
                        for (const Models::User& u : guests) names.push_back(u.fullname + " (" + u.phone + ")");
                        names.push_back("<- Назад");

                        int idx = ConsoleMenu::Menu::verticalMenu("Оберіть користувача для редагування:", names, ConsoleMenu::Center, 8);
                        if (idx == -1 || idx == static_cast<int>(guests.size())) 
                            continue;

                        system("cls");
                        std::string newName, newAddr, newPhone, newPass;
                        std::cout << "Редагування: " << guests[idx].fullname << " (залиште поле порожнім, щоб не змінювати)\n";
                        std::cout << "Нове ПІБ: "; std::getline(std::cin, newName);
                        std::cout << "Нова адреса: "; std::getline(std::cin, newAddr);
                        std::cout << "Новий телефон: "; std::getline(std::cin, newPhone);
                        std::cout << "Новий пароль: "; std::getline(std::cin, newPass);

                        if (Auth::Manag::modifyGuest(guests[idx], newName, newAddr, newPhone, newPass)) 
                        {
                            std::cout << "\nДані оновлено!\n";
                        }
                        else {
                            std::cout << "\nПомилка оновлення.\n";
                        }
                        system("pause");
                    }
                    else if (choice == 3) 
                    {
                        std::vector<Models::User> guests = Auth::Manag::getAll();
                        if (guests.empty()) 
                        {
                            std::cout << "Немає користувачів для видалення.\n";
                            system("pause");
                            continue;
                        }
                        std::vector<std::string> names;
                        for (const Models::User& u : guests) names.push_back(u.fullname + " (" + u.phone + ")");
                        names.push_back("<- Назад");

                        int idx = ConsoleMenu::Menu::verticalMenu("Оберіть користувача для ВИДАЛЕННЯ:", names, ConsoleMenu::Center, 8);
                        if (idx == -1 || idx == static_cast<int>(guests.size())) 
                            continue;

                        if (Auth::Manag::removeGuest(guests[idx].loginHash)) 
                        {
                            std::cout << "Користувача видалено!\n";
                        }
                        else 
                        {
                            std::cout << "Помилка видалення.\n";
                        }
                        system("pause");
                    }
                }
            }

            
            void adminManageTests()
            {
                while (true) 
                {
                    system("cls");
                    std::vector<std::string> items = {
                        "Створити нову категорію / новий тест з питаннями",
                        "Додати питання до існуючого тесту",
                        "Видалити тест",
                        "Експортувати категорію з тестами у файл",
                        "Імпортувати категорію з тестами з файлу",
                        "<- Назад"
                    };

                    int choice = ConsoleMenu::Menu::verticalMenu("УПРАВЛІННЯ ТЕСТУВАННЯМ", items, ConsoleMenu::Center, 8);
                    if (choice == -1 || choice == 5) 
                        break;

                    if (choice == 0) 
                    {
                        system("cls");
                        std::string catName, testName;
                        std::cout << "=== СТВОРЕННЯ НОВОГО ТЕСТУ ===\n";
                        std::cout << "Назва категорії (розділу, напр. Математика): ";
                        std::getline(std::cin, catName);
                        std::cout << "Назва тесту (напр. Алгебра): ";
                        std::getline(std::cin, testName);

                        int qCount = readIntInRange("Скільки питань додати зараз (1..50)? ", 1, 50);
                        std::vector<Models::Question> questions;

                        for (int i = 0; i < qCount; ++i) 
                        {
                            Models::Question q;
                            std::cout << "\n--- Питання №" << (i + 1) << " ---\n";
                            std::cout << "Текст питання: ";
                            std::getline(std::cin, q.text);

                            int optCount = readIntInRange("Кількість варіантів відповіді (2..6): ", 2, 6);
                            for (int j = 0; j < optCount; ++j) 
                            {
                                std::string opt;
                                std::cout << "  Варіант " << (j + 1) << ": ";
                                std::getline(std::cin, opt);
                                q.options.push_back(opt);
                            }
                            int correctNum = readIntInRange("Номер правильної відповіді (1.." + std::to_string(optCount) + "): ", 1, optCount);
                            q.correctOption = correctNum - 1;
                            questions.push_back(q);
                        }

                        std::string createdId;
                        if (Engine::Test::createTest(catName, testName, questions, createdId))
                        {
                            std::cout << "\nТест успішно створено та збережено!\n";
                        }
                        else 
                        {
                            std::cout << "\nПомилка створення тесту.\n";
                        }
                        system("pause");
                    }
                    else if (choice == 1) 
                    {
                        std::vector<Models::CategoryInfo> cats = Engine::Test::getCategories();
                        if (cats.empty()) 
                        {
                            std::cout << "Спочатку створіть хоча б один тест.\n";
                            system("pause");
                            continue;
                        }
                        std::vector<std::string> catNames;
                        for (const Models::CategoryInfo& c : cats) catNames.push_back(c.name);
                        catNames.push_back("<- Назад");

                        int cIdx = ConsoleMenu::Menu::verticalMenu("Оберіть категорію:", catNames, ConsoleMenu::Center, 8);
                        if (cIdx == -1 || cIdx == static_cast<int>(cats.size())) 
                            continue;

                        std::vector<std::string> tNames;
                        for (const Models::TestHeader& h : cats[cIdx].tests) tNames.push_back(h.testName);
                        tNames.push_back("<- Назад");

                        int tIdx = ConsoleMenu::Menu::verticalMenu("Оберіть тест:", tNames, ConsoleMenu::Center, 8);
                        if (tIdx == -1 || tIdx == static_cast<int>(cats[cIdx].tests.size())) 
                            continue;

                        system("cls");
                        Models::Question q;
                        std::cout << "=== ДОДАВАННЯ ПИТАННЯ У \"" << cats[cIdx].tests[tIdx].testName << "\" ===\n";
                        std::cout << "Текст питання: ";
                        std::getline(std::cin, q.text);

                        int optCount = readIntInRange("Кількість варіантів відповіді (2..6): ", 2, 6);
                        for (int j = 0; j < optCount; ++j)
                        {
                            std::string opt;
                            std::cout << "  Варіант " << (j + 1) << ": ";
                            std::getline(std::cin, opt);
                            q.options.push_back(opt);
                        }
                        int correctNum = readIntInRange("Номер правильної відповіді (1.." + std::to_string(optCount) + "): ", 1, optCount);
                        q.correctOption = correctNum - 1;

                        if (Engine::Test::addQuestion(cats[cIdx].tests[tIdx].testID, q)) {
                            std::cout << "\nПитання додано!\n";
                        }
                        else 
                        {
                            std::cout << "\nПомилка додавання питання.\n";
                        }
                        system("pause");
                    }
                    else if (choice == 2) {
                        std::vector<Models::CategoryInfo> cats = Engine::Test::getCategories();
                        if (cats.empty()) 
                            continue;

                        std::vector<std::string> catNames;
                        for (const Models::CategoryInfo& c : cats) catNames.push_back(c.name);
                        catNames.push_back("<- Назад");

                        int cIdx = ConsoleMenu::Menu::verticalMenu("Оберіть категорію:", catNames, ConsoleMenu::Center, 8);
                        if (cIdx == -1 || cIdx == static_cast<int>(cats.size())) 
                            continue;

                        std::vector<std::string> tNames;
                        for (const Models::TestHeader& h : cats[cIdx].tests) tNames.push_back(h.testName);
                        tNames.push_back("<- Назад");

                        int tIdx = ConsoleMenu::Menu::verticalMenu("Оберіть тест для ВИДАЛЕННЯ:", tNames, ConsoleMenu::Center, 8);
                        if (tIdx == -1 || tIdx == static_cast<int>(cats[cIdx].tests.size()))
                            continue;

                        if (Engine::Test::removeTest(cats[cIdx].tests[tIdx].testID)) {
                            std::cout << "Тест видалено!\n";
                        }
                        else 
                        {
                            std::cout << "Помилка видалення.\n";
                        }
                        system("pause");
                    }
                    else if (choice == 3)
                    {
                        std::vector<Models::CategoryInfo> cats = Engine::Test::getCategories();
                        if (cats.empty()) 
                            continue;

                        std::vector<std::string> catNames;
                        for (const Models::CategoryInfo& c : cats) catNames.push_back(c.name);
                        catNames.push_back("<- Назад");

                        int cIdx = ConsoleMenu::Menu::verticalMenu("Оберіть категорію для експорту:", catNames, ConsoleMenu::Center, 8);
                        if (cIdx == -1 || cIdx == static_cast<int>(cats.size())) 
                            continue;

                        system("cls");
                        std::string path;
                        std::cout << "Введіть ім'я файлу для експорту (напр. math_export.txt): ";
                        std::getline(std::cin, path);

                        if (Engine::Test::exportCategory(cats[cIdx].name, path)) 
                        {
                            std::cout << "Категорію успішно експортовано у файл \"" << path << "\"!\n";
                        }
                        else 
                        {
                            std::cout << "Помилка експорту.\n";
                        }
                        system("pause");
                    }
                    else if (choice == 4) 
                    {
                        system("cls");
                        std::string path;
                        std::cout << "Введіть ім'я файлу для імпорту (напр. math_export.txt): ";
                        std::getline(std::cin, path);

                        if (Engine::Test::importCategory(path)) 
                        {
                            std::cout << "Категорію та тести успішно імпортовано!\n";
                        }
                        else 
                        {
                            std::cout << "Не вдалося відкрити файл або формат пошкоджено.\n";
                        }
                        system("pause");
                    }
                }
            }

            
            void adminViewStatistics() 
            {
                while (true) 
                {
                    system("cls");
                    std::vector<std::string> items = {
                        "Загальна статистика (всі результати)",
                        "Статистика за категорією",
                        "Статистика за конкретним тестом",
                        "Статистика за конкретним користувачем",
                        "<- Назад"
                    };

                    int choice = ConsoleMenu::Menu::verticalMenu("ПЕРЕГЛЯД СТАТИСТИКИ", items, ConsoleMenu::Center, 8);
                    if (choice == -1 || choice == 4) 
                        break;

                    std::vector<Models::TestResult> selectedResults;
                    std::string reportTitle;

                    if (choice == 0) 
                    {
                        selectedResults = Engine::Test::getStatsAll();
                        reportTitle = "Загальна статистика системи";
                    }
                    else if (choice == 1) 
                    {
                        std::vector<Models::CategoryInfo> cats = Engine::Test::getCategories();
                        if (cats.empty()) 
                            continue;
                        std::vector<std::string> catNames;
                        for (const Models::CategoryInfo& c : cats) catNames.push_back(c.name);
                        catNames.push_back("<- Назад");

                        int cIdx = ConsoleMenu::Menu::verticalMenu("Оберіть категорію:", catNames, ConsoleMenu::Center, 8);
                        if (cIdx == -1 || cIdx == static_cast<int>(cats.size()))
                            continue;

                        selectedResults = Engine::Test::getStatsCategory(cats[cIdx].name);
                        reportTitle = "Категорія: " + cats[cIdx].name;
                    }
                    else if (choice == 2) 
                    {
                        std::vector<Models::CategoryInfo> cats = Engine::Test::getCategories();
                        if (cats.empty()) 
                            continue;
                        std::vector<std::string> catNames;
                        for (const Models::CategoryInfo& c : cats) catNames.push_back(c.name);
                        catNames.push_back("<- Назад");

                        int cIdx = ConsoleMenu::Menu::verticalMenu("Оберіть категорію:", catNames, ConsoleMenu::Center, 8);
                        if (cIdx == -1 || cIdx == static_cast<int>(cats.size())) 
                            continue;

                        std::vector<std::string> tNames;
                        for (const Models::TestHeader& h : cats[cIdx].tests) tNames.push_back(h.testName);
                        tNames.push_back("<- Назад");

                        int tIdx = ConsoleMenu::Menu::verticalMenu("Оберіть тест:", tNames, ConsoleMenu::Center, 8);
                        if (tIdx == -1 || tIdx == static_cast<int>(cats[cIdx].tests.size())) 
                            continue;

                        selectedResults = Engine::Test::getStatsTest(cats[cIdx].name, cats[cIdx].tests[tIdx].testName);
                        reportTitle = "Тест: " + cats[cIdx].name + " -> " + cats[cIdx].tests[tIdx].testName;
                    }
                    else if (choice == 3)
                    {
                        std::vector<Models::User> guests = Auth::Manag::getAll();
                        if (guests.empty())
                        {
                            std::cout << "Користувачів ще немає.\n";
                            system("pause");
                            continue;
                        }
                        std::vector<std::string> names;
                        for (const Models::User& u : guests) names.push_back(u.fullname);
                        names.push_back("<- Назад");

                        int uIdx = ConsoleMenu::Menu::verticalMenu("Оберіть користувача:", names, ConsoleMenu::Center, 8);
                        if (uIdx == -1 || uIdx == static_cast<int>(guests.size())) 
                            continue;

                        selectedResults = Engine::Test::getStatUser(guests[uIdx].loginHash);
                        reportTitle = "Користувач: " + guests[uIdx].fullname;
                    }

                    printResultsList(selectedResults, reportTitle);

                    std::vector<std::string> saveChoice = { "Так, вивести статистику у файл", "Ні, повернутися в меню" };
                    int exportAns = ConsoleMenu::Menu::verticalMenu("Зберегти цей звіт у текстовий файл?", saveChoice, ConsoleMenu::Center, 10);
                    if (exportAns == 0)
                    {
                        system("cls");
                        std::string fileName;
                        std::cout << "Введіть ім'я файлу звіту (напр. report.txt): ";
                        std::getline(std::cin, fileName);
                        if (Engine::Test::saveFile(selectedResults, fileName, reportTitle)) 
                        {
                            std::cout << "Звіт успішно збережено у \"" << fileName << "\"!\n";
                        }
                        else
                        {
                            std::cout << "Помилка запису у файл.\n";
                        }
                        system("pause");
                    }
                }
            }

            
            void runAdminSession(Models::User& admin) 
            {
                while (true)
                {
                    system("cls");
                    std::vector<std::string> items = {
                        "Управління користувачами",
                        "Управління тестуванням (категорії, тести, імпорт/експорт)",
                        "Перегляд та експорт статистики",
                        "Змінити логін та пароль Адміністратора",
                        "Налаштування консолі",
                        "Вийти з режиму Адміністратора"
                    };

                    int choice = ConsoleMenu::Menu::verticalMenu("ПАНЕЛЬ АДМІНІСТРАТОРА", items, ConsoleMenu::Center, 8);
                    if (choice == -1 || choice == 5)
                        break;

                    if (choice == 0) 
                    {
                        adminManageUsers();
                    }
                    else if (choice == 1)
                    {
                        adminManageTests();
                    }
                    else if (choice == 2) 
                    {
                        adminViewStatistics();
                    }
                    else if (choice == 3) 
                    {
                        system("cls");
                        std::string newLogin, newPass, err;
                        std::cout << "=== ЗМІНА ОБЛІКОВИХ ДАНИХ АДМІНІСТРАТОРА ===\n";
                        std::cout << "Новий логін: "; std::getline(std::cin, newLogin);
                        std::cout << "Новий пароль: "; std::getline(std::cin, newPass);

                        if (Auth::Manag::updateAdmin(admin, newLogin, newPass, err)) 
                        {
                            std::cout << "\nЛогін та пароль Адміністратора успішно оновлено!\n";
                        }
                        else 
                        {
                            std::cout << "\nПомилка: " << err << "\n";
                        }
                        system("pause");
                    }
                    else if (choice == 4)
                    {
                        ConsoleMenu::Menu::consoleSettings();
                    }
                }
            }

        } 

        
        void runApp() 
        {
            SetConsoleCP(CP_UTF8);
            SetConsoleOutputCP(CP_UTF8);

            Storage::FileManager::initStorage();
            seedDefaultTestsIfEmpty();

            if (!Auth::Manag::isAdmins())
            {
                handleFirstRunAdminSetup();
            }

            while (true)
            {
                system("cls");
                std::vector<std::string> mainMenuItems = {
                    "Вхід у систему (Адміністратор / Тестований)",
                    "Реєстрація нового тестованого (Гостя)",
                    "Налаштування консолі",
                    "Вихід з програми"
                };

                int choice = ConsoleMenu::Menu::verticalMenu(
                    "СИСТЕМА ТЕСТУВАННЯ ЗНАНЬ",
                    mainMenuItems,
                    ConsoleMenu::Center,
                    8
                );

                if (choice == -1 || choice == 3) 
                {
                    system("cls");
                    std::cout << "Роботу програми завершено.\n";
                    break;
                }

                if (choice == 0)
                {
                    system("cls");
                    std::string login, password;
                    std::cout << "=== ВХІД У СИСТЕМУ ===\n";
                    std::cout << "Логін: ";
                    std::getline(std::cin, login);
                    std::cout << "Пароль: ";
                    std::getline(std::cin, password);

                    Models::User currentUser;
                    if (Auth::Manag::login(login, password, currentUser))
                    {
                        if (currentUser.role == Models::Userrole::Admin) 
                        {
                            runAdminSession(currentUser);
                        }
                        else 
                        {
                            runGuestSession(currentUser);
                        }
                    }
                    else 
                    {
                        std::cout << "\nНевірний логін або пароль!\n";
                        system("pause");
                    }
                }
                else if (choice == 1) 
                {
                    system("cls");
                    std::string login, password, fullName, address, phone, err;
                    std::cout << "=== РЕЄСТРАЦІЯ ТЕСТОВАНОГО (ГОСТЯ) ===\n";
                    std::cout << "Придумайте унікальний логін: ";
                    std::getline(std::cin, login);
                    std::cout << "Придумайте пароль: ";
                    std::getline(std::cin, password);
                    std::cout << "Введіть ПІБ: ";
                    std::getline(std::cin, fullName);
                    std::cout << "Введіть домашню адресу: ";
                    std::getline(std::cin, address);
                    std::cout << "Введіть номер телефону: ";
                    std::getline(std::cin, phone);

                    if (Auth::Manag::registerGuest(login, password, fullName, address, phone, err)) 
                    {
                        std::cout << "\nРеєстрація успішна! Тепер ви можете увійти за своїм логіном та паролем.\n";
                    }
                    else
                    {
                        std::cout << "\nПомилка реєстрації: " << err << "\n";
                    }
                    system("pause");
                }
                else if (choice == 2) 
                {
                    ConsoleMenu::Menu::consoleSettings();
                }
            }
        }

    }
} 