// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef UIM_DAEMON_ARGUMENTS_H
#define UIM_DAEMON_ARGUMENTS_H

#include <optional>
#include <ostream>
#include <string>

#include "config.h"

namespace UserIdentificationManager::Daemon
{
    struct Arguments
    {
        static std::optional<Arguments> parse(int argc, char *argv[], std::ostream &output);

        bool print_version_and_exit = false;

        std::string config_file = UIM_CONFIG_DAEMON_DEFAULT_CONFIG_FILE;
    };
}

#endif // UIM_DAEMON_ARGUMENTS_H
