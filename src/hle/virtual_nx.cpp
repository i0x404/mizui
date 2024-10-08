#include <ranges>
#include <unistd.h>
#include <sys/stat.h>

#include <exe/nso.h>
#include <exe/nsp.h>
#include <hle/virtual_nx.h>

#include <core/assets_manager.h>
namespace mizui::hle {
    VirtualNx::VirtualNx(const core::AssetsManager& assets) {
        const auto& executables{assets.collection.readable};
        stockEveryExecutable(executables);

        keysDb.initialize(assets.keys);
    }

    void VirtualNx::stockEveryExecutable(const std::vector<vfs::Path>& executables) {
        for (const auto& [idx, path] : std::views::enumerate(executables)) {
            const std::filesystem::path& local{path};
            bool duplicate{};
#if defined(__linux__)
            struct stat64 ioStatus{};
            stat64(local.c_str(), &ioStatus);
            if (S_ISLNK(ioStatus.st_mode)) {
                std::vector<char> followLink(100);
                readlink(local.c_str(), &followLink[0], followLink.size());
                stat64(&followLink[0], &ioStatus);
            }
#endif
            for (const auto& unique : playable) {
                if (unique.handle == ioStatus.st_ino)
                    duplicate = true;
                if (duplicate)
                    break;
            }
            if (!duplicate)
                playable.emplace_back(idx, ioStatus.st_ino, path);
        }
    }
    bool VirtualNx::loadExecutable(const exe::ExecutableFormat format, std::fstream&& handle) {
        auto loadProgram = [&] -> std::unique_ptr<exe::Executable> {
            if (format == exe::ExecutableFormat::Nso)
                return std::make_unique<exe::Nso>(std::move(handle));
            return std::make_unique<exe::Nsp>(keysDb, std::move(handle));
        };

        program = loadProgram();
        if (!program)
            return {};
        if (program->checkExecutableType() !=
            exe::ExecutableFormat::Unrecognized) {
            return {};
        }
        program->loadExecutable();
        return true;
    }
}
