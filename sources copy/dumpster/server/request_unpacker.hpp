#ifndef REQUEST_UNPACKER_HPP
#define REQUEST_UNPACKER_HPP

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <crow.h>
#include <string>

#include "domain/holy_trinity.hpp"

namespace serv
{
struct RequestUnpacker
{
    static boost::optional<const str::String&> getToken(
        const crow::request& aReq) noexcept;

    static boost::optional<const str::String&> getPart(
        const crow::multipart::message& aMsg,
        str::String aStr) noexcept;
    static const str::String& getPartUnsafe(
        const crow::multipart::message& aMsg,
        str::String aStr) noexcept;
};

} // namespace serv

#endif // !REQUEST_UNPACKER_HPP
