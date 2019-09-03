// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "cli/run.h"

#include <glib-unix.h>
#include <glibmm.h>

#include <csignal>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <vector>

namespace UserIdentificationManager::Cli
{
    namespace
    {
        using ManagerProxy = com::luxoft::UserIdentificationManagerProxy;

        gboolean sigint_callback(void *main_loop)
        {
            static_cast<Glib::MainLoop *>(main_loop)->quit();
            return G_SOURCE_CONTINUE;
        }

        void print_identified_user_line(const std::string &prefix,
                                        const std::string &user_identification_id,
                                        guint16 seat_id)
        {
            std::cout << prefix << user_identification_id << ", 0x" << std::setfill('0')
                      << std::setw(4) << std::hex << seat_id << '\n';
        }

        bool print_identified_users(const Glib::RefPtr<ManagerProxy> &manager_proxy)
        {
            std::vector<std::tuple<Glib::ustring, guint16>> users;

            try {
                users = manager_proxy->GetIdentifiedUsers_sync();
            } catch (const Glib::Error &e) {
                std::cout << "Failed to get identified users: " << e.what() << '\n';
                return false;
            }

            std::cout << "Identified users (user identification id, seat id):\n";

            if (users.empty()) {
                std::cout << "  None\n";
            } else {
                for (const auto &user : users) {
                    const Glib::ustring &user_identification_id = std::get<0>(user);
                    guint16 seat_id = std::get<1>(user);

                    print_identified_user_line("  ", user_identification_id.raw(), seat_id);
                }
            }

            return true;
        }

        bool print_sources(const Glib::RefPtr<ManagerProxy> &manager_proxy)
        {
            std::vector<Glib::ustring> enabled_sources;
            std::vector<Glib::ustring> disabled_sources;

            try {
                std::tie(enabled_sources, disabled_sources) = manager_proxy->GetSources_sync();
            } catch (const Glib::Error &e) {
                std::cout << "Failed to get sources: " << e.what() << '\n';
                return false;
            }

            std::cout << "Enabled sources:\n";

            if (enabled_sources.empty()) {
                std::cout << "  None\n";
            } else {
                for (const Glib::ustring &source : enabled_sources) {
                    std::cout << "  " << source << '\n';
                }
            }

            std::cout << "Disabled sources:\n";

            if (disabled_sources.empty()) {
                std::cout << "  None\n";
            } else {
                for (const Glib::ustring &source : disabled_sources) {
                    std::cout << "  " << source << '\n';
                }
            }

            return true;
        }

        bool monitor_until_ctrl_c(const Glib::RefPtr<ManagerProxy> &manager_proxy)
        {
            Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();

            guint sigint_source_id = g_unix_signal_add(SIGINT, sigint_callback, main_loop.get());
            if (sigint_source_id == 0) {
                std::cout << "Failed to setup Ctrl-C handler.\n";
                return false;
            }

            std::cout << "Waiting for users to be identified, press Ctrl-C to quit.\n";

            manager_proxy->UserIdentified_signal.connect(
                [](const Glib::ustring &user_identification_id, guint16 seat_id) {
                    print_identified_user_line("", user_identification_id.raw(), seat_id);
                });

            main_loop->run();

            g_source_remove(sigint_source_id);

            return true;
        }
    }

    int run(const Glib::RefPtr<ManagerProxy> &manager_proxy, const Arguments &arguments)
    {
        if (arguments.print_identified_users) {
            if (!print_identified_users(manager_proxy)) {
                return EXIT_FAILURE;
            }
        }

        if (arguments.print_sources) {
            if (!print_sources(manager_proxy)) {
                return EXIT_FAILURE;
            }
        }

        if (arguments.monitor) {
            if (!monitor_until_ctrl_c(manager_proxy)) {
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }
}
