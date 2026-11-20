#include "Manag.h"
#include "FileManager.h"
#include "md5.h"

namespace TestingSystem
{
	namespace Auth
	{

		bool Manag::isAdmins()
		{
			return Storage::FileManager::isAdmin();
		}

		bool Manag::InitialAdmin(const std::string& Login, const std::string& Password)
		{
			if (isAdmins())
			{
				return false;
			}

			if (Login.empty() || Password.empty())
			{
				return false;
			}

			Models::User admin;
			admin.loginHash = md5(Login);
			admin.passwordHash = md5(Password);
			admin.fullname = "Admin";
			admin.address = "-";
			admin.phone = "-";
			admin.role = Models::Userrole::Admin;

			return Storage::FileManager::addUSer(admin);
		}

		bool Manag::login(const std::string& Login, const std::string& Password, Models::User& User)
		{
			if (Login.empty() || Password.empty())
			{
				return false;
			}
			std::string loginHash = md5(Login);
			std::string passwordHash = md5(Password);

			Models::User foundUser;

			if (!Storage::FileManager::findUser(loginHash, foundUser))
			{
				return false;
			}

			if (foundUser.passwordHash != passwordHash)
			{
				return false;
			}
			User = foundUser;
			return true;
		}

		bool Manag::registerGuest(const std::string& Login,
			const std::string& Password,
			const std::string& Name,
			const std::string& address,
			const std::string& phone,
			std::string& Message)
		{
			if (Login.empty() || Password.empty() || Name.empty())
			{
				Message = "Логін пароль та ПІБ не можуть бути порожніми";
				return false;
			}

			std::string loginHash = md5(Login);

			if (Storage::FileManager::isLogin(loginHash))
			{
				Message = "Користувач із таким логіном вже існує. Оберіть інший логін";
				return false;
			}

			Models::User guest;
			guest.loginHash = loginHash;
			guest.passwordHash = md5(Password);
			guest.fullname = Name;
			guest.address = address.empty() ? "-" : address;
			guest.phone = phone.empty() ? "-" : phone;
			guest.role = Models::Userrole::Guest;

			if (!Storage::FileManager::addUSer(guest))
			{
				Message = "Помилка збереження користувача у базу даних";
				return false;
			}

			Message.clear();
			return true;
		}


		bool Manag::updateAdmin(Models::User& adminUser,
			const std::string& newLogin,
			const std::string& newPassword,
			std::string& Message)
		{
			if (adminUser.role != Models::Userrole::Admin)
			{
				Message = "Доступ заборонено : змінювати облікові дані адміна може тільки адмін";
				return false;
			}

			if (newLogin.empty() || newPassword.empty()  )
			{
				Message = "Новий логін та пароль не можуть бути порожніми";
				return false;
			}

			std::string newLoginHash = md5(newLogin);
			std::string newPasswordHash = md5(newPassword);

			if (newLoginHash != adminUser.loginHash && Storage::FileManager::isLogin(newLoginHash))
			{
				Message = "Цей логін вже зайнятий іншим користувачем";
				return false;
			}

			if (newLoginHash != adminUser.loginHash)
			{
				Models::User updateAdmin = adminUser;
				updateAdmin.loginHash = newLoginHash;
				updateAdmin.passwordHash = newPasswordHash;

				if (!Storage::FileManager::addUSer(updateAdmin))
				{
					Message = "Не вдалося оновити логін адміністратора";
					return false;
				}

				Models::User oldAdmin = adminUser;
				oldAdmin.role = Models::Userrole::Guest;
				Storage::FileManager::updateUser(oldAdmin);
				Storage::FileManager::deleteUSer(adminUser.loginHash);

				adminUser = updateAdmin;
			}
			else
			{
				adminUser.passwordHash = newPasswordHash;
				if (!Storage::FileManager::updateUser(adminUser))
				{
					Message = "Не вдалося оновити пароль адміністратора";
					return false;
				}
			}
			Message.clear();
			return true;
		}

		std::vector < Models::User> Manag::getAll()
		{
			return Storage::FileManager::loadAllGuests();
		}

		bool Manag::modifyGuest(
			Models::User& Guest,
			const std::string newName,
			const std::string& newAddress,
			const std::string& newPhone,
			const std::string& newPassword
		)
		{
			if (Guest.role != Models::Userrole::Guest)
			{
				return false;
			}

			if (!newName.empty())
			{
				Guest.fullname = newName;
			}
			if (!newAddress.empty())
			{
				Guest.address = newAddress;
			}
			if (!newPhone.empty())
			{
				Guest.phone = newPhone;
			}
			if (!newPassword.empty())
			{
				Guest.passwordHash = md5(newPassword);
			}
			return Storage::FileManager::updateUser(Guest);
		}

		bool Manag::removeGuest(const std::string& Login)
		{
			return Storage::FileManager::deleteUSer(Login);
		}
	}
}