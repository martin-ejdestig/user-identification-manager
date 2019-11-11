// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef UIM_DAEMON_STRING_H
#define UIM_DAEMON_STRING_H

#include <cstdint>
#include <string>
#include <vector>

namespace UserIdentificationManager::Daemon
{
    std::string string_hex_data(const std::vector<std::uint8_t> &uid);

    std::vector<std::string> string_split_multi_string(const char *multi_string);
}

#endif // UIM_DAEMON_STRING_H
