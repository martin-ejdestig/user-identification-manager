// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/dbus_service.h"

#include <giomm.h>
#include <glibmm.h>

#include <string>
#include <tuple>
#include <vector>

#include "common/dbus.h"

namespace UserIdentificationManager::Daemon
{
    DBusService::DBusService(const Glib::RefPtr<Glib::MainLoop> &main_loop,
                             IdSource::Group &id_source_group) :
        main_loop_(main_loop),
        manager_(id_source_group)
    {
    }

    DBusService::~DBusService()
    {
        unown_name();
    }

    void DBusService::own_name()
    {
        if (connection_id_ != 0) {
            return;
        }

        connection_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                             Common::DBus::MANAGER_SERVICE_NAME,
                                             sigc::mem_fun(*this, &DBusService::bus_acquired),
                                             sigc::mem_fun(*this, &DBusService::name_acquired),
                                             sigc::mem_fun(*this, &DBusService::name_lost));
    }

    void DBusService::unown_name()
    {
        if (connection_id_ == 0) {
            return;
        }

        manager_.stop_listening_for_user_identified();

        Gio::DBus::unown_name(connection_id_);
        connection_id_ = 0;
    }

    void DBusService::bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connection,
                                   const Glib::ustring & /*name*/)
    {
        if (manager_.register_object(connection, Common::DBus::MANAGER_OBJECT_PATH) == 0) {
            main_loop_->quit();
        }

        manager_.start_listening_for_user_identified();
    }

    void DBusService::name_acquired(const Glib::RefPtr<Gio::DBus::Connection> & /*connection*/,
                                    const Glib::ustring &name)
    {
        g_info("Acquired D-Bus name %s", name.c_str());
    }

    void DBusService::name_lost(const Glib::RefPtr<Gio::DBus::Connection> & /*connection*/,
                                const Glib::ustring &name)
    {
        g_warning("Lost D-Bus name %s, quitting", name.c_str());
        main_loop_->quit();
    }

    DBusService::Manager::Manager(IdSource::Group &id_source_group) :
        id_source_group_(id_source_group)
    {
    }

    void DBusService::Manager::start_listening_for_user_identified()
    {
        user_identified_connection_ = id_source_group_.user_identified_signal().connect(
            sigc::mem_fun(*this, &DBusService::Manager::user_identified));
    }

    void DBusService::Manager::stop_listening_for_user_identified()
    {
        user_identified_connection_.disconnect();
    }

    void DBusService::Manager::user_identified(const IdSource::IdentifiedUser &identified_user)
    {
        UserIdentified_signal.emit(identified_user.user_identification_id, identified_user.seat_id);
    }

    void DBusService::Manager::GetIdentifiedUsers(MethodInvocation &invocation)
    {
        std::vector<std::tuple<Glib::ustring, guint16>> result;

        for (const IdSource::IdentifiedUser &user : id_source_group_.identified_users()) {
            result.emplace_back(user.user_identification_id, user.seat_id);
        }

        invocation.ret(result);
    }

    void DBusService::Manager::GetSources(MethodInvocation &invocation)
    {
        std::vector<Glib::ustring> enabled;
        std::vector<Glib::ustring> disabled;

        for (const std::string &name : id_source_group_.enabled_names()) {
            enabled.emplace_back(name);
        }

        for (const std::string &name : id_source_group_.disabled_names()) {
            disabled.emplace_back(name);
        }

        invocation.ret(enabled, disabled);
    }
}
