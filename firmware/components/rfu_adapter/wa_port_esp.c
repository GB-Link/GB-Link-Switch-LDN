#include "wa_port.h"

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/gpio_filter.h"
#include "driver/spi_slave.h"
#include "esp_cpu.h"
#include "esp_random.h"
#include "esp_rom_gpio.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/gpio_ll.h"
#include "hal/spi_ll.h"
#include "hal/spi_slave_hal.h"
#include "soc/gpio_pins.h"
#include "soc/gpio_sig_map.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"
#include "soc/soc_caps.h"
#include "soc/spi_periph.h"
#if SOC_PCNT_SUPPORTED
#include "driver/pulse_cnt.h"
#else
#include "driver/dedic_gpio.h"
#include "hal/dedic_gpio_cpu_ll.h"
#endif

/* The GBA link port on the ESP32's pins.

   A word the GBA clocks goes through SPI2 as a slave in CPU-buffer mode (no DMA): 2 MHz
   is far beyond what GPIO polling can follow. The GBA has no chip select, so the
   slave's CS input is fed from the GPIO matrix's constant-0/1 inputs; asserting and
   deasserting it is one register write and needs no pin. A pulse counter on SC counts
   the GBA's rising (sampling) edges and raises the word interrupt at 32. A chip without
   one takes SC's first falling edge as an interrupt and follows the rest of the word on
   the pins through dedicated GPIO, read by one CPU instruction (a GPIO register read
   takes longer than a 2 MHz half bit): the word has ended once SC rests high, and SO read
   in the last bit gives the bit the slave misses.

   SI is switched between plain GPIO and the slave's MISO with one write to the pin's
   output select. Every route used is captured once from the ROM routing helpers and
   then written whole, so the per-word path is a handful of register writes.

   How the slave presents its first bit differs between chips and modes, so the
   self-test clocks it from the matrix constants (no pin involved) and watches SI. */

#define WA_HOST SPI2_HOST
#define FRAME_BITS 40   /* lead bit, 32-bit reply, padding to whole bytes */
#define IN_INV (1u << GPIO_FUNC0_IN_INV_SEL_S)   /* inverts an input route */

static wa_pins_t s_pins;
static wa_isr_t s_isr;
static const spi_signal_conn_t *s_sig;
static spi_dev_t *s_hw;
static spi_slave_hal_context_t s_hal;
#if SOC_PCNT_SUPPORTED
static pcnt_unit_handle_t s_pcnt;
static pcnt_unit_handle_t s_ones;
#else
static uint32_t s_word_edges, s_word_ones;   /* the last word's, until the next arm */
static int s_bit0 = -1;                      /* SO in its last low half; -1 if not seen */
static uint32_t s_idle_ticks, s_limit_ticks;
static uint32_t s_fast_sc, s_fast_so;        /* SC and SO in the dedicated GPIO input word */
#endif
static bool s_counting;
static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;
static TaskHandle_t s_task;
static int s_core;
static wa_cal_t s_cal;
static uint8_t s_tx[8] __attribute__((aligned(4)));
static uint8_t s_rx[8] __attribute__((aligned(4)));

static uint32_t s_si_gpio, s_si_spi;                    /* SI output select */
static uint32_t s_cs_on, s_cs_off;                      /* slave CS input */
static uint32_t s_clk_pad, s_clk_idle, s_clk_active;    /* slave clock input */
static uint32_t s_mosi_pad, s_mosi_one, s_mosi_zero;    /* slave data input */

static inline void in_sel(uint32_t signal, uint32_t value) { GPIO.func_in_sel_cfg[signal].val = value; }

/* The select word the ROM helper writes for this source, then left in place. */
static uint32_t capture_in(uint32_t source, uint32_t signal, bool inv)
{
    esp_rom_gpio_connect_in_signal(source, signal, inv);
    return GPIO.func_in_sel_cfg[signal].val;
}

