// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef UIM_CLI_RUN_H
#define UIM_CLI_RUN_H

#include <glibmm.h>

#include "cli/arguments.h"
#include "generated/dbus/user_identification_manager_proxy.h"

namespace UserIdentificationManager::Cli
{
    int run(const Glib::RefPtr<com::luxoft::UserIdentificationManagerProxy> &manager_proxy,
            const Arguments &arguments);
}

#endif // UIM_CLI_RUN_H
