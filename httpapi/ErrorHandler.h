#pragma once
#include "../typedefs.h"


// will log exception message.
class MyErrorHandler : public Poco::ErrorHandler
{
    void exception(const std::exception &exc) override
    {
        httpLog(std::string("std::exception ") + exc.what());
    }

    void exception(const Poco::Exception &exc) override
    {
        httpLog("Poco::Exception " + exc.message());
    }
};