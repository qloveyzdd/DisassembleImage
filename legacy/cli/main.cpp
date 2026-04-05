#include <iostream>
#include <string>

#include "../../core/engine/CpuDisassemblyRunner.h"
#include "../../core/io/LegacyTextConfigLoader.h"
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "用法: legacy_cli <config.txt>" << std::endl;
        return 1;
    }

    try
    {
        auto task = disassemble::core::LegacyTextConfigLoader::load(argv[1]);
        disassemble::core::CpuDisassemblyRunner runner;
        auto result = runner.run(task);

        for (const auto &log : result.logs)
        {
            std::cout << log << std::endl;
        }

        if (!result.ok())
        {
            for (const auto &failure : result.failures)
            {
                std::cerr << failure.inputPath << ": " << failure.reason << std::endl;
            }
            return 2;
        }

        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << std::endl;
        return 3;
    }
}
