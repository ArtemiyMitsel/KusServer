#include "user_handler.hpp"

#include <sstream>

#include "domain/date_and_time.hpp"
#include "domain/mail.hpp"

#include "database/connection_manager.hpp"

#include "core/role.hpp"
#include "core/token_handler.hpp"
#include "file_data/file.hpp"
#include "file_data/parser.hpp"
#include "file_data/path.hpp"
#include "get/get_handler.hpp"

std::mutex post::UserHandler::mRegMut;

crow::json::wvalue
post::UserHandler::process(post::PostRequest<data::User>& aReq) noexcept
{
    auto it = aReq.leftovers.find("role");
    if (it != aReq.leftovers.end())
    {
        std::set<std::string> roles;
        for (auto& i : it->second)
        {
            roles.insert(i.s());
        }

        aReq.data.roleID = core::Role::getInstance().getRoleID(roles);
    }
    aReq.leftovers.erase("role");

    int res;
    {
        auto connection = data::ConnectionManager::getUserConnection();
        fiil(aReq.data);
        res = connection.val.write(aReq.data);
    }

    return {res};
}

crow::json::wvalue
post::UserHandler::rawDataHandler(data::RawData& aData) noexcept
{
    for (size_t i = 0; i < aData.value.size(); ++i)
    {
        if (aData.additionalInfo.size() && aData.additionalInfo[i].size())
        {
            std::set<std::string> roles;
            for (auto& j : aData.additionalInfo[i])
            {
                roles.insert(j);
            }

            aData.value[i].emplace_back(
                data::wrap(core::Role::getInstance().getRoleID(roles)));
        }
    }

    return rawDataInsert<data::User>(aData.value);
}

crow::response
post::UserHandler::autorisation(const crow::request& aReq) noexcept
{
    auto x    = crow::json::load(aReq.body);
    auto resp = crow::response(400);
    if (x)
    {
        // data::DatabaseQuery dbq(data::DatabaseQuery::UserType::USER);
        data::User user = parseRequest<data::User>(x).data;

        std::string cond = "login = " + data::wrap(user.login) + " AND " +
                           "password = " + data::wrap(user.password);
        {
            auto connection = data::ConnectionManager::getUserConnection();
            user            = connection.val.getData<data::User>(cond);
        }

        if (!user.id)
        {
            resp      = {"Wrong username or password!"};
            resp.code = 409;
        }
        else if (user.status <= 0)
        {
            resp      = {"Need account confirmation."};
            resp.code = 409;
        }
        else
        {
            crow::json::wvalue uJson;
            uJson["user"] = user.getAsJson({"password", "role_id"});

            auto& tokenHandler = core::TokenHandler::getInstance();
            uJson["user"]["token"] =
                tokenHandler.generate(user, aReq.remote_ip_address);

            auto roles = core::Role::getInstance().getRoles(user.roleID);
            crow::json::wvalue::list roleList;
            for (auto& i : roles)
            {
                roleList.emplace_back(std::move(i));
            }
            uJson["user"]["role"] = std::move(roleList);

            resp = std::move(uJson);
        }
    }
    // resp = crow::response(401);
    return resp;
}

crow::response
post::UserHandler::registration(const crow::request& aReq,
                                bool aNoConfirmation) noexcept
{
    auto resp = crow::response(400);
    auto body = crow::json::load(aReq.body);

    if (body)
    {
        bool allGoodFlag = false;

        data::User newUser = parseRequest<data::User>(body).data;

        mRegMut.lock();

        auto connection = data::ConnectionManager::getUserConnection();

        std::string loginCond = "login = " + data::wrap(newUser.login);
        data::User sameLogin  = connection.val.getData<data::User>(loginCond);

        std::string emailCond = "email = " + data::wrap(newUser.email);
        data::User sameEmail  = connection.val.getData<data::User>(emailCond);

        if (sameLogin.id)
        {
            // crow::json::wvalue uJson;
            // uJson["code"] = 406;
            // uJson["resp"] = "Username already in use!";
            // resp          = std::move(uJson);
            // resp.code     = crow::response::code;

            resp = {"Username already in use!"};
            // resp.set_header("text", "Username already in use!");
        }
        else if (sameEmail.id)
        {
            resp = {"Email already in use!"};
        }
        else if (!applyKey(newUser))
        {
            resp = {"Bad key!"};
        }
        else
        {
            fiil(newUser);
            newUser.status = -1;
            connection.val.write(newUser);

            auto link = sendComfLink(newUser);
            if (link.has_value())
            {
                crow::json::wvalue uJson;
                uJson["user"] = newUser.getAsJson();
                uJson["link"] = link.value();
                resp          = std::move(uJson);
                allGoodFlag   = true;

                data::UserRegistration reg;
                reg.userID = newUser.id;
                reg.link   = link.value();
                connection.val.write(reg);
            }
            else
            {
                // TODO: remove drop
                connection.val.drop(newUser);
                resp = {"Bad email address!"};
            }
        }

        mRegMut.unlock();

        if (!allGoodFlag) resp.code = 409;
    }

    return resp;
}

