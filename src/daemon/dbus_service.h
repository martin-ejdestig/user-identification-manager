// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef USER_IDENTIFICATION_MANAGER_DAEMON_DBUS_SERVICE_H
#define USER_IDENTIFICATION_MANAGER_DAEMON_DBUS_SERVICE_H

#include <glibmm.h>
#include <sigc++/sigc++.h>

#include "daemon/id_source.h"
#include "generated/dbus/user_identification_manager_common.h"
#include "generated/dbus/user_identification_manager_stub.h"

namespace UserIdentificationManager::Daemon
{
    class DBusService
    {
    public:
        explicit DBusService(const Glib::RefPtr<Glib::MainLoop> &main_loop,
                             IdSource::Group &id_source_group);
        ~DBusService();

        DBusService(const DBusService &other) = delete;
        DBusService(DBusService &&other) = delete;
        DBusService &operator=(const DBusService &other) = delete;
        DBusService &operator=(DBusService &&other) = delete;

        void own_name();
        void unown_name();

    private:
        class Manager : public com::luxoft::UserIdentificationManagerStub
        {
        public:
            explicit Manager(IdSource::Group &id_source_group);

            void start_listening_for_user_identified();
            void stop_listening_for_user_identified();

        private:
            void user_identified(const IdSource::IdentifiedUser &identified_user);

            void GetIdentifiedUsers(MethodInvocation &invocation) override;
            void GetSources(MethodInvocation &invocation) override;

            IdSource::Group &id_source_group_;
            sigc::connection user_identified_connection_;
        };

        void bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connection,
                          const Glib::ustring &name);
        void name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connection,
                           const Glib::ustring &name);
        void name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connection,
                       const Glib::ustring &name);

        Glib::RefPtr<Glib::MainLoop> main_loop_;
        guint connection_id_ = 0;

        Manager manager_;
    };
}

#endif // USER_IDENTIFICATION_MANAGER_DAEMON_DBUS_SERVICE_H
