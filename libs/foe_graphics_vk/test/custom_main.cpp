// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

int gpuSelection = 0;

int getGpuSelection() { return gpuSelection; }

int main(int argc, char *argv[]) {
    Catch::Session session; // There must be exactly one instance

    // Build a new parser on top of Catch's
    using namespace Catch::clara;
    auto cli = session.cli()                   // Get Catch's composite command line parser
               | Opt(gpuSelection, "gpu")      // bind variable to a new option, with a hint string
                     ["--gpu"]                 // the option names it will respond to
               ("Select GPU to run tests on"); // description string for the help output

    // Now pass the new composite back to Catch so it uses that
    session.cli(cli);

    // Let Catch (using Clara) parse the command line
    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0) // Indicates a command line error
        return returnCode;

    return session.run();
}