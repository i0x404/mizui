#include <core/application.h>
#include <common/decl.h>
int main(const mizui::i32 argc, char** argv) {
    mizui::core::Application app;

    std::vector<char*> args;
    for (mizui::i32 arg{}; arg < argc; arg++) {
        args.push_back(argv[arg]);
    }
    app.initialize(args);
    const auto collectedGames{app.getAllApplications()};

    for (const auto& game : collectedGames) {
        const std::filesystem::path& gamePath{game.path};
        if (gamePath.extension() == ".nsp") {
            if (app.loadApplication(game.associatedId))
                break;
        }
    }

    app.halt();
    return {};
}
