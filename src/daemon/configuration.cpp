// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/configuration.h"

#include <glib.h>
#include <glibmm.h>

#include <utility>

namespace UserIdentificationManager::Daemon
{
    namespace
    {
        std::vector<std::string> get_string_list(const Glib::KeyFile &key_file,
                                                 const std::string &group,
                                                 const std::string &key,
                                                 std::vector<std::string> default_value)
        {
            std::vector<std::string> value;

            try {
                value = key_file.get_string_list(group, key);
            } catch (const Glib::Error &) {
                value = std::move(default_value);
            }

            return value;
        }
    }

    Configuration Configuration::from_file(const std::string &file_name)
    {
        Configuration config;
        config.config_file = file_name;

        Glib::KeyFile key_file;
        key_file.set_list_separator(',');

        try {
            key_file.load_from_file(file_name);
        } catch (const Glib::Error &e) {
            bool file_does_not_exist = e.matches(G_FILE_ERROR, Glib::FileError::NO_SUCH_ENTITY);

            if (!file_does_not_exist) {
                g_warning("Failed to load %s: %s", file_name.c_str(), e.what().c_str());
            }

            return config;
        }

        config.sources_enable = get_string_list(key_file, "sources", "enable", {});

        return config;
    }
}
