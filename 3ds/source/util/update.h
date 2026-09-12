/*
 * Auto-update: ask the server whether a newer build exists, and fetch it.
 *
 * See update.c for the two limitations that matter (CIA cannot self-install;
 * the running .3dsx is replaced on disk and applied at next launch).
 */

#ifndef PONG_UPDATE_H
#define PONG_UPDATE_H

#include <stdint.h>
#include <stddef.h>
#include "net.h"
#include "pong_proto.h"

/** Where a .3dsx launched from the Homebrew Launcher normally lives. */
#define PONG_DSX_PATH "sdmc:/3ds/pong3ds.3dsx"

/**
 * Build number, stamped by CI via -DPONG_BUILD_ID.
 *
 * 0 for a local build, which no published build can ever be, so a workstation
 * build is always visibly not-a-release and the updater always offers the real
 * one rather than silently considering itself current.
 */
#ifndef PONG_BUILD_ID
#define PONG_BUILD_ID 0
#endif

/*
 * The build number as a string, embedded in the binary.
 *
 * Means `strings pong3ds.3dsx | grep "Pong3DS build"` identifies any artifact
 * without running it -- useful when several .3dsx files have accumulated on an
 * SD card and it is no longer obvious which is which. It also makes the build
 * stamping verifiable at compile time rather than by trusting the Makefile.
 */
#define PONG__STR2(x) #x
#define PONG__STR(x)  PONG__STR2(x)
#define PONG_VERSION_BANNER "Pong3DS build " PONG__STR(PONG_BUILD_ID)

/** Where update checks look. Toggled in the 3DS settings. */
typedef enum {
    PONG_UPDATE_SRC_SERVER = 0,  /* /api/version on the game server */
    PONG_UPDATE_SRC_GITHUB,      /* the repository's Releases */
} PongUpdateSource;

/* Repository the GitHub source reads. Overridable from the SD config. */
#define PONG_GH_OWNER "johndoe6345789"
#define PONG_GH_REPO  "Pong3DS-Wii"

typedef enum {
    PONG_UPDATE_CURRENT = 0,   /* already newest */
    PONG_UPDATE_AVAILABLE,     /* newer build exists */
    PONG_UPDATE_DONE,          /* downloaded and written */
    PONG_UPDATE_ERROR,
} PongUpdateResult;

typedef struct {
    uint32_t local_build;
    uint32_t remote_build;
    uint32_t remote_protocol;
    char     dsx_path[96];
    char     cia_path[96];
    char     release_url[192];
    /* Absolute URL when the source is GitHub; empty for the server source,
     * where dsx_path is relative to the game server. */
    char     dsx_url[320];
    char     message[160];
} PongUpdateInfo;

/**
 * Asks the configured source what the newest build is.
 *
 * The server source reports what that server is running, which is what you want
 * when the question is "can I play against it". The GitHub source reports the
 * newest published build regardless of what any server is running, which is
 * what you want when the server has simply not been redeployed yet -- a gap
 * that is invisible from the console otherwise.
 */
PongUpdateResult pong_update_check(const PongNetConfig *net, uint32_t local_build,
                                   PongUpdateSource source,
                                   const char *gh_owner, const char *gh_repo,
                                   PongUpdateInfo *out);

PongUpdateResult pong_update_download(const PongNetConfig *net,
                                      const PongUpdateInfo *info,
                                      const char *dest_path,
                                      char *message, size_t message_cap);

/** Human name for the source, for the settings row. */
const char *pong_update_source_name(PongUpdateSource s);

#endif /* PONG_UPDATE_H */
