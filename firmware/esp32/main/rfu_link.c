#include "rfu_link.h"

#include "driver/gpio.h"
#include "driver/spi_slave.h"
#include "esp_attr.h"
#include "esp_cpu.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include <string.h>

#include "soc/gpio_reg.h"

// SPI-slave implementation of the RFU link.
//
// Why not bit-bang: measured 403 ns per GPIO sample (GPIO lives on the 80 MHz
// APB bus, ~97 CPU cycles per access), against a 250 ns half-bit at the 2 MHz
// the GBA masters at -- 0.6 samples per half-bit, so ~19% of words were lost and
// no amount of tuning helped. The RP2040 manages it because PIO owns the pins
// directly and is deterministic; the ESP32 has no equivalent.
//
// So the hardware shifts the bits and software only acts in the GBA's inter-word
// gap, which is >=40 us -- roughly 100 samples even at 403 ns. Every software
// deadline here is microseconds, not nanoseconds.
//
// Geometry (hardware-proven on the Pico): SC idles HIGH, the GBA sets up on the
// falling edge and samples on the rising edge, 32 bits MSB first. From the slave
// side that is CPOL=1 with data shifted on the falling edge and sampled on the
// rising -- SPI mode 3.
//
// The GBA provides no chip-select, so we synthesise one: CS is a spare pin we
// drive ourselves, pulsed high then low inside the gap to frame each word.

#define SC_MASK (1u << RFU_PIN_SC)
#define SO_MASK (1u << RFU_PIN_SO)
#define SI_MASK (1u << RFU_PIN_SI)
#define CS_MASK (1u << RFU_PIN_CS)

static inline IRAM_ATTR bool sc_high(void) { return (REG_READ(GPIO_IN_REG) & SC_MASK) != 0; }
static inline IRAM_ATTR bool so_high(void) { return (REG_READ(GPIO_IN_REG) & SO_MASK) != 0; }
static inline IRAM_ATTR void cs_set(bool high)
{
    if (high) REG_WRITE(GPIO_OUT_W1TS_REG, CS_MASK); else REG_WRITE(GPIO_OUT_W1TC_REG, CS_MASK);
}
static inline IRAM_ATTR void si_set(bool high)
{
    if (high) REG_WRITE(GPIO_OUT_W1TS_REG, SI_MASK); else REG_WRITE(GPIO_OUT_W1TC_REG, SI_MASK);
}
static inline void si_drive(bool enable)
{
    if (enable) REG_WRITE(GPIO_ENABLE_W1TS_REG, SI_MASK); else REG_WRITE(GPIO_ENABLE_W1TC_REG, SI_MASK);
}
static inline void sc_drive(bool enable)
{
    if (enable) REG_WRITE(GPIO_ENABLE_W1TS_REG, SC_MASK); else REG_WRITE(GPIO_ENABLE_W1TC_REG, SC_MASK);
}

#define SPI_HOST_ID SPI2_HOST

static rfu_transfer_done_t g_done_cb;
static void *g_done_user;
static volatile rfu_role_t g_role = RFU_ROLE_GBA_MASTER;
static volatile bool g_enabled;
static volatile uint32_t g_staged_tx;
static volatile bool g_sd_latched;
static uint32_t g_slave_words, g_master_words, g_partial;
static uint32_t g_partialAtBit0, g_partialLate, g_glitches, g_polls;
static uint8_t g_lastPartialBit;
static uint32_t g_highRun;
static volatile bool g_sawClock;   // did SC actually move while armed?
// First words as received, for comparing against what the protocol expects.
// The GBA's opening checkID word is 0x0000494E on the wire (MSB first).
#define WORD_LOG 6
static uint32_t g_wordLog[WORD_LOG];
static uint8_t g_wordLogCount;
static uint32_t g_emptyFrames;   // framing pulses that carried no clocks
static uint16_t g_lastTransLen;
// Consecutive high samples that prove we are between words rather than between
// bits. Sampling costs ~1.6 us while armed (each pass calls into the SPI
// driver), so this must stay small: at 24 it took ~38 us of a ~40 us gap just to
// decide, leaving no time to arm before the next word began. Four samples is
// ~6 us, still far longer than any within-word high phase (250 ns at 2 MHz,
// 2 us at 256 kHz), and leaves most of the gap to arm in.
#define IDLE_GAP_POLLS 4

// One armed word. The buffers must be word aligned for the DMA path.
static WORD_ALIGNED_ATTR uint8_t g_tx_buf[4];
static WORD_ALIGNED_ATTR uint8_t g_rx_buf[4];
static spi_slave_transaction_t g_trans;
static bool g_armed;

static void IRAM_ATTR sd_isr(void *arg) { (void)arg; g_sd_latched = true; }

