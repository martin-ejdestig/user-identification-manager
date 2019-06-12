// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef USER_IDENTIFICATION_MANAGER_DAEMON_CONFIGURATION_H
#define USER_IDENTIFICATION_MANAGER_DAEMON_CONFIGURATION_H

#include <optional>
#include <string>
#include <vector>

#include "config.h"

namespace UserIdentificationManager::Daemon
{
    struct Configuration
    {
        static Configuration from_file(const std::string &file_name);

        std::string config_file = UIM_CONFIG_DAEMON_DEFAULT_CONFIG_FILE;

        std::vector<std::string> sources_enable; // Empty means enable all.
    };
}

#endif // USER_IDENTIFICATION_MANAGER_DAEMON_CONFIGURATION_H