/* Output enable comes from the GPIO enable register, not the peripheral, so SI is
   driven whichever source feeds it. */
static uint32_t capture_out(int pin, uint32_t signal)
{
    esp_rom_gpio_connect_out_signal(pin, signal, false, false);
    GPIO.func_out_sel_cfg[pin].oen_sel = 1;
    return GPIO.func_out_sel_cfg[pin].val;
}

static inline bool level(int pin) { return gpio_ll_get_level(&GPIO, pin) != 0; }

static void clear_pin_intr(int pin)
{
    if (pin < 32) gpio_ll_clear_intr_status(&GPIO, 1u << pin);
    else gpio_ll_clear_intr_status_high(&GPIO, 1u << (pin - 32));
}

/* ---- the SPI slave ------------------------------------------------------------- */

static void load(uint64_t frame)
{
    for (int i = 0; i < FRAME_BITS / 8; i++) s_tx[i] = (uint8_t)(frame >> (FRAME_BITS - 8 - 8 * i));
    in_sel(s_sig->spics_in, s_cs_off);
    spi_slave_hal_hw_reset(&s_hal);
    spi_slave_hal_push_tx_buffer(&s_hal);
    spi_slave_hal_hw_fifo_reset(&s_hal, true, false);
    spi_slave_hal_set_trans_bitlen(&s_hal);
#if CONFIG_IDF_TARGET_ESP32
    spi_slave_hal_enable_data_line(&s_hal);
#endif
    spi_slave_hal_user_start(&s_hal);
}

static uint64_t frame_for(uint32_t reply, bool lead)
{
    return ((uint64_t)(lead ? 1 : 0) << (FRAME_BITS - 1)) | ((uint64_t)reply << (FRAME_BITS - 33));
}

/* One clock so the slave puts its first bit out before the GBA's first edge, made by
   inverting the clock's route for a moment: the resting level becomes one short pulse.
   Moving the input between the pin and the matrix constants instead steps the S3's
   slave on by two more clocks. */
static void preclock(void)
{
    const uint32_t route = GPIO.func_in_sel_cfg[s_sig->spiclk_in].val;
    in_sel(s_sig->spiclk_in, route ^ IN_INV);
    esp_rom_delay_us(1);
    in_sel(s_sig->spiclk_in, route);
    esp_rom_delay_us(1);
}

static uint64_t finish(uint16_t *bits)
{
    in_sel(s_sig->spics_in, s_cs_off);
    for (int i = 0; i < 64 && !spi_slave_hal_usr_is_done(&s_hal); i++) { }
    if (bits) *bits = (uint16_t)spi_ll_slave_get_rcv_bitlen(s_hw);
    spi_ll_read_buffer(s_hw, s_rx, FRAME_BITS);
    uint64_t frame = 0;
    for (int i = 0; i < FRAME_BITS / 8; i++) frame = (frame << 8) | s_rx[i];
    return frame;
}

void wa_port_arm(uint32_t reply, bool lead)
{
    load(frame_for(reply, lead));
#if SOC_PCNT_SUPPORTED
    if (s_counting)
    {
        pcnt_unit_clear_count(s_pcnt);
        pcnt_unit_clear_count(s_ones);
    }
#else
    s_word_edges = 0;
    s_word_ones = 0;
    s_bit0 = -1;
    clear_pin_intr(s_pins.sc);
#endif
    in_sel(s_sig->spics_in, s_cs_on);
    if (s_cal.preclock) preclock();   // SC rests high between the GBA's words
}

uint32_t wa_port_collect(uint16_t *bits)
{
    uint16_t count = 0;
    const uint64_t frame = finish(&count);
    if (bits) *bits = count;
    const uint32_t word = (uint32_t)(frame >> (FRAME_BITS - 32 - s_cal.rx_offset));
#if !SOC_PCNT_SUPPORTED
    /* The edges from the slave's count against a clean word's; a count that cannot tell a
       short word from a full one passes for 32. */
    const int edges = s_cal.word_bits ? 32 + (int)count - (int)s_cal.word_bits : 32;
    s_word_edges = edges < 1 ? 1 : (uint32_t)edges;
    s_word_ones = (uint32_t)__builtin_popcount(word & ~1u) + (s_bit0 >= 0 ? (uint32_t)s_bit0 : (word & 1u));
#endif
    return word;
}