crow::response
post::UserHandler::confirmation(const std::string& aUrl) noexcept
{
    auto resp = crow::response(400);

    auto connection = data::ConnectionManager::getUserConnection();
    auto userReg    = connection.val.getData<data::UserRegistration>(
        "link = " + data::wrap(aUrl));

    if (userReg.id)
    {
        connection.val.drop(userReg);
        auto user = connection.val.getData<data::User>(
            "id = " + data::wrap(userReg.userID));

        if (user.id)
        {
            if (user.status <= 0)
            {
                user.status = 1;
                connection.val.write(user);
                resp = {"Success"};
            }
            else
            {
                resp = {"Status alredy greater than zero!"};
            }
        }
        else
        {
            resp = {"No such user found!"};
        }
    }
    else
    {
        std::string id;
        for (auto c : aUrl)
        {
            if (!std::isdigit(c)) break;
            if (id.size() > 7) break;
            id.push_back(c);
        }
        if (!id.empty())
        {
            auto user = connection.val.getData<data::User>("id = " + id);

            if (user.id == 0)
            {
                resp = {"Bad link, no such user found!"};
            }
            else if (user.status > 0)
            {
                resp = {"User already confirmed."};
            }
            else
            {
                resp = {"Bad link!"};
            }
        }
        else
        {
            resp = {"Bad link, no user ID!"};
        }
    }

    return resp;
}

void
post::UserHandler::fiil(data::User& aUser) noexcept
{
    if (aUser.name.empty()) aUser.name = "NUN";
    if (aUser.surname.empty()) aUser.surname = "NUN";

    if (aUser.schoolID == 0) aUser.schoolID = -1;

    if (aUser.email.empty()) aUser.email = "NUN";
    if (aUser.key.empty()) aUser.key = "NUN";

    if (aUser.status == 0) aUser.status = 10;
}

std::unordered_map<std::string, std::set<std::string>>
post::UserHandler::getKeyMap() noexcept
{
    std::unordered_map<std::string, std::set<std::string>> result;
    auto data = file::File::getLines("config", "key_role.pass");
    for (int i = 0; i < data.size(); i += 2)
    {
        auto roles      = file::Parser::slice(data[i + 1], " ");
        result[data[i]] = std::set<std::string>(roles.begin(), roles.end());
    }
    result["NUN"] = {""};
    return result;
}

bool
post::UserHandler::applyKey(data::User& aUser) noexcept
{
    bool result = true;

    if (!aUser.key.empty())
    {
        static auto roles = getKeyMap();

        auto it = roles.find(aUser.key);
        if (it != roles.end())
        {
            aUser.roleID = core::Role::getInstance().getRoleID(it->second);
        }
        else
        {
            result = false;
        }
    }

    return result;
}

std::optional<std::string>
post::UserHandler::sendComfLink(const data::User& aUser) noexcept
{
    static dom::Mail mail;

    // to erase whitespaces
    static auto curSiteUrl = file::File::getWords("config", "url.pass")[0][0];

    std::string link = dom::toString(aUser.id) + "=" +
                       dom::DateAndTime::getCurentTimeSafe() + "=";
    for (int i = 0; i < 10; ++i) link += 'a' + rand() % 26;

    std::optional<std::string> result;
    if (mail.send(aUser.email,
                  "Ссылка подтверждения для акаунта на сайте kussystem",
                  "https://" + curSiteUrl + "/api/confirm/" + link))
    {
        result = link;
    }

    return result;
}
