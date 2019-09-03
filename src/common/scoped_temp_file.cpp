// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "common/scoped_temp_file.h"

#include <glib.h>
#include <unistd.h>

#include <cstdio>
#include <stdexcept>

namespace
{
    std::string create_temp_file(const std::string &content)
    {
        g_autofree char *path = nullptr;
        int fd = g_file_open_tmp(nullptr, &path, nullptr);
        if (fd == -1) {
            throw std::runtime_error("Failed to create temporary file");
        }

        ssize_t bytes_written = write(fd, content.data(), content.size());
        close(fd);

        if (bytes_written != ssize_t(content.size())) {
            remove(path);
            throw std::runtime_error("Failed to write to temporary file");
        }

        return path;
    }
}

namespace UserIdentificationManager::Common
{
    ScopedTempFile::ScopedTempFile(const std::string &content) : path_(create_temp_file(content))
    {
    }

    ScopedTempFile::~ScopedTempFile()
    {
        if (!path_.empty()) {
            remove(path_.c_str());
        }
    }
}
