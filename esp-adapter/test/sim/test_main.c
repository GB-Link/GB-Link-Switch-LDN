/* Runs the GBA programs against the adapter under a spread of timings and faults, each
   in its own process so every run starts from power-up. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sim_api.h"
#include "wa_adapter.h"

extern int g_failures;
extern int g_rounds;
extern int g_allow_restarts;
void gba_session(void);
void gba_reset_during_delivery(void);
void gba_interrupts_held(void);
void gba_mode_entry(void);
void gba_sd_chatter(void);
void gba_host_rounds(void);

typedef struct {
    const char *name;
    SimParams p;
    void (*gba)(void);
    int rounds;
} Case;

static int run_case(const Case *c, int verbose)
{
    fflush(stdout);
    const pid_t pid = fork();
    if (pid == 0)
    {
        SimParams p = c->p;
        p.verbose = verbose;
        if (c->rounds) g_rounds = c->rounds;
        g_allow_restarts = p.corruptWord || p.glitchAfterWord;
        const int finished = sim_run(&p, c->gba ? c->gba : gba_session, 120000000);
        wa_stats_t s;
        wa_adapter_stats(&s);
        printf("  words %lu (bad bit counts %lu, realigned %lu), released at once %lu, by interrupt %lu, held %lu\n",
               (unsigned long)s.words, (unsigned long)s.bad_bits, (unsigned long)s.realigns,
               (unsigned long)s.released_spin, (unsigned long)s.released_irq, (unsigned long)s.held);
        printf("  events delivered %lu, failed %lu, retried %lu; resets %lu; ID restarts %lu, command restarts %lu\n",
               (unsigned long)s.deliveries, (unsigned long)s.delivery_fails, (unsigned long)s.delivery_retries,
               (unsigned long)s.sd_resets, (unsigned long)s.login_restarts, (unsigned long)s.command_restarts);
        if (!finished)
        {
            printf("  FAIL: the GBA program did not finish\n");
            g_failures++;
        }
        fflush(stdout);
        _exit(g_failures > 100 ? 100 : g_failures);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

int main(int argc, char **argv)
{
    int verbose = 0;
    const char *only = NULL;
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-v")) verbose = 1;
        else only = argv[i];
    }

    /* gbaRegNs gbaIrqEntryNs adaCallNs wordIsrNs gpioIsrNs spinUs preclock verbose corrupt glitch */
    static const Case cases[] = {
        {"typical", {150, 3000, 100, 2000, 2000, 15, 0, 0, 0, 0}, NULL, 0},
        {"pre-clocked slave", {150, 3000, 100, 2000, 2000, 15, 1, 0, 0, 0}, NULL, 0},
        {"slow port (classic ESP32 APB reads)", {150, 3000, 400, 3000, 3000, 15, 0, 0, 0, 0}, NULL, 0},
        {"late interrupts (Wi-Fi on the same core)", {150, 3000, 100, 30000, 30000, 15, 0, 0, 0, 0}, NULL, 0},
        {"no spin, every release by interrupt", {150, 3000, 100, 2000, 2000, 2, 0, 0, 0, 0}, NULL, 0},
        {"slow GBA (handshake loop in ROM)", {600, 6000, 100, 2000, 2000, 15, 0, 0, 0, 0}, NULL, 0},
        {"fast GBA, slow adapter", {60, 1500, 400, 10000, 10000, 15, 1, 0, 0, 0}, NULL, 0},
        {"a corrupted bit (librfu restarts the command)", {150, 3000, 100, 2000, 2000, 15, 0, 0, 160, 0}, NULL, 0},
        {"a spike on SC between words", {150, 3000, 100, 2000, 2000, 15, 0, 0, 0, 170}, NULL, 0},
        {"a spike on SC, no bit count from the slave", {150, 3000, 100, 2000, 2000, 15, 0, 0, 0, 170, 1}, NULL, 0},
        {"reset during an event delivery", {150, 3000, 100, 2000, 2000, 15, 0, 0, 0, 0}, gba_reset_during_delivery, 0},
        {"interrupts held off for 5 ms", {150, 3000, 100, 2000, 2000, 15, 0, 0, 0, 0}, gba_interrupts_held, 0},
        {"mode entry after a session", {150, 3000, 100, 2000, 2000, 15, 0, 0, 0, 0}, gba_mode_entry, 0},
        {"SD chattering before the game starts", {150, 3000, 100, 2000, 2000, 15, 0, 0, 0, 0}, gba_sd_chatter, 0},
        {"long session", {150, 3000, 400, 3000, 3000, 15, 0, 0, 0, 0}, NULL, 400},
        {"leading a group: idle and data rounds", {150, 3000, 100, 2000, 2000, 15, 0, 0, 0, 0}, gba_host_rounds, 0},
    };
    int failed = 0;
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
    {
        if (only && !strstr(cases[i].name, only)) continue;
        printf("%s\n", cases[i].name);
        const int f = run_case(&cases[i], verbose);
        printf("  %s\n", f ? "FAILED" : "passed");
        failed += f != 0;
    }
    return failed != 0;
}
