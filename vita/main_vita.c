/*
 * PS Vita front end.
 *
 * Staged startup with a log, because the first hardware run crashed with a PC
 * inside .bss -- a wild jump, which a core dump alone cannot attribute. The 3DS
 * had the same problem early on and the answer was the same: write down what
 * happened, in order, so the failure names itself instead of being deduced.
 *
 * The log lands at ux0:data/pong_vita.log and is flushed after every line: a
 * buffered log stops exactly where it became interesting.
 */

#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include <string.h>

#include "gfx.h"
#include "render.h"

#define LOG_PATH "ux0:data/pong_vita.log"

static void vlog(const char *msg)
{
    FILE *f = fopen(LOG_PATH, "a");
    if (!f) return;
    fprintf(f, "%s\n", msg);
    fclose(f);          /* reopened per line on purpose; see above */
}

int main(void)
{
    /* Truncate, so each run reads as one run. */
    FILE *f = fopen(LOG_PATH, "w");
    if (f) { fprintf(f, "pong-vita starting\n"); fclose(f); }

    vlog("stage: gfx init");
    if (!pong_gfx_init("Pong")) {
        extern char g_vita_gfx_error[96];
        char line[160];
        snprintf(line, sizeof line, "FAILED: pong_gfx_init -- %s",
                 g_vita_gfx_error[0] ? g_vita_gfx_error : "(no detail)");
        vlog(line);
        sceKernelExitProcess(0);
        return 1;
    }
    vlog("stage: gfx init ok");

    vlog("stage: ctrl sampling");
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    vlog("stage: ctrl ok");

    PongHud hud;
    memset(&hud, 0, sizeof hud);
    hud.screen = SCREEN_TITLE;
    hud.my_name = "VITA PLAYER";
    hud.opp_name = "";
    hud.server_addr = "192.168.4.29";
    hud.status_line = "";
    hud.detail_line = "";
    hud.message = "";
    hud.diag = "";
    hud.room_code = "";
    hud.update_src = "";

    PongView view;
    memset(&view, 0, sizeof view);

    SceCtrlData pad, prev;
    memset(&pad, 0, sizeof pad);
    memset(&prev, 0, sizeof prev);

    /*
     * The first frame is logged either side.
     *
     * Everything before this point is setup that either works or reports; a
     * crash between these two lines is the renderer, and a crash after the
     * second is something that only happens on a later frame. That distinction
     * is most of the diagnosis.
     */
    vlog("stage: first frame");
    pong_gfx_frame_begin();
    pong_render_frame(&view, &hud);
    pong_gfx_frame_end();
    vlog("stage: first frame ok");

    unsigned long frames = 0;
    for (;;) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        unsigned pressed = pad.buttons & ~prev.buttons;
        prev = pad;

        if (pressed & SCE_CTRL_START) break;
        if (pressed & SCE_CTRL_DOWN) hud.menu_sel = (hud.menu_sel + 1) % MENU_COUNT;
        if (pressed & SCE_CTRL_UP)   hud.menu_sel = (hud.menu_sel + MENU_COUNT - 1) % MENU_COUNT;

        hud.frame++;

        pong_gfx_frame_begin();
        pong_render_frame(&view, &hud);
        pong_gfx_frame_end();

        /* Sparse, so the log cannot itself become the slow part. */
        if (++frames == 60 || frames == 600) {
            char line[64];
            snprintf(line, sizeof line, "stage: %lu frames drawn", frames);
            vlog(line);
        }
    }

    vlog("stage: exiting normally");
    pong_gfx_exit();
    sceKernelExitProcess(0);
    return 0;
}