void wa_port_disarm(void) { in_sel(s_sig->spics_in, s_cs_off); }

#if SOC_PCNT_SUPPORTED
uint32_t wa_port_edges(void)
{
    int count = 0;
    pcnt_unit_get_count(s_pcnt, &count);
    return count < 0 ? 0 : (uint32_t)count;
}

uint32_t wa_port_ones(void)
{
    int count = 0;
    pcnt_unit_get_count(s_ones, &count);
    return count < 0 ? 0 : (uint32_t)count;
}

void wa_port_word_irq(bool on)
{
    if (on == s_counting) return;
    s_counting = on;
    if (on)
    {
        pcnt_unit_clear_count(s_pcnt);
        pcnt_unit_clear_count(s_ones);
        pcnt_unit_start(s_pcnt);
        pcnt_unit_start(s_ones);
    }
    else
    {
        pcnt_unit_stop(s_pcnt);
        pcnt_unit_stop(s_ones);
    }
}
#else
/* Between an arm and the word's end the count is 1 once SC has fallen: the word is under
   way. The other callers run only between words, as the follow holds the core. */
uint32_t wa_port_edges(void)
{
    if (s_word_edges) return s_word_edges;
    return (REG_READ(GPIO_STATUS_REG) & (1u << s_pins.sc)) ? 1 : 0;
}

uint32_t wa_port_ones(void) { return s_word_ones; }

void wa_port_word_irq(bool on)
{
    if (on == s_counting) return;
    s_counting = on;
    s_word_edges = 0;
    clear_pin_intr(s_pins.sc);
    if (on) gpio_ll_intr_enable_on_core(&GPIO, s_core, s_pins.sc);
    else gpio_ll_intr_disable(&GPIO, s_pins.sc);
}
#endif

void wa_port_so_irq(bool on)
{
    clear_pin_intr(s_pins.so);
    if (on) gpio_ll_intr_enable_on_core(&GPIO, s_core, s_pins.so);
    else gpio_ll_intr_disable(&GPIO, s_pins.so);
}

void wa_port_sd_irq(bool on)
{
    clear_pin_intr(s_pins.sd);
    if (on) gpio_ll_intr_enable_on_core(&GPIO, s_core, s_pins.sd);
    else gpio_ll_intr_disable(&GPIO, s_pins.sd);
}

void wa_port_si_drive(bool high)
{
    gpio_ll_set_level(&GPIO, s_pins.si, high ? 1 : 0);
    GPIO.func_out_sel_cfg[s_pins.si].val = s_si_gpio;
}

void wa_port_si_spi(void) { GPIO.func_out_sel_cfg[s_pins.si].val = s_si_spi; }

/* ---- the adapter clocking ---------------------------------------------------------- */

void wa_port_sc_drive(bool on)
{
    if (on)
    {
        /* The latch goes high before the pin drives: a GBA armed as a slave counts any
           falling edge as a bit. */
        gpio_ll_set_level(&GPIO, s_pins.sc, 1);
        gpio_ll_output_enable(&GPIO, s_pins.sc);
    }
    else
        gpio_ll_output_disable(&GPIO, s_pins.sc);
}

/* ~150 kHz, the GB-Link's rate. Per bit: the falling edge and SI change together (the
   GBA shifts its SO on that edge), SO is read mid-low, the rising edge latches SI into
   the GBA. The GBA is the slave here, so interrupts that stretch a bit are harmless. */
bool wa_port_master_word(uint32_t word, uint32_t *rx, volatile const bool *stop)
{
    uint32_t in = 0;
    for (int bit = 31; bit >= 0; bit--)
    {
        if (*stop) return false;
        gpio_ll_set_level(&GPIO, s_pins.sc, 0);
        gpio_ll_set_level(&GPIO, s_pins.si, (word >> bit) & 1);
        esp_rom_delay_us(2);
        in = (in << 1) | (level(s_pins.so) ? 1 : 0);
        esp_rom_delay_us(1);
        gpio_ll_set_level(&GPIO, s_pins.sc, 1);
        esp_rom_delay_us(3);
    }
    *rx = in;
    return true;
}

bool wa_port_sc(void) { return level(s_pins.sc); }
bool wa_port_so(void) { return level(s_pins.so); }
bool wa_port_si(void) { return level(s_pins.si); }
bool wa_port_sd(void) { return level(s_pins.sd); }

/* ---- self-test --------------------------------------------------------------------- */

typedef struct
{
    uint8_t mode;
    bool flip13;
    bool inv;
} setting_t;

static void apply(const setting_t *s)
{
    spi_ll_slave_set_mode(s_hw, s->mode, false);
#if !CONFIG_IDF_TARGET_ESP32
    if (s->flip13) s_hw->slave.clk_mode_13 = !s_hw->slave.clk_mode_13;
#endif
    /* The received-bit count is kept only when asked for in CPU-buffer mode. */
#if CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
    s_hw->slave.wrbuf_bitlen_en = 1;
#elif CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32C5
    s_hw->slave.slv_wrbuf_bitlen_en = 1;
#endif
    s_clk_pad = capture_in(s_pins.sc, s_sig->spiclk_in, s->inv);
    s_clk_idle = capture_in(s->inv ? GPIO_MATRIX_CONST_ZERO_INPUT : GPIO_MATRIX_CONST_ONE_INPUT, s_sig->spiclk_in, false);
    s_clk_active = capture_in(s->inv ? GPIO_MATRIX_CONST_ONE_INPUT : GPIO_MATRIX_CONST_ZERO_INPUT, s_sig->spiclk_in, false);
}

static inline bool frame_bit(uint64_t frame, int i) { return (frame >> (FRAME_BITS - 1 - i)) & 1; }

/* Clocks a word through the slave the way the GBA does (SC idles high, both sides
   shift on the falling edge, sample on the rising one) and checks that SI shows the
   lead bit before the first edge and frame bit k after falling edge k, steady across
   the rising edge. seen gets the lead level and then the level after each falling
   edge. Returns where the received word starts, or -1. */
static int try_word(bool pre, uint64_t frame, uint32_t sent, int clocks, uint16_t *bits, char *seen)
{
    in_sel(s_sig->spiclk_in, s_clk_idle);
    in_sel(s_sig->spid_in, s_mosi_zero);
    load(frame);
    in_sel(s_sig->spics_in, s_cs_on);
    if (pre) preclock();
    esp_rom_delay_us(2);
    bool ok = level(s_pins.si) == frame_bit(frame, 0);
    if (seen) seen[0] = level(s_pins.si) ? '1' : '0';
    for (int k = 1; k <= clocks; k++)
    {
        in_sel(s_sig->spiclk_in, s_clk_active);
        esp_rom_delay_us(1);
        in_sel(s_sig->spid_in, ((sent >> (32 - k)) & 1) ? s_mosi_one : s_mosi_zero);
        esp_rom_delay_us(1);
        const bool fell = level(s_pins.si);
        in_sel(s_sig->spiclk_in, s_clk_idle);
        esp_rom_delay_us(1);
        const bool rose = level(s_pins.si);
        ok = ok && fell == frame_bit(frame, k) && rose == fell;
        if (seen) seen[k] = fell ? '1' : '0';
    }
    if (seen) seen[clocks + 1] = 0;
    const uint64_t got = finish(bits);
    in_sel(s_sig->spid_in, s_mosi_zero);
    if (!ok || clocks != 32) return -1;
    for (int offset = 0; offset <= FRAME_BITS - 32; offset++)
        if ((uint32_t)(got >> (FRAME_BITS - 32 - offset)) == sent) return offset;
    return -1;
}

