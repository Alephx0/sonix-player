#ifndef WIFITRANSFER_H
#define WIFITRANSFER_H

#include <stdbool.h>
#include <stdint.h>


// True when the firmware carries the server and the page. False on the host
// build and on a firmware without them, in which case the page says so instead
// of offering a switch that cannot do anything.
bool wifitransfer_available(void);

// The accent colour the served page should use, as 0xRRGGBB. Set it before
// switching the server on: the page is written out at that moment, with the
// colour baked in. Left unset, the template keeps its own blue.
void wifitransfer_set_accent(uint32_t rgb);

// Starts or stops the server. Returns immediately: the work happens on a
// detached thread, because the first start copies the web tree into the
// writable partition and that is not something to do on the interface thread.
void wifitransfer_set_enabled(bool on);

// What was last asked for.
bool wifitransfer_get_enabled(void);

// Whether the server is actually there right now (thttpd in /proc). This is
// what the page should believe, not the flag above.
bool wifitransfer_running(void);

// The name to type into the browser -- "http://sonix-transfer.local" -- or an
// empty string when the responder that publishes it is not up.
//
// This is the one to show first. It is published over mDNS (src/system/mdns.c,
// which the firmware has no equivalent of) and served by a second thttpd on
// port 80, because a .local name resolves to an address and carries no port.
void wifitransfer_url(char *out, int size);

// The address, "http://192.168.1.42", or an empty string when there is no Wi-Fi
// address to build it from.
//
// Worth showing alongside the name rather than instead of it: Chrome and Firefox
// on Android do not resolve .local at all, and an Android phone is exactly the
// thing someone sends songs from. Both work; the name is just nicer to type.
void wifitransfer_address(char *out, int size);

// Puts the server back if it has gone away. The stock player does this every
// 15 seconds while its transfer page is open; call it on the same sort of
// cadence. Does nothing when the feature is off.
void wifitransfer_watchdog(void);

#endif /* WIFITRANSFER_H */
