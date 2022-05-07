#pragma once

#include <xorstr.hpp>
#include <httplib.h>
#include <string>

using namespace httplib;

static Server svr;

bool StartBackend(int port = 8080)
{
    /*

    if (!svr.listen(_("0.0.0.0"), port))
    {
        std::cout << _("Failed to listen on port: ") + port << '\n';
        return false;
    }

    */

    svr.listen(_("0.0.0.0"), port);
	
    return true;
}