// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef UIM_CLI_ARGUMENTS_H
#define UIM_CLI_ARGUMENTS_H

#include <optional>
#include <ostream>

namespace UserIdentificationManager::Cli
{
    struct Arguments
    {
        static std::optional<Arguments> parse(int argc, char *argv[], std::ostream &output);

        bool print_version_and_exit = false;

        bool print_identified_users = false;
        bool print_sources = false;

        bool monitor = false;
    };
}

#endif // UIM_CLI_ARGUMENTS_H
