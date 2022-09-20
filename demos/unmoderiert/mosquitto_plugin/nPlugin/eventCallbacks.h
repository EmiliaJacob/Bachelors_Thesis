#ifndef EVENTCALLBACKS_H
#define EVENTCALLBACKS_H


int onReload();
int onAclCheck();
int onBasicAuth();
int onExtAuthStart();
int onControl();
int onMessage();
int onPskKey();
int onTick();
int onDisconnect();

#endif