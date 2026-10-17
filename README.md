# Plant Water Stress Monitor (CWSI)

An embedded real-time plant water-stress monitoring system written in C17.
It continuously computes your plant's stress using the **Crop Water Stress
Index (CWSI)** by combining leaf temperature measurements with ambient
temperature and relative humidity over Linux I2C.

---

## Overview

Over the summer exam season of 2026, London underwent a malignant heatwave.
Meaning my favourite houseplant and I were both stressed, one from the heat
and one from exams (you can guess which way around).

As part of our C project extension in first year computing, I convinced my
group to help me save my plant. Of course, we would use technology to do this,
rather than researching plants.

We realised it would be easy to take care of a plant if it could tell you
when it was stressed and needed water, so we tried talking to it, but every
plant we came across seemed unresponsive - perhaps they were already too
stressed, and in dire need of help! We would have to act fast.

Thus, we needed a way for plants to communicate to us even while
incapacitated, so we used the CWSI, a stress-index for plants.
It uses the mechanism of transpiration in plants:
When a plant has ample water, it can transpire freely, which cools it down
under heat stress (such as the sun).
However when a plant lacks water, its stomata close, reducing
transpiration rate, meaning it struggles to cool down. So its leaf
temperature rises relative to the air temperature.

To calculate this stress index, we wired up an infrared sensor, and an air
temperature and humidity sensor to a Raspberry Pi. Then wrote this codebase
to run the calculations. Halfway through, we realised while testing that we
didn't want to go back to secondary school science classes, frantically
recording results and manually looking to remove anomalies.
That would be slow and boring and *manual*.

So we decided to make the computer do all the work: this is our validity
state machine. It took no less than many hours to get working and tested,
but at least it saved us from needing to do under an hour of manual result
checking! #worthit #efficiency #yayplants

After looking at results in the terminal for too long, we decided to get a
cute little OLED screen (this model was much smaller than we thought, would
not recommend) to view our results, just like the mythical 'user' would.
I say mythical as we were the only user (it's ok this was just a learning
exercise at the end of the day).

Thus we hooked it up to my houseplant, and... it told us to research plant
science before trying to help it. Heeding the wishes of the plant, I did do
this and managed to save it, now it's back to its old self, green and healthy.
Although still not the most talkative...

---

## Scientific Background & Mathematical Model

The Crop Water Stress Index (Idso et al., 1981; Jackson et al., 1981)
normalizes the canopy-to-air temperature differential against atmospheric
demand:

```text
CWSI = (ΔT - ΔT_ll) / (ΔT_ul - ΔT_ll)
```

Where:

- `ΔT = T_canopy - T_air` (°C)
- `ΔT_ll = a + b * VPD` (Non-water-stressed lower baseline, fully transpiring)
- `ΔT_ul` (Non-transpiring upper limit, fully stressed stomatal closure)
- `VPD` is the Vapour Pressure Deficit (kPa), derived via Tetens' equation:

```text
e_s = 0.6108 * exp(17.27 * T_air / (T_air + 237.3))
VPD = e_s * (1.0 - RH / 100.0)
```

### Emissivity Correction

Raw infrared radiometric temperatures are corrected using target leaf
emissivity (ε ≈ 0.97):

```text
T_canopy = ((T_obj^4 - (1 - ε) * T_amb^4) / ε)^(1/4)
```

Sources:

- https://www.sciencedirect.com/science/article/pii/S0378377422001226
- https://en.wikipedia.org/wiki/Tetens_equation
- https://www.researchgate.net/figure/Leaf-emissivity-values-for-five-composited-vegetation-land-cover-types_tbl1_321365208

---

## Hardware Architecture & Pinout

All sensors communicate over a single shared I2C bus (default: `/dev/i2c-1`):

| Device | Type | Default I2C Address | Description |
| :--- | :--- | :--- | :--- |
| **MLX90614** | Sensor | `0x5A` | IR thermometer (35° FOV version) |
| **SHT31** | Sensor | `0x44` | Air temperature & relative humidity |
| **SSD1306** | Display | `0x3C` | 128 × 64 OLED |

