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
#include <optional>

#include "common/version.h"
#include "daemon/arguments.h"
#include "daemon/configuration.h"
#include "daemon/daemon.h"

namespace
{
    using Arguments = UserIdentificationManager::Daemon::Arguments;
    using Configuration = UserIdentificationManager::Daemon::Configuration;
    using Daemon = UserIdentificationManager::Daemon::Daemon;
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

    Daemon daemon(Configuration::from_file(arguments->config_file));

    return daemon.run();
}
