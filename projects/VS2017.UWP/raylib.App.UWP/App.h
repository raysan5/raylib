#pragma once

#include <string>

#include "pch.h"

#define PCH "pch.h"
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