static int try_setting(bool pre)
{
    /* The lead bit and every reply bit take both levels across the two words. */
    static const uint32_t replies[2] = {0xB38F0F5A, 0x4C70F0A5};
    static const uint32_t sent[2] = {0x5AF0A3C3, 0xA50F5C3C};
    int offset = -1;
    for (int i = 0; i < 2; i++)
    {
        const int o = try_word(pre, frame_for(replies[i], i == 1), sent[i], 32, NULL, NULL);
        if (o < 0 || (offset >= 0 && o != offset)) return -1;
        offset = o;
    }
    return offset;
}

bool wa_port_calibrate(wa_cal_t *cal)
{
    static const uint8_t modes[4] = {3, 0, 1, 2};
#if CONFIG_IDF_TARGET_ESP32
    const int flips = 1;
#else
    const int flips = 2;
#endif
    memset(&s_cal, 0, sizeof(s_cal));
    GPIO.func_out_sel_cfg[s_pins.si].val = s_si_spi;
    bool found = false;
    setting_t chosen = {3, false, false};
    for (int pre = 0; pre < 2 && !found; pre++)
        for (int inv = 0; inv < 2 && !found; inv++)
            for (int m = 0; m < 4 && !found; m++)
                for (int f = 0; f < flips && !found; f++)
                {
                    const setting_t s = {modes[m], f == 1, inv == 1};
                    apply(&s);
                    const int offset = try_setting(pre == 1);
                    if (offset < 0) continue;
                    found = true;
                    chosen = s;
                    s_cal.preclock = pre == 1;
                    s_cal.rx_offset = (uint8_t)offset;
                }

    apply(&chosen);
    s_cal.ok = found;
    s_cal.mode = chosen.mode;
    s_cal.flip13 = chosen.flip13;
    s_cal.sc_inverted = chosen.inv;
    if (found)
    {
        /* The slave's received-bit count after a word the way the GBA clocks it (32
           edges, plus the pre-clock's). It is used to spot a word that straddled two
           only if a shorter word reads differently. */
        uint16_t full = 0, half = 0;
        try_word(s_cal.preclock, frame_for(0x80000000, true), 0, 32, &full, NULL);
        try_word(s_cal.preclock, frame_for(0x80000000, true), 0, 16, &half, NULL);
        s_cal.word_bits = full != half ? full : 0;
        snprintf(s_cal.detail, sizeof(s_cal.detail), "mode %u%s%s%s, word at bit %u, bit count %u", chosen.mode,
                 chosen.flip13 ? " (first-bit timing flipped)" : "", chosen.inv ? ", clock inverted" : "",
                 s_cal.preclock ? ", pre-clocked" : "", s_cal.rx_offset, s_cal.word_bits);
    }
    else
    {
        /* What SI showed for mode 3 and mode 0: the lead bit, then after each falling
           edge. Expected 0 then 10110011100011110000111101011010. */
        char seen3[34], seen0[34];
        static const setting_t m3 = {3, false, false}, m0 = {0, false, false};
        apply(&m3);
        try_word(false, frame_for(0xB38F0F5A, false), 0x5AF0A3C3, 32, NULL, seen3);
        apply(&m0);
        try_word(false, frame_for(0xB38F0F5A, false), 0x5AF0A3C3, 32, NULL, seen0);
        apply(&chosen);
        snprintf(s_cal.detail, sizeof(s_cal.detail), "no setting passed; SI mode 3 %s mode 0 %s", seen3, seen0);
    }

    in_sel(s_sig->spiclk_in, s_clk_pad);
    in_sel(s_sig->spid_in, s_mosi_pad);
    in_sel(s_sig->spics_in, s_cs_off);
    wa_port_si_drive(true);
    if (cal) *cal = s_cal;
    return found;
}

