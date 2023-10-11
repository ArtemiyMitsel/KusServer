#include "token_handler.hpp"

#include <cmath>

#include "domain/date_and_time.hpp"

#include "core/role.hpp"
#include "file_data/file.hpp"
#include "file_data/parser.hpp"
#include "file_data/path.hpp"

#include "pass_generator.hpp"
#include "request_unpacker.hpp"
#include "variable_storage.hpp"

//--------------------------------------------------------------------------------

serv::TokenHandler::TokenHandler() noexcept
    : mIsActive(core::VariableStorage::touchFlag("token_isActive")),
      mAuthorizationMemorise(
          core::VariableStorage::touchFlag("token_isMemory")),
      mTokenIterator(1)
{
    auto size = core::VariableStorage::touchInt("token_size", 100);
    size      = std::max(size, 10);
    mTokens.resize(size);

    std::vector<int> lifespan = {24, 0, 0};
    auto lifespanInput =
        core::VariableStorage::touchWord("token_lifespan", "24:00:00");
    auto temp = file::Parser::slice(lifespanInput, ":");
    for (int i = 0; i < std::min(temp.size(), size_t(3)); ++i)
    {
        lifespan[i] = std::stoi(temp[i]);
    }
    mTokenLifespan = {lifespan[0], lifespan[1], lifespan[2]};

    auto urlsRoles =
        file::File::getWords("config", "url.conf", file::FileType::File,
                             [](char c) { return c == ' ' || c == '\0'; });
    for (auto& i : urlsRoles)
    {
        mAutorisation[i[0]] = std::stoi(i[1]);
    }
}

serv::TokenHandler&
serv::TokenHandler::getInstance() noexcept
{
    static TokenHandler instance;
    return instance;
}

//--------------------------------------------------------------------------------

std::string
serv::TokenHandler::generate(const data::User& aUser,
                             const std::string& aIP) noexcept
{
    static TokenHandler& instance = getInstance();
    return instance.generate(aUser, aIP);
}

boost::optional<const serv::UserData const*>
serv::TokenHandler::process(const crow::request& aReq) noexcept
{
    static TokenHandler& instance = getInstance();
    return instance.processNonstatic(aReq);
}

void
serv::TokenHandler::printAutorisation() const noexcept
{
    std::string data;
    for (auto& i : mAutorisation)
    {
        data += i.first + " " + dom::toString(i.second) + "\n";
    }
    file::File::writeData("config", "url.conf", data);
}

std::string
serv::TokenHandler::generateNonstatic(const data::User& aUser,
                                      const std::string& aIP) noexcept
{
    std::string result;

    mTokenGenerationMutex.lock();
    if (++mTokenIterator == mTokens.size())
    {
        mTokenIterator = 1;
    }
    mTokenGenerationMutex.unlock();

    auto& user      = mTokens[mTokenIterator];
    user.inUse      = true;
    user.ip         = aIP;
    user.id         = aUser.id;
    user.time       = boost::posix_time::second_clock::local_time();
    user.falseLogin = 0;
    user.role       = aUser.roleID;
    user.password =
        PassGenerator::generate() + "=" + dom::toString(mTokenIterator);

    return user.password;
}

serv::UserDataPtr
serv::TokenHandler::processNonstatic(const crow::request& aReq) noexcept
{
    bool result = !mIsActive;

    static std::unordered_set<std::string> withoutAuthentication = {
        "/api/login", "/api/registration",
        // "/api/get/if/competition",
        // "/api/get_question",
        // "/api/post/answer",
        // "/api/get/if/competition",
        // "/api/get/if/competition_user[competition_id[]]",
        // "/api/get/if/competition_question[question_id[id,name]]",
    };
    auto url      = urlDedaction(aReq.raw_url);
    auto tokenOpt = RequestUnpacker::getToken(aReq);

    if (withoutAuthentication.count(url))
    {
        result = true;
    }
    else if (tokenOpt.has_value())
    {
        auto& token = tokenOpt.value();
        result      = mAuthorizationMemorise
                          ? apply(token, url)
                          : check(token, url, aReq.remote_ip_address);
    }

    return result;
}

