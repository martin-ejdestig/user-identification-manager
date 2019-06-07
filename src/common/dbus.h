// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef USER_IDENTIFICATION_MANAGER_COMMON_DBUS_H
#define USER_IDENTIFICATION_MANAGER_COMMON_DBUS_H

namespace UserIdentificationManager::Common
{
    class DBus
    {
    public:
        static constexpr char MANAGER_SERVICE_NAME[] = "com.luxoft.UserIdentificationManager";
        static constexpr char MANAGER_OBJECT_PATH[] = "/com/luxoft/UserIdentificationManager";
    };
}

#endif // USER_IDENTIFICATION_MANAGER_COMMON_DBUS_H