/* ---- interrupts -------------------------------------------------------------------- */

#if SOC_PCNT_SUPPORTED
static bool on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *ctx)
{
    (void)unit;
    (void)edata;
    (void)ctx;
    s_isr.word();
    return false;
}
#else
/* The word has ended once SC has rested high longer than any half bit (~2 us at the GBA's
   slowest clock). SO is read in every low half, so the last read holds bit 0; a word
   that ended before this ran leaves bit 0 to the slave. The later falling edges of the
   word are dropped, or they would start a follow of their own. */
#define WORD_IDLE_US 3
#define WORD_LIMIT_US 300

static void IRAM_ATTR on_sc(void *arg)
{
    (void)arg;
    if (!s_counting) return;
    const uint32_t sc = s_fast_sc, so = s_fast_so, idle = s_idle_ticks, limit = s_limit_ticks;
    const uint32_t start = esp_cpu_get_cycle_count();
    uint32_t high_since = start;
    bool high = false;
    int bit0 = -1;
    for (;;)
    {
        const uint32_t in = dedic_gpio_cpu_ll_read_in();
        const uint32_t now = esp_cpu_get_cycle_count();
        if (!(in & sc))
        {
            bit0 = (in & so) ? 1 : 0;
            high = false;
        }
        else if (!high)
        {
            high = true;
            high_since = now;
        }
        else if (now - high_since >= idle) break;
        if (now - start >= limit) break;
    }
    clear_pin_intr(s_pins.sc);
    s_bit0 = bit0;
    s_isr.word();
}
#endif

static void on_so(void *arg)
{
    (void)arg;
    s_isr.so_rise();
}

static void on_sd(void *arg)
{
    (void)arg;
    s_isr.sd_fall();
}

/* ---- set-up ----------------------------------------------------------------------- */

bool wa_port_init(const wa_pins_t *pins, const wa_isr_t *isr, wa_cal_t *cal)
{
    s_pins = *pins;
    s_isr = *isr;
    s_task = xTaskGetCurrentTaskHandle();
    s_core = xPortGetCoreID();

    /* The driver claims and clocks the peripheral and routes the three lines; no
       transaction is ever queued through it, so its interrupt stays off. The lines go
       through the GPIO matrix even on the peripheral's own pins: the routes below are
       matrix routes. */
    const spi_bus_config_t bus = {
        .mosi_io_num = pins->so,
        .miso_io_num = pins->si,
        .sclk_io_num = pins->sc,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 64,
        .flags = SPICOMMON_BUSFLAG_GPIO_PINS,
    };
    const spi_slave_interface_config_t slave = {
        .spics_io_num = -1,
        .queue_size = 1,
        .mode = 3,
    };
    ESP_ERROR_CHECK(spi_slave_initialize(WA_HOST, &bus, &slave, SPI_DMA_DISABLED));
    s_sig = &spi_periph_signal[WA_HOST];
    s_hw = s_sig->hw;
    memset(&s_hal, 0, sizeof(s_hal));
    s_hal.hw = s_hw;
    s_hal.mode = 3;
    s_hal.tx_buffer = s_tx;
    s_hal.rx_buffer = s_rx;
    s_hal.tx_bitlen = FRAME_BITS;
    s_hal.rx_bitlen = FRAME_BITS;
    spi_slave_hal_setup_device(&s_hal);

    const gpio_config_t sd = {
        .pin_bit_mask = 1ULL << pins->sd,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&sd));

