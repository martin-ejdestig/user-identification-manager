// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef USER_IDENTIFICATION_MANAGER_COMMON_SCOPED_TEMP_FILE_H
#define USER_IDENTIFICATION_MANAGER_COMMON_SCOPED_TEMP_FILE_H

#include <string>

namespace UserIdentificationManager::Common
{
    class ScopedTempFile
    {
    public:
        explicit ScopedTempFile(const std::string &content);
        ScopedTempFile(const ScopedTempFile &other) = delete;
        ScopedTempFile(ScopedTempFile &&other) = default;

        ~ScopedTempFile();

        ScopedTempFile &operator=(const ScopedTempFile &other) = delete;
        ScopedTempFile &operator=(ScopedTempFile &&other) = default;

        const std::string &path() const
        {
            return path_;
        }

    private:
        std::string path_;
    };
}

#endif // USER_IDENTIFICATION_MANAGER_COMMON_SCOPED_TEMP_FILE_H
