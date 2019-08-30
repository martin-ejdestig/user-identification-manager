// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef UIM_DAEMON_ID_SOURCES_MASS_STORAGE_DEVICE_ID_SOURCE_H
#define UIM_DAEMON_ID_SOURCES_MASS_STORAGE_DEVICE_ID_SOURCE_H

#include <giomm.h>
#include <glibmm.h>
#include <sigc++/sigc++.h>

#include <optional>
#include <string>
#include <unordered_map>

#include "daemon/id_source.h"

namespace UserIdentificationManager::Daemon
{
    // Identify user by reading file from mass storage device.
    //
    // Used for testing and demo purposes. A file called "pelux-user-id" must exist in the device
    // root when it is mounted. The first two lines of the file must be in the format:
    //
    // ID <id>
    // SEAT <seat id>
    //
    // <id> must be a numeric string and <seat id> must be a 16-bit hexadecimal string. The file is
    // monitored for changes so it is possible to test emitting another user by just modifying the
    // file.
    class MassStorageDeviceIdSource : public IdSource, public sigc::trackable
    {
    public:
        struct Parser;

        MassStorageDeviceIdSource();

        void enable() override;
        void disable() override;

    private:
        void check_existing_mounts();

        void mount_added(const Glib::RefPtr<Gio::Mount> &mount);
        void mount_removed(const Glib::RefPtr<Gio::Mount> &mount);

        void start_monitoring_file(const Glib::RefPtr<Gio::File> &file);
        void stop_monitoring_file(const Glib::RefPtr<Gio::File> &file);

        void file_changed(const Glib::RefPtr<Gio::File> &file,
                          const Glib::RefPtr<Gio::File> &other_file,
                          Gio::FileMonitorEvent event) const;

        void read_file_and_notify(const std::string &path) const;

        Glib::RefPtr<Gio::VolumeMonitor> volume_monitor_ = Gio::VolumeMonitor::get();
        sigc::connection mount_added_connection_;
        sigc::connection mount_removed_connection_;

        std::unordered_map<std::string, Glib::RefPtr<Gio::FileMonitor>> file_monitors_;
    };

    struct MassStorageDeviceIdSource::Parser
    {
        static std::optional<IdentifiedUser> read_file(const std::string &path);
    };
}

#endif // UIM_DAEMON_ID_SOURCES_MASS_STORAGE_DEVICE_ID_SOURCE_H
