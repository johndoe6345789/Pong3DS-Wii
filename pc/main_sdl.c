/*
 * PC front end.
 *
 * Drives the SAME ui/render.c the console uses, through the gfx seam. What this
 * file supplies is the things a platform owns and the interface does not: a
 * window, an input device, and a clock.
 *
 * Networking is not wired up here yet. The interface, the attract mode and the
 * menus are, which is what proves the renderer seam actually holds -- a backend
 * that draws the real interface correctly is a much stronger claim than one
 * that draws a test pattern.
 */

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#include "gfx.h"
#include "render.h"

int main(int argc, char **argv)
{
    /* --size WxH renders another platform's layout. The arrangement rule is
     * shared between backends, so a 960x544 window really is what the Vita
     * lays out -- only the font differs from the console's. */
    for (int i = 1; i + 1 < argc; i++) {
        if (strcmp(argv[i], "--size") == 0) {
            int w = 0, h = 0;
            if (sscanf(argv[i + 1], "%dx%d", &w, &h) == 2) pong_gfx_request_size(w, h);
        }
    }

    if (!pong_gfx_init("Pong - cross-play")) return 1;

    PongHud hud;
    memset(&hud, 0, sizeof hud);
    hud.screen = SCREEN_TITLE;
    hud.menu_sel = 0;
    hud.build_id = 0;
    hud.my_name = "PC PLAYER";   /* --name overrides, for previews */
    hud.opp_name = "";
    hud.server_addr = "pong.wardcrew.com";
    hud.update_src = "GitHub: josheeb0";

    PongView view;
    memset(&view, 0, sizeof view);
    view.valid = false;

    /* --shot <file> renders a few frames and saves one, so the renderer can be
     * checked without a human watching a window. */
    const char *shot = NULL;
    for (int i = 1; i + 1 < argc; i++) {
        if (strcmp(argv[i], "--shot") == 0) shot = argv[i + 1];
    }
    /* Which screen to capture, so the menu and the playfield can both be seen. */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--play") == 0) {
            hud.screen = SCREEN_PLAY;
            hud.status_line = "LAN TCP 192.168.4.29:9787";
            hud.detail_line = "RTT 4  60 poll/s  60 snap/s  buf 50  60 fps";
            hud.opp_name = "BROWSER";
            hud.my_side = 0;
            view.valid = true;
            view.state = PONG_MATCH_STATE_PLAY;
            view.ball_x = (PONG_FIELD_W_Q4 * 3) / 5;
            view.ball_y = PONG_FIELD_H_Q4 / 3;
            view.left_y = PONG_FIELD_H_Q4 / 2;
            view.right_y = PONG_FIELD_H_Q4 / 3;
            view.score_l = 7;
            view.score_r = 4;
        }
    }

    for (int i = 1; i + 1 < argc; i++) {
        if (strcmp(argv[i], "--name") == 0) hud.my_name = argv[i + 1];
    }

    int frames = 0;
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN) {
                switch (e.key.key) {
                case SDLK_ESCAPE: running = false; break;
                case SDLK_DOWN:
                    hud.menu_sel = (hud.menu_sel + 1) % MENU_COUNT;
                    break;
                case SDLK_UP:
                    hud.menu_sel = (hud.menu_sel + MENU_COUNT - 1) % MENU_COUNT;
                    break;
                default: break;
                }
            }
            /* The bottom surface is a touch screen on the console, so a mouse
             * click is the honest equivalent here. */
            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                /* Coordinates need mapping back through the viewport before the
                 * shared hit-testing can use them; not wired yet. */
            }
        }

        hud.frame++;

        pong_gfx_frame_begin();
        pong_render_frame(&view, &hud);
        pong_gfx_frame_end();

        SDL_Delay(16);

        if (shot && ++frames >= 8) {     /* a few frames, so animation settles */
            if (!pong_gfx_screenshot(shot)) {
                fprintf(stderr, "could not save %s\n", shot);
                pong_gfx_exit();
                return 2;
            }
            printf("wrote %s\n", shot);
            running = false;
        }
    }

    pong_gfx_exit();
    return 0;
}
