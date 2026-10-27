#ifndef SONIX_DOWNLOAD_IMPORT_H
#define SONIX_DOWNLOAD_IMPORT_H
/* Call once per second on the UI thread. Uses the existing scanner worker. */
void download_import_poll(void);
#endif
