// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/string.h"

#include <cstdint>
#include <string>
#include <vector>

namespace UserIdentificationManager::Daemon
{
    namespace
    {
        char nibble_to_char(std::uint8_t nibble)
        {
            return nibble < 10 ? char(nibble + '0') : char(nibble - 10 + 'a');
        }
    }

    std::string string_hex_data(const std::vector<std::uint8_t> &uid)
    {
        std::string ret;

        for (std::uint8_t byte : uid) {
            ret += nibble_to_char(byte >> 4);
            ret += nibble_to_char(byte & 0x0f);
        }

        return ret;
    }

    std::vector<std::string> string_split_multi_string(const char *multi_string)
    {
        std::vector<std::string> strings;
        const char *pos = multi_string;

        while (*pos) {
            strings.emplace_back(pos);
            pos += strings.back().size() + 1;
        }

        return strings;
    }
}
