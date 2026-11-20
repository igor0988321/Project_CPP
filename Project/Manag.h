#pragma once
#include "DataTypes.h"
#include <string>
#include <vector>


namespace TestingSystem
{
	namespace Auth
	{
		class Manag
		{
		public:
			static bool isAdmins();

			static bool InitialAdmin(const std::string& Login, const std::string& Password);

			static bool login(const std::string& Login, const std::string& Password, Models::User& User);


			static bool  registerGuest(
				const std::string& Login,
				const std::string& Password,
				const std::string& fullName,
				const std::string& address,
				const std::string& prhone,
				std::string& Message
			);

			static bool updateAdmin(
				Models::User& adminUser,
				const std::string& newLogin,
				const std::string& newPassword,
				std::string& Massage
			);


			static std::vector < Models::User> getAll();

			static bool modifyGuest(
				Models::User& Guest,
				const std::string newName,
				const std::string& newAddress,
				const std::string& newPhone,
				const std::string& newPassword
			);
			static bool removeGuest(const std::string& LoginHash);

		};
	}
}
