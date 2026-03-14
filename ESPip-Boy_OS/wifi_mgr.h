#ifndef WIFI_MGR_H
#define WIFI_MGR_H

#include "log.h"
#include "secrets.h"
#include <WiFi.h>

class WifiMgr {
public:
    enum class Result {
        Connected,
        AlreadyConnected,
        Failed,
    };

    static Result connect(int maxRetries = 20, unsigned long retryDelayMs = 500);
    static void disconnect();
    static bool isConnected();
};

#endif // WIFI_MGR_H
