#ifndef _WEBSERVERCALLBACK_H_20220329_
#define _WEBSERVERCALLBACK_H_20220329_

#include <iostream>
#include "ProtoXML/ProtoXML.h"

int startWebServer();
int sendtoWebClient(hmiproto::hmiact &act, bool bBroadcast = false);
int disconnectWebClient(const std::list<int> &sIdSet, unsigned int nLimit);

int doRemoteHmiEvent(hmiproto::hmievent &event);

#endif // _WEBSERVERCALLBACK_H_20220329_