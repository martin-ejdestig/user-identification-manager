// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "common/scoped_temp_file.h"

#include <gtest/gtest.h>
#include <unistd.h>

#include <fstream>
#include <iterator>
#include <string>

namespace UserIdentificationManager::Common
{
    TEST(ScopedTempFile, CreatedAndRemoved)
    {
        std::string path;
        {
            ScopedTempFile file("test");
            path = file.path();
            EXPECT_EQ(0, access(path.c_str(), R_OK));
        }
        EXPECT_EQ(-1, access(path.c_str(), R_OK));
    }

    TEST(ScopedTempFile, ContentWrittenCorrectly)
    {
        std::string write_content = "test\n123";
        ScopedTempFile file(write_content);

        std::ifstream stream(file.path());
        std::string read_content((std::istreambuf_iterator<char>(stream)),
                                 std::istreambuf_iterator<char>());

        EXPECT_EQ(write_content, read_content);
    }
}