void rfu_link_set_done_callback(rfu_transfer_done_t cb, void *user)
{
    g_done_user = user;
    g_done_cb = cb;
}

void rfu_link_enable(void)
{
    if (g_enabled) return;

    // CS is ours to drive, and also read by the peripheral, so it must be both.
    gpio_config_t cs_cfg = {
        .pin_bit_mask = 1ULL << RFU_PIN_CS,
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cs_cfg);
    cs_set(true);   // deasserted until we frame a word
    // NOTE: spi_slave_initialize() reconfigures this pin ("GPIO n is conflict
    // with others and be overwritten"), so the output enable is re-asserted
    // after init below -- otherwise our framing pulses never reach the pin.

    // SD carries the game's soft-reset pulse: a ~1 ms high pulse mid-sequence
    // (AgbRFU_SoftReset drives it low, high, low, then releases). Edge-latched
    // because it is far too short to poll for reliably.
    gpio_config_t sd_cfg = {
        .pin_bit_mask = 1ULL << RFU_PIN_SD,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };
    gpio_config(&sd_cfg);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(RFU_PIN_SD, sd_isr, NULL);
    g_sd_latched = false;

    spi_bus_config_t bus = {
        .mosi_io_num = RFU_PIN_SO,   // the GBA's output is data into us
        .miso_io_num = RFU_PIN_SI,   // our output is data into the GBA
        .sclk_io_num = RFU_PIN_SC,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4,
    };
    spi_slave_interface_config_t slv = {
        .spics_io_num = RFU_PIN_CS,
        .flags = 0,                  // MSB first, matching the GBA's wire order
        .queue_size = 2,
        .mode = 3,                   // CPOL=1: shift on falling, sample on rising
        .post_setup_cb = NULL,
        .post_trans_cb = NULL,
    };
    // The GBA drives SC and SO and they idle high; bias them so a floating line
    // is never mistaken for activity.
    gpio_set_pull_mode(RFU_PIN_SC, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(RFU_PIN_SO, GPIO_PULLUP_ONLY);

    if (spi_slave_initialize(SPI_HOST_ID, &bus, &slv, SPI_DMA_CH_AUTO) != ESP_OK) return;

    // Reclaim CS as something we drive. The peripheral reads the pin level via
    // the GPIO matrix, so driving it ourselves is what frames each word.
    gpio_config(&cs_cfg);
    gpio_set_direction(RFU_PIN_CS, GPIO_MODE_INPUT_OUTPUT);
    cs_set(true);

    g_staged_tx = 0;   // RESET-state response
    g_role = RFU_ROLE_GBA_MASTER;
    g_armed = false;
    g_enabled = true;
}

void rfu_link_disable(void)
{
    if (!g_enabled) return;
    g_enabled = false;
    gpio_isr_handler_remove(RFU_PIN_SD);
    spi_slave_free(SPI_HOST_ID);
}

void rfu_link_push_tx(uint32_t word) { g_staged_tx = word; }
rfu_role_t rfu_link_get_role(void) { return g_role; }
bool rfu_link_gba_line_high(void) { return so_high(); }
bool rfu_link_enabled(void) { return g_enabled; }
uint32_t rfu_link_glitches(void) { return g_glitches; }
uint32_t rfu_link_polls(void) { return g_polls; }

uint8_t rfu_link_word_log(uint32_t *out, uint8_t max)
{
    const uint8_t n = g_wordLogCount < max ? g_wordLogCount : max;
    for (uint8_t i = 0; i < n; ++i) out[i] = g_wordLog[i];
    return n;
}
uint32_t rfu_link_empty_frames(void) { return g_emptyFrames; }

// Does driving CS actually move the pin? Reads the pin back after forcing each
// level. If these do not follow, the SPI driver still owns the pin and no word
// can ever be framed.
bool rfu_link_cs_controllable(bool *reads_low, bool *reads_high)
{
    cs_set(false);
    esp_rom_delay_us(5);
    const bool low = (REG_READ(GPIO_IN_REG) & CS_MASK) != 0;
    cs_set(true);
    esp_rom_delay_us(5);
    const bool high = (REG_READ(GPIO_IN_REG) & CS_MASK) != 0;
    if (reads_low) *reads_low = low;
    if (reads_high) *reads_high = high;
    return (!low) && high;   // must read 0 when driven low and 1 when driven high
}
uint16_t rfu_link_last_trans_bits(void) { return g_lastTransLen; }
bool rfu_link_idle_gap(void) { return g_highRun >= IDLE_GAP_POLLS; }

bool rfu_link_sd_reset_seen(void)
{
    if (!g_sd_latched) return false;
    g_sd_latched = false;
    return true;
}

uint8_t rfu_link_levels(void)
{
    const uint32_t in = REG_READ(GPIO_IN_REG), out = REG_READ(GPIO_OUT_REG);
    return (uint8_t)(((in & SC_MASK) ? 1 : 0) | ((in & SO_MASK) ? 2 : 0) |
                     ((out & SI_MASK) ? 4 : 0) | ((in & (1u << RFU_PIN_SD)) ? 8 : 0) |
                     ((in & CS_MASK) ? 16 : 0));
}

void rfu_link_stats(uint32_t *slave_words, uint32_t *master_words, uint32_t *partial)
{
    if (slave_words) *slave_words = g_slave_words;
    if (master_words) *master_words = g_master_words;
    if (partial) *partial = g_partial;
}

void rfu_link_partial_detail(uint32_t *at_bit0, uint32_t *late, uint8_t *last_bit)
{
    if (at_bit0) *at_bit0 = g_partialAtBit0;
    if (late) *late = g_partialLate;
    if (last_bit) *last_bit = g_lastPartialBit;
}

static void arm_word(void);

// Self-test with no GBA: clock 32 pulses into our own slave and report how many
// bits the transaction actually closed with. SC and SI are briefly driven as
// outputs (the GBA drives them normally, so this is only valid while it is idle
// or unplugged) and SO is left to its pull-up, so the received word should be
// all ones. This is how the framing gets validated without occupying the GBA.
bool rfu_link_loopback_test(uint16_t *bits_seen, uint32_t *word_seen)
{
    if (!g_enabled) return false;

    g_staged_tx = 0xA5A5F00Fu;
    arm_word();                       // CS asserted, transaction queued
    if (!g_armed) return false;

    // Drive SC ourselves: idle high, then 32 low/high pairs, mirroring the
    // geometry the GBA uses (set up on the falling edge, sample on the rising).
    REG_WRITE(GPIO_OUT_W1TS_REG, SC_MASK);
    sc_drive(true);
    esp_rom_delay_us(5);
    for (int i = 0; i < 32; ++i)
    {
        REG_WRITE(GPIO_OUT_W1TC_REG, SC_MASK);
        esp_rom_delay_us(2);
        REG_WRITE(GPIO_OUT_W1TS_REG, SC_MASK);
        esp_rom_delay_us(2);
    }
    esp_rom_delay_us(5);
    sc_drive(false);                  // hand SC back

    cs_set(true);                     // close the transaction
    spi_slave_transaction_t *done = NULL;
    const bool got = spi_slave_get_trans_result(SPI_HOST_ID, &done, pdMS_TO_TICKS(50)) == ESP_OK && done;
    g_armed = false;
    if (!got) return false;
    if (bits_seen) *bits_seen = (uint16_t)done->trans_len;
    if (word_seen) *word_seen = ((uint32_t)g_rx_buf[0] << 24) | ((uint32_t)g_rx_buf[1] << 16) |
                                ((uint32_t)g_rx_buf[2] << 8) | g_rx_buf[3];
    return done->trans_len == 32;
}

// Arm one word: frame it with a CS pulse so the peripheral's bit counter starts
// clean, with our staged response already loaded.
static void arm_word(void)
{
    const uint32_t tx = g_staged_tx;
    g_tx_buf[0] = (uint8_t)(tx >> 24);   // MSB first on the wire
    g_tx_buf[1] = (uint8_t)(tx >> 16);
    g_tx_buf[2] = (uint8_t)(tx >> 8);
    g_tx_buf[3] = (uint8_t)tx;

    memset(&g_trans, 0, sizeof(g_trans));
    g_trans.length = 32;
    g_trans.tx_buffer = g_tx_buf;
    g_trans.rx_buffer = g_rx_buf;

    cs_set(true);                        // ensure framing starts clean
    if (spi_slave_queue_trans(SPI_HOST_ID, &g_trans, 0) != ESP_OK) return;
    cs_set(false);                       // armed: the GBA's next clocks are a word
    g_sawClock = false;
    g_armed = true;
}

void IRAM_ATTR rfu_link_poll(void)
{
    if (!g_enabled || g_role != RFU_ROLE_GBA_MASTER) return;
    ++g_polls;

    if (sc_high()) { if (g_highRun < IDLE_GAP_POLLS) ++g_highRun; }
    else { g_highRun = 0; if (g_armed) g_sawClock = true; }

    if (g_armed)
    {
        // CLOSE THE WORD. Measured the hard way: this peripheral treats the
        // transaction length as a buffer bound, NOT a bit counter that ends the
        // transfer -- only CS deassertion ends one. Leaving CS asserted let a
        // single transaction accumulate 7684 bits across many words. So once the
        // clock has rested high long enough to prove we are between words,
        // deassert CS and the transaction closes holding exactly that word.
        // Only close a word that actually began. Closing purely on the gap
        // meant re-arming inside a still-present gap and closing again at once:
        // 2.4 MILLION empty frames, thrashing CS continuously and destroying any
        // word the GBA tried to send.
        if (g_sawClock && rfu_link_idle_gap()) cs_set(true);

        // Checking completion calls into the driver and dominates the sample
        // cost; a word takes >=16 us so a few samples of latency is harmless.
        if ((g_polls & 7u) != 0) return;
        spi_slave_transaction_t *done = NULL;
        if (spi_slave_get_trans_result(SPI_HOST_ID, &done, 0) == ESP_OK && done)
        {
            g_armed = false;
            // trans_len is what actually clocked. A framing pulse with no clocks
            // completes at 0 bits, so only a full word is real data -- counting
            // every completion produced phantom words while the GBA sat idle.
            g_lastTransLen = (uint16_t)done->trans_len;
            // Do NOT gate on trans_len: the loopback self-test showed it
            // reporting 0 while the received buffer plainly held clocked data,
            // so this target does not populate it reliably. Gate on whether the
            // clock actually moved while we were armed instead -- that also
            // rejects a framing pulse the GBA never used.
            if (g_sawClock)
            {
                const uint32_t rx = ((uint32_t)g_rx_buf[0] << 24) | ((uint32_t)g_rx_buf[1] << 16) |
                                    ((uint32_t)g_rx_buf[2] << 8) | g_rx_buf[3];
                ++g_slave_words;
                if (g_wordLogCount < WORD_LOG) g_wordLog[g_wordLogCount++] = rx;
                if (g_done_cb) g_done_cb(rx, g_done_user);
            }
            else ++g_emptyFrames;
            g_sawClock = false;
        }
        return;
    }

    // Not armed: hold the idle-gap mirror (SI = NOT SO), which is what releases
    // librfu's inter-word handshake_wait busy-waits, and re-arm once the gap is
    // established. While unarmed we own SI directly.
    si_set(!so_high());
    if (rfu_link_idle_gap()) arm_word();
}

void rfu_link_set_role(rfu_role_t role)
{
    if (!g_enabled || role == g_role) return;
    if (role == RFU_ROLE_ADAPTER_MASTER)
    {
        // Hand SC over without letting it dip: an armed GBA-as-slave counts any
        // edge as a data bit. Seed the latch high before driving.
        cs_set(true);
        REG_WRITE(GPIO_OUT_W1TS_REG, SC_MASK);
        si_drive(true);
        si_set(false);
        sc_drive(true);
    }
    else
    {
        REG_WRITE(GPIO_OUT_W1TS_REG, SC_MASK);
        sc_drive(false);
        g_armed = false;
        g_highRun = 0;
    }
    g_role = role;
}

void rfu_link_master_drive_si(bool high)
{
    if (g_role == RFU_ROLE_ADAPTER_MASTER) si_set(high);
}

// Adapter-master role stays bit-banged: we own the clock, ~154 kHz, so the
// timing is ours to set and there is nothing to keep up with.
#define CYCLES_PER_US (CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ)
#define MASTER_HALF_CYCLES ((uint32_t)3 * CYCLES_PER_US)

static inline IRAM_ATTR void busy_wait_cycles(uint32_t n)
{
    const uint32_t start = esp_cpu_get_cycle_count();
    while (esp_cpu_get_cycle_count() - start < n) { }
}

bool IRAM_ATTR rfu_link_master_exchange(uint32_t word, uint32_t *rx, uint32_t timeout_us)
{
    (void)timeout_us;
    if (!g_enabled || g_role != RFU_ROLE_ADAPTER_MASTER) return false;

    uint32_t in = 0;
    portDISABLE_INTERRUPTS();
    for (int bit = 0; bit < 32; ++bit)
    {
        REG_WRITE(GPIO_OUT_W1TC_REG, SC_MASK);          // falling edge
        si_set((word & 0x80000000u) != 0);
        word <<= 1;
        busy_wait_cycles(MASTER_HALF_CYCLES / 2);
        in = (in << 1) | (so_high() ? 1u : 0u);         // it shifted SO at the fall
        busy_wait_cycles(MASTER_HALF_CYCLES - MASTER_HALF_CYCLES / 2);
        REG_WRITE(GPIO_OUT_W1TS_REG, SC_MASK);          // rising edge latches ours
        busy_wait_cycles(MASTER_HALF_CYCLES);
    }
    portENABLE_INTERRUPTS();

    ++g_master_words;
    if (rx) *rx = in;
    return true;
}
