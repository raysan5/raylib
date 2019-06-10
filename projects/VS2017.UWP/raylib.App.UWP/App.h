#pragma once

#include <string>

#include "pch.h"

// Define what header we use for BaseApp.h
#define PCH "pch.h"

// Enable hold hack
#define HOLDHACK

#include "BaseApp.h"

namespace raylibUWP
{
    ref class App sealed : public BaseApp
    {
    public:
        App();

        // IFrameworkView Methods.
        void Update() override;
    };
}
