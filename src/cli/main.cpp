// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include <giomm.h>
#include <glibmm.h>

#include <clocale>
#include <cstdlib>
#include <iostream>

#include "cli/arguments.h"
#include "cli/run.h"
#include "common/dbus.h"
#include "common/version.h"
#include "generated/dbus/user_identification_manager_proxy.h"

namespace
{
    using Arguments = UserIdentificationManager::Cli::Arguments;
    using ManagerProxy = com::luxoft::UserIdentificationManagerProxy;
}

int main(int argc, char *argv[])
{
    std::setlocale(LC_ALL, "");

    Glib::init();
    Gio::init();

    std::optional<Arguments> arguments = Arguments::parse(argc, argv, std::cout);
    if (!arguments) {
        return EXIT_FAILURE;
    }

    if (arguments->print_version_and_exit) {
        std::cout << Glib::get_prgname() << " " << UserIdentificationManager::Common::VERSION
                  << '\n';
        return EXIT_SUCCESS;
    }

    Glib::RefPtr<ManagerProxy> manager_proxy = ManagerProxy::createForBus_sync(
        Gio::DBus::BUS_TYPE_SYSTEM,
        Gio::DBus::PROXY_FLAGS_NONE,
        UserIdentificationManager::Common::DBus::MANAGER_SERVICE_NAME,
        UserIdentificationManager::Common::DBus::MANAGER_OBJECT_PATH);

    if (manager_proxy->dbusProxy()->get_name_owner().empty()) {
        std::cout << "Service not available, quitting.\n";
        return EXIT_FAILURE;
    }

    return UserIdentificationManager::Cli::run(manager_proxy, *arguments);
}
