#ifndef MODULE_HANDLER_HPP
#define MODULE_HANDLER_HPP

//--------------------------------------------------------------------------------

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "file_data/value.hpp"

#include "module_base.hpp"

//--------------------------------------------------------------------------------

namespace mod
{
class ModuleHandler
{
public:
    static void run() noexcept;
    static str::String processCommand(const str::String& aCommand,
                                      const str::String& aArgument) noexcept;

    static void addCommand(const str::String& aCommand,
                           ModuleBase* aPtr) noexcept;
    static bool hasCommand(const str::String& aCommand) noexcept;

private:
    ModuleHandler() noexcept = default;
    static ModuleHandler& getInstance() noexcept;

    std::mutex mCommandMutex;
    std::vector<ModuleBase::Command> mCommands;

    std::mutex mResultMutex;
    std::unordered_map<int, str::String> mResults;

    std::unordered_map<str::String, ModuleBase*> mModules;

    void executeCommand(int aResultNumber,
                        const ModuleBase::Command& aCommand) noexcept;

    void runNonstatic() noexcept;
    str::String processCommandNonstatic(const str::String& aCommand,
                                        const str::String& aArgument) noexcept;
};
} // namespace mod

//--------------------------------------------------------------------------------

#endif // !MODULE_HANDLER_HPP