```text
                 +--------------------------+
                 | Host (RPi)               |
                 | Linux I2C (`/dev/i2c-1`) |
                 +------------+-------------+
                              |
       +----------------------+----------------------+
       | (SDA / SCL)          |                      |
+------+--------+     +-------+-------+      +-------+-------+
|   MLX90614    |     |     SHT31     |      |    SSD1306    |
| IR Thermometer|     | Temp/Humidity |      | 128x64 OLED   |
| Addr: 0x5A    |     | Addr: 0x44    |      | Addr: 0x3C    |
+---------------+     +---------------+      +---------------+
```

---

## Project Structure

```text
.
├── Makefile
├── results/                # Output: experiment logs
├── src/
│   ├── main.c              # Entry point & measurement loop
│   ├── config.[c|h]        # Config & calibration constants
│   ├── cwsi.[c|h]          # Stress index equations (CWSI, VPD)
│   ├── display.[c|h]       # SSD1306 OLED driver & frame rendering
│   ├── filter.[c|h]        # Mean & variance accumulator
│   ├── font5x7.h           # ASCII bitmap font glyphs for OLED
│   ├── i2c.[c|h]           # Linux I2C ioctl wrappers
│   ├── logger.[c|h]        # CSV data logging
│   ├── mlx90614.[c|h]      # MLX90614 driver
│   ├── sht31.[c|h]         # SHT31 driver
│   └── validity.[c|h]      # Validity state evaluator
└── test/
    ├── test_main.c         # Test suite harness
    ├── test_cwsi.c         # CWSI & VPD calculation unit tests
    ├── test_filter.c       # Ring buffer and variance tests
    ├── test_validity.c     # Validity state transition tests
    ├── test_config.c       # Config parameter tests
    └── test_util.[c|h]     # Test utils
```

---

## Build & Installation

### Prerequisites

- Linux running on Raspberry Pi 3B (other hardware may work)
- GCC with C17 support
- Linux I2C headers (i2c-dev)

### Compilation

Build the production monitor executable:

```bash
make
```

This produces the standalone binary `cwsi_monitor`.

---

## Usage

```bash
./cwsi_monitor [-o log_path] [-p sample_period_s]
```

### Options

- `-o <path>`: Destination CSV log path (default: `~/logs/cwsi_log.csv`).
- `-p <sec>`: Sampling period in seconds (default: 1).

### Example

```bash
# Run with a 5-second interval and custom log file
./cwsi_monitor -o results/experiment1.csv -p 5
```

### Display & Console Output

Console output provides real-time telemetry:

```text
# CWSI monitor on /dev/i2c-1, logging to results/experiment1.csv
cwsi=0.214  dt=-2.15  vpd=1.420  VALID
cwsi=0.220  dt=-2.12  vpd=1.418  VALID
```

Validity states include:

- `WARMUP`: Ring buffer filling up to `filter_window` capacity.
- `VALID`: Readings are stable and within physiological limits.
- `INVALID_UNSTABLE`: Canopy temperature variance exceeds `variance_max`.
- `INVALID_RANGE`: `T_air`, `RH`, or `ΔT` are outside operational thresholds.

---

## Calibration Workflow

To adapt CWSI for a specific species and microclimate, empirical baselines must
be established:

1. **Lower Baseline (`ΔT_ll`):**
   - Mist leaves under sunlight to measure the maximum transpiration baseline
     (`a + b * VPD`).
2. **Upper Baseline (`ΔT_ul`):**
   - Inhibit transpiration (we applied petroleum jelly) to determine the
     non-transpiring thermal ceiling.
3. **Update values in `src/config.c`:**

   ```c
   cfg->cwsi = (cwsi_params_t){
       .a     = -6.70, // Baseline intercept (°C)
       .b     =  0.00, // Baseline slope vs VPD (°C/kPa)
       .dt_ul = -0.49  // Non-transpiring upper limit (°C)
   };
   ```

---

## Running Tests

The test suite checks accuracy of cwsi calculations, ring buffer bounds, edge
case maths, validity state transitions and config values.
Acts as software unit and integration tests without any hardware involved.

```bash
make test
```

Full integration test is running the whole thing with hardware and seeing if
your plant is dying...hopefully not!
