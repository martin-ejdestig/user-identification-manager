// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef UIM_DAEMON_DAEMON_H
#define UIM_DAEMON_DAEMON_H

#include <glibmm.h>

#include <memory>
#include <string>

#include "daemon/configuration.h"
#include "daemon/dbus_service.h"
#include "daemon/id_source.h"

namespace UserIdentificationManager::Daemon
{
    class Daemon
    {
    public:
        explicit Daemon(Configuration &&configuration);
        ~Daemon();

        Daemon(const Daemon &other) = delete;
        Daemon(Daemon &&other) = delete;
        Daemon &operator=(const Daemon &other) = delete;
        Daemon &operator=(Daemon &&other) = delete;

        int run();
        void quit() const;

        void reload_config();

    private:
        void apply_config(Configuration &&new_config);

        bool register_signal_handlers();
        void unregister_signal_handlers();

        Configuration configuration_;

        Glib::RefPtr<Glib::MainLoop> main_loop_ = Glib::MainLoop::create();

        guint sigint_source_id_ = 0;
        guint sigterm_source_id_ = 0;
        guint sighup_source_id_ = 0;

        IdSource::Group id_source_group_;

        DBusService dbus_service_{main_loop_, id_source_group_};
    };
}

#endif // UIM_DAEMON_DAEMON_H
