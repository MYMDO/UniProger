/**
 * @file version.h
 * @brief UniProger version information
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 UniProger Contributors
 */

#ifndef UNIPROGER_VERSION_H
#define UNIPROGER_VERSION_H

#define UNIPROGER_VERSION_MAJOR  0
#define UNIPROGER_VERSION_MINOR  1
#define UNIPROGER_VERSION_PATCH  0

#define UNIPROGER_VERSION_STRING "0.1.0-mvp"
#define UNIPROGER_PROJECT_NAME   "UniProger"
#define UNIPROGER_PROJECT_DESC   "Universal Professional Programmer-Analyzer"

#define UNIPROGER_VERSION_MAKE(major, minor, patch) \
    (((major) << 16) | ((minor) << 8) | (patch))

#define UNIPROGER_VERSION_CODE \
    UNIPROGER_VERSION_MAKE(UNIPROGER_VERSION_MAJOR, \
                           UNIPROGER_VERSION_MINOR, \
                           UNIPROGER_VERSION_PATCH)

#endif /* UNIPROGER_VERSION_H */
