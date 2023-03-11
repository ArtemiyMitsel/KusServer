#ifndef USER_HANDLER_HPP
#define USER_HANDLER_HPP

#include "post_handler.hpp"

namespace post
{
class UserHandler : public PostHandler
{
public:
    static crow::json::wvalue process(const crow::request& aReq,
                                      data::DatabaseQuery& aDBQ);

    static crow::response autorisation(const crow::request& aReq,
                                       const data::DBSettings& aDBS);
};
} // namespace post

//--------------------------------------------------------------------------------

#endif // !USER_HANDLER_HPP
