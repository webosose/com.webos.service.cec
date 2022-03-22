// Copyright (c) 2022 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>

#include "CecLunaService.h"
#include "Logger.h"

static gboolean option_version = FALSE;

static GOptionEntry options[] = {
    { "version", 'v', 0, G_OPTION_ARG_NONE, &option_version,
      "Show version information and exit" },
    { NULL },
};

static GMainLoop *mainLoop = nullptr;

void term_handler(int signal)
{
    const char *str = nullptr;
    switch (signal) {
        case SIGTERM:
            str = "SIGTERM";
            break;
        case SIGABRT:
            str = "SIGABRT";
            break;
        case SIGINT:
            str = "SIGINT";
            break;
        default:
            str = "Unknown";
            break;
    }

    AppLogDebug() << "signal received.. signal[" << str << "]";
    g_main_loop_quit(mainLoop);
}

int main(int argc, char **argv)
{
    try
    {
        GOptionContext *context;
        GError *err = NULL;

        context = g_option_context_new(NULL);
        g_option_context_add_main_entries(context, options, NULL);

        if (g_option_context_parse(context, &argc, &argv, &err) == FALSE) {
            if (err != NULL) {
                g_printerr("%s\n", err->message);
                g_error_free(err);
            } else
                g_printerr("An unknown error occurred\n");
            exit(1);
        }

        g_option_context_free(context);

        signal(SIGTERM, term_handler);
        signal(SIGINT, term_handler);
        mainLoop = g_main_loop_new(NULL, FALSE);

        AppLogDebug() << "Starting cec service";

        CecLunaService service;
        service.attachToLoop(mainLoop);

        g_main_loop_run(mainLoop);
        g_main_loop_unref(mainLoop);
    }
    catch (const std::length_error& le)
    {
        AppLogError() << "Cec service failed, with length error:" << le.what();
    }
    catch (const LS::Error& error)
    {
        AppLogError() << "Cec service failed, with error:" << error.what();
    }

    return 0;
}