//--------------------------------------------------------------------------------

bool
serv::TokenHandler::executeCommand(const std::string& aCommand) noexcept
{
    bool result = true;
    if (aCommand == "turn_off")
    {
        mIsActive            = false;
        mAuthorizationSetter = false;
    }
    else if (aCommand == "turn_on")
    {
        mIsActive            = true;
        mAuthorizationSetter = false;
    }
    else if (aCommand == "memory")
    {
        mIsActive            = false;
        mAuthorizationSetter = true;
    }
    else if (aCommand == "print")
    {
        printAutorisation();
    }
    else
    {
        result = false;
    }
    return result;
}

int
serv::TokenHandler::getRoleID(const crow::request& aReq) noexcept
{
    int result = 0;

    auto tokenOpt = getTokenFromReq(aReq);
    if (tokenOpt.has_value())
    {
        auto userOpt = getUserByToken(tokenOpt.value());
        if (userOpt.has_value())
        {
            result = userOpt.value().role;
        }
    }

    return result;
}

std::unordered_set<std::string>
serv::TokenHandler::getRoleName(const crow::request& aReq) noexcept
{
    return core::Role::getRoles(getRoleID(aReq));
}

bool
serv::TokenHandler::checkAuthorizationStatus() noexcept
{
    return mIsActive;
}

//--------------------------------------------------------------------------------

serv::UserDataPtr
serv::TokenHandler::check(const std::string& aToken,
                          const std::string& aURL,
                          const std::string& aIP) noexcept
{
    serv::UserDataPtr result;

    if (mIsActive)
    {
        auto userOpt = getUserDataByToken(aToken);
        if (userOpt.has_value())
        {
            auto& user = userOpt.value();

            if (!dom::DateAndTime::curentTimeAssert(user.time, mTokenLifespan))
            {
                user.inUse = false;
            }

            if (user.inUse) // && user.ip == aIP)
            {
                auto it = mAutorisation.find(aURL);
                if (it != mAutorisation.end() && it->second & user.role)
                {
                    result = &user;
                }
            }
            // else
            // {
            //     if (++mTokens[num].falseLogin > 10)
            //     {
            //         mTokens[num].inUse = false;
            //     }
            // }
        }
    }

    return result;
}

serv::UserDataPtr
serv::TokenHandler::apply(const std::string& aToken,
                          const std::string& aURL) noexcept
{
    serv::UserDataPtr result;
    auto userOpt = getUserDataByToken(aToken);
    if (userOpt.has_value())
    {
        result = &userOpt.value();
        mAutorisation[aURL] |= userOpt.value().role;
    }
    else
    {
        result = &mTokens[0];
        mAutorisation[aURL] |= core::Role::getRoleID(aToken + ",admin");
    }
    return result;
}

//--------------------------------------------------------------------------------

boost::optional<serv::UserData&>
serv::TokenHandler::getUserDataByToken(const std::string& aToken) noexcept
{
    boost::optional<UserData&> result;

    int num = 0;
    for (int i = aToken.size() - 1; i >= 0 && std::isdigit(aToken[i]); --i)
    {
        num *= 10;
        num += aToken[i] - '0';
    }

    if (num > 0 && num < mTokens.size())
    {
        result = mTokens[num];
    }

    return result;
}

std::string
serv::TokenHandler::urlDedaction(const std::string& aUrl) noexcept
{
    std::string result = aUrl;
    while (true)
    {
        auto num = result.rfind("/");
        if (num == std::string::npos) break;
        else
        {
            if (std::isdigit(result[num + 1]) ||
                result.find("=", num) != std::string::npos)
            {
                result.resize(num);
            }
            else break;
        }
    }
    return result;
}