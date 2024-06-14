#ifndef GET_COMPETITION_QUESTION_HANDLER_HPP
#define GET_COMPETITION_QUESTION_HANDLER_HPP

//--------------------------------------------------------------------------------

#include "database/connection_manager.hpp"

#include "get_handler.hpp"

//--------------------------------------------------------------------------------

namespace get
{
class CompetitionQuestionHandler : public GetHandler
{
public:
    static crow::json::wvalue process(
        const std::unordered_set<int>& aColumn,
        data::SmartConnection& aConnection) noexcept;

    // static std::vector<str::String> mRoles;

    // private:
    // static str::String mProblemPath;

    // static std::vector<str::String> getAllRoles() noexcept;
};
} // namespace get

//--------------------------------------------------------------------------------

#endif // !GET_COMPETITION_QUESTION_HANDLER_HPP