#if SOC_PCNT_SUPPORTED
    const pcnt_unit_config_t unit = {.low_limit = -1, .high_limit = 100, .intr_priority = 3};
    ESP_ERROR_CHECK(pcnt_new_unit(&unit, &s_pcnt));
    /* SC rings on a cable; at 2 MHz a half bit is ~240 ns. */
    const pcnt_glitch_filter_config_t filter = {.max_glitch_ns = 100};
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(s_pcnt, &filter));
    const pcnt_chan_config_t chan = {.edge_gpio_num = pins->sc, .level_gpio_num = -1};
    pcnt_channel_handle_t channel;
    ESP_ERROR_CHECK(pcnt_new_channel(s_pcnt, &chan, &channel));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(channel, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                                 PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(s_pcnt, 32));
    const pcnt_event_callbacks_t callbacks = {.on_reach = on_reach};
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(s_pcnt, &callbacks, NULL));
    ESP_ERROR_CHECK(pcnt_unit_enable(s_pcnt));
    /* The same rising edges, counted only while SO is high: the number of ones in the
       word the GBA sent, read at its own clock edges. */
    const pcnt_unit_config_t ones_unit = {.low_limit = -1, .high_limit = 100};
    ESP_ERROR_CHECK(pcnt_new_unit(&ones_unit, &s_ones));
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(s_ones, &filter));
    const pcnt_chan_config_t ones_chan = {.edge_gpio_num = pins->sc, .level_gpio_num = pins->so};
    pcnt_channel_handle_t ones_channel;
    ESP_ERROR_CHECK(pcnt_new_channel(s_ones, &ones_chan, &ones_channel));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(ones_channel, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                                 PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(ones_channel, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                                  PCNT_CHANNEL_LEVEL_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_unit_enable(s_ones));
#else
    _Static_assert(SOC_DEDICATED_GPIO_SUPPORTED, "the follow reads SC and SO through dedicated GPIO");
    s_idle_ticks = WORD_IDLE_US * esp_rom_get_cpu_ticks_per_us();
    s_limit_ticks = WORD_LIMIT_US * esp_rom_get_cpu_ticks_per_us();
    /* Bundled on this core, where the SC interrupt lands. */
    const int followed[2] = {pins->sc, pins->so};
    const dedic_gpio_bundle_config_t fast = {.gpio_array = followed, .array_size = 2, .flags = {.in_en = 1}};
    dedic_gpio_bundle_handle_t bundle;
    ESP_ERROR_CHECK(dedic_gpio_new_bundle(&fast, &bundle));
    uint32_t offset = 0;
    ESP_ERROR_CHECK(dedic_gpio_get_in_offset(bundle, &offset));
    s_fast_sc = 1u << offset;
    s_fast_so = 1u << (offset + 1);
#endif
    s_counting = false;

    /* The GBA's lines idle high: pull SC, SO and SD (above) up so a floating line never
       reads as activity. */
    gpio_set_pull_mode(pins->sc, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(pins->so, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(pins->si, GPIO_FLOATING);
    gpio_ll_input_enable(&GPIO, pins->si);   /* SI is read back by the self-test */

    s_si_gpio = capture_out(pins->si, SIG_GPIO_OUT_IDX);
    s_si_spi = capture_out(pins->si, s_sig->spiq_out);
    s_cs_on = capture_in(GPIO_MATRIX_CONST_ZERO_INPUT, s_sig->spics_in, false);
    s_cs_off = capture_in(GPIO_MATRIX_CONST_ONE_INPUT, s_sig->spics_in, false);
    s_mosi_one = capture_in(GPIO_MATRIX_CONST_ONE_INPUT, s_sig->spid_in, false);
    s_mosi_zero = capture_in(GPIO_MATRIX_CONST_ZERO_INPUT, s_sig->spid_in, false);
    s_mosi_pad = capture_in(pins->so, s_sig->spid_in, false);
    wa_port_si_drive(true);
    gpio_ll_output_enable(&GPIO, pins->si);

    /* SO is the GBA's output. The driver routes the slave's data output to it as well
       (for half-duplex slaves): make it an input only. */
    GPIO.func_out_sel_cfg[pins->so].val = s_si_gpio;
    gpio_ll_output_disable(&GPIO, pins->so);

    /* SC is ours only while delivering: a GPIO output, enabled then. */
    gpio_ll_set_level(&GPIO, pins->sc, 1);
    GPIO.func_out_sel_cfg[pins->sc].val = s_si_gpio;
    gpio_ll_output_disable(&GPIO, pins->sc);
    gpio_ll_input_enable(&GPIO, pins->sc);

    /* SI switches on SC's falling edges, and along a link cable a sharp SI edge puts
       glitches on SC. The slave clocks on every one of them (the pulse counter filters
       its own copy), so SI and SC drive gently, and the GBA's lines are filtered where
       the chip can: SO and SD carry the same kind of spikes into the data and the reset
       interrupt. */
    gpio_set_drive_capability(pins->si, GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability(pins->sc, GPIO_DRIVE_CAP_0);
#if SOC_GPIO_SUPPORT_PIN_GLITCH_FILTER
    const int filtered[3] = {pins->sc, pins->so, pins->sd};
    for (int i = 0; i < 3; i++)
    {
        const gpio_pin_glitch_filter_config_t filtering = {.clk_src = GLITCH_FILTER_CLK_SRC_DEFAULT,
                                                           .gpio_num = filtered[i]};
        gpio_glitch_filter_handle_t filter;
        if (gpio_new_pin_glitch_filter(&filtering, &filter) == ESP_OK) gpio_glitch_filter_enable(filter);
        else printf("WA no glitch filter on GPIO%d\n", filtered[i]);
    }
#endif

    const esp_err_t service = gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    if (service != ESP_OK && service != ESP_ERR_INVALID_STATE) ESP_ERROR_CHECK(service);
    ESP_ERROR_CHECK(gpio_set_intr_type(pins->so, GPIO_INTR_POSEDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(pins->so, on_so, NULL));
    wa_port_so_irq(false);
    ESP_ERROR_CHECK(gpio_isr_handler_add(pins->sd, on_sd, NULL));
    wa_port_sd_irq(false);
#if !SOC_PCNT_SUPPORTED
    ESP_ERROR_CHECK(gpio_set_intr_type(pins->sc, GPIO_INTR_NEGEDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(pins->sc, on_sc, NULL));
    gpio_ll_intr_disable(&GPIO, pins->sc);
    clear_pin_intr(pins->sc);
#endif

    return wa_port_calibrate(cal);
}

/* ---- OS ------------------------------------------------------------------------------ */

bool wa_port_spawn(void (*entry)(void *), void *arg)
{
#if CONFIG_FREERTOS_UNICORE
    const BaseType_t core = 0;
#else
    const BaseType_t core = CONFIG_WA_TASK_CORE;
#endif
    return xTaskCreatePinnedToCore(entry, "wireless_adapter", 6144, arg, CONFIG_WA_TASK_PRIORITY, &s_task, core) == pdPASS;
}

uint32_t wa_port_ms(void) { return (uint32_t)(esp_timer_get_time() / 1000); }
uint32_t wa_port_us(void) { return (uint32_t)esp_timer_get_time(); }
void wa_port_delay_us(uint32_t us) { esp_rom_delay_us(us); }
void wa_port_lock(void) { portENTER_CRITICAL(&s_mux); }
void wa_port_unlock(void) { portEXIT_CRITICAL(&s_mux); }
void wa_port_lock_isr(void) { portENTER_CRITICAL_ISR(&s_mux); }
void wa_port_unlock_isr(void) { portEXIT_CRITICAL_ISR(&s_mux); }

void wa_port_wake_isr(void)
{
    BaseType_t woken = pdFALSE;
    if (s_task) vTaskNotifyGiveFromISR(s_task, &woken);
    if (woken) portYIELD_FROM_ISR();
}

void wa_port_wake(void)
{
    if (s_task) xTaskNotifyGive(s_task);
}

void wa_port_wait(uint32_t ms)
{
    TickType_t ticks = pdMS_TO_TICKS(ms);
    ulTaskNotifyTake(pdTRUE, ticks ? ticks : 1);
}

uint16_t wa_port_random16(void) { return (uint16_t)esp_random(); }
