#ifndef GET_COMPETITION_HANDLER_HPP
#define GET_COMPETITION_HANDLER_HPP

//--------------------------------------------------------------------------------

#include "database/connection_manager.hpp"

#include "get_handler.hpp"

//--------------------------------------------------------------------------------

namespace get
{
class CompetitionHandler : public GetHandler
{
public:
    static crow::json::wvalue process(int aUserID, int aCompetitionID) noexcept;

    // static std::vector<std::string> mRoles;

    // private:
    // static std::string mProblemPath;

    // static std::vector<std::string> getAllRoles() noexcept;
};
} // namespace get

//--------------------------------------------------------------------------------

#endif // !GET_COMPETITION_HANDLER_HPP
