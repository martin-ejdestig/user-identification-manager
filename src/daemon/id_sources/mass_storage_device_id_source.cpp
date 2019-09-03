// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/id_sources/mass_storage_device_id_source.h"

#include <giomm.h>
#include <glib.h>
#include <glibmm.h>

#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace UserIdentificationManager::Daemon
{
    namespace
    {
        constexpr char MASS_STORAGE_DEVICE_SOURCE_NAME[] = "MSD";
        constexpr char USER_ID_FILE_NAME[] = "pelux-user-id";

        std::optional<std::vector<std::string>> read_lines_from_file(const std::string &path,
                                                                     unsigned int num_lines)
        {
            std::ifstream stream(path);

            if (!stream.is_open()) {
                return {};
            }

            std::vector<std::string> lines;

            for (unsigned int i = 0; i < num_lines; i++) {
                std::string line;

                if (!std::getline(stream, line)) {
                    return {};
                }

                lines.emplace_back(std::move(line));
            }

            return lines;
        }

        bool string_starts_with(const std::string &str, const std::string &start)
        {
            return str.compare(0, start.size(), start) == 0;
        }

        bool string_is_numeric(const std::string &str)
        {
            for (char c : str) {
                if (c < '0' || c > '9') {
                    return false;
                }
            }
            return true;
        }
    }

    MassStorageDeviceIdSource::MassStorageDeviceIdSource() :
        IdSource(MASS_STORAGE_DEVICE_SOURCE_NAME)
    {
    }

    void MassStorageDeviceIdSource::enable()
    {
        if (enabled()) {
            return;
        }

        set_enabled(true);

        mount_added_connection_ = volume_monitor_->signal_mount_added().connect(
            sigc::mem_fun(*this, &MassStorageDeviceIdSource::mount_added));

        mount_removed_connection_ = volume_monitor_->signal_mount_removed().connect(
            sigc::mem_fun(*this, &MassStorageDeviceIdSource::mount_removed));

        check_existing_mounts();
    }

    void MassStorageDeviceIdSource::disable()
    {
        if (!enabled()) {
            return;
        }

        file_monitors_.clear();

        mount_added_connection_.disconnect();
        mount_removed_connection_.disconnect();

        set_enabled(false);
    }

    void MassStorageDeviceIdSource::check_existing_mounts()
    {
        std::vector<Glib::RefPtr<Gio::Mount>> mounts = volume_monitor_->get_mounts();

        for (auto &mount : mounts) {
            mount_added(mount);
        }
    }

    void MassStorageDeviceIdSource::mount_added(const Glib::RefPtr<Gio::Mount> &mount)
    {
        Glib::RefPtr<Gio::File> file = mount->get_root()->get_child(USER_ID_FILE_NAME);

        if (!file->query_exists()) {
            return;
        }

        start_monitoring_file(file);
        read_file_and_notify(file->get_path());
    }

    void MassStorageDeviceIdSource::mount_removed(const Glib::RefPtr<Gio::Mount> &mount)
    {
        Glib::RefPtr<Gio::File> file = mount->get_root()->get_child(USER_ID_FILE_NAME);

        stop_monitoring_file(file);
    }

    void MassStorageDeviceIdSource::start_monitoring_file(const Glib::RefPtr<Gio::File> &file)
    {
        Glib::RefPtr<Gio::FileMonitor> file_monitor = file->monitor();

        file_monitor->signal_changed().connect(
            sigc::mem_fun(*this, &MassStorageDeviceIdSource::file_changed));

        file_monitors_.emplace(file->get_path(), file_monitor);
    }

    void MassStorageDeviceIdSource::stop_monitoring_file(const Glib::RefPtr<Gio::File> &file)
    {
        file_monitors_.erase(file->get_path());
    }

    void MassStorageDeviceIdSource::file_changed(const Glib::RefPtr<Gio::File> &file,
                                                 const Glib::RefPtr<Gio::File> & /*other_file*/,
                                                 Gio::FileMonitorEvent event) const
    {
        if (event == Gio::FILE_MONITOR_EVENT_CHANGES_DONE_HINT) {
            read_file_and_notify(file->get_path());
        }
    }

    void MassStorageDeviceIdSource::read_file_and_notify(const std::string &path) const
    {
        std::optional<IdentifiedUser> identified_user = Parser::read_file(path);

        if (!identified_user) {
            return;
        }

        user_identified(*identified_user);
    }

    std::optional<IdSource::IdentifiedUser> MassStorageDeviceIdSource::Parser::read_file(
        const std::string &path)
    {
        std::optional<std::vector<std::string>> lines = read_lines_from_file(path, 2);

        if (!lines) {
            g_warning("%s: failed to read 2 first lines", path.c_str());
            return {};
        }

        const std::string id_prefix = "ID ";
        const std::string seat_prefix = "SEAT ";
        const std::string &id_line = (*lines)[0];
        const std::string &seat_line = (*lines)[1];

        if (!string_starts_with(id_line, id_prefix)) {
            g_warning("%s: first line must start with \"ID \"", path.c_str());
            return {};
        }

        if (!string_starts_with(seat_line, seat_prefix)) {
            g_warning("%s: second line must start with \"SEAT \"", path.c_str());
            return {};
        }

        const std::string id_str = id_line.substr(id_prefix.size());
        const std::string seat_str = seat_line.substr(seat_prefix.size());

        if (id_str.empty() || !string_is_numeric(id_str)) {
            g_warning("%s: ID must be followed by a numeric string", path.c_str());
            return {};
        }

        constexpr unsigned int SEAT_ID_BASE = 16;
        const std::string seat_id_hex_prefix = "0x";
        guint64 seat_id = 0;

        if (!string_starts_with(seat_str, seat_id_hex_prefix) ||
            !g_ascii_string_to_unsigned(seat_str.c_str() + seat_id_hex_prefix.size(),
                                        SEAT_ID_BASE,
                                        SEAT_ID_MIN,
                                        SEAT_ID_MAX,
                                        &seat_id,
                                        nullptr)) {
            g_warning("%s: SEAT must be followed by a hexadecimal 16 bit string", path.c_str());
            return {};
        }

        IdSource::IdentifiedUser identified_user;

        identified_user.user_identification_id =
            std::string(MASS_STORAGE_DEVICE_SOURCE_NAME) + "-" + id_str;
        identified_user.seat_id = seat_id;

        return identified_user;
    }
}
