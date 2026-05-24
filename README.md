# CPartsClassification

A real-time industrial parts classification system implemented in C++. The project simulates a factory production line with multiple conveyor belts and a robotic arm that sorts incoming parts into quality pallets based on dimensional tolerance.

---

## Features

- **Dual conveyor belt simulation** — two independent belt threads read parts from data files in real-time
- **Concurrent sensor processing** — each belt runs its own thread, continuously polling for new parts
- **Robotic arm classifier** — a dedicated thread per part that measures diameter deviation and routes each piece to the correct pallet
- **Semaphore-based synchronization** — guarantees the robotic arm handles only one part at a time, preventing race conditions
- **File-based I/O pipeline** — belt state is persisted in text files; classified output is appended to CSV pallets for traceability
- **Graceful shutdown** — press any key to signal all threads to stop cleanly

---

## Technologies

| Layer | Detail |
|---|---|
| Language | C++17 |
| Threading | Windows API — `CreateThread`, `WaitForSingleObject` |
| Synchronization | Windows Semaphore — `CreateSemaphore`, `ReleaseSemaphore` |
| I/O | Standard file streams (`fstream`) |
| Build target | Windows (MSVC / MinGW) |

---

## How to Build and Run

### Prerequisites
- Windows OS
- A C++ compiler with Windows API support: **MinGW-w64** or **MSVC** (Visual Studio)

### Build with MinGW

```bash
g++ -o partsClassification main.cpp
```

### Build with MSVC

```bash
cl main.cpp /Fe:partsClassification.exe
```

### Prepare the input data

Create two plain-text belt files in the same directory as the executable. Each line represents one part with the format:

```
<sequence_number> <manufacturer_code> <part_code> <diameter_mm>
```

**Example — `cinta1.txt`:**
```
1 A 101 20.1
2 B 202 21.5
3 C 303 25.0
```

**Example — `cinta2.txt`:**
```
4 D 404 19.8
5 E 505 18.0
```

### Run

```bash
./partsClassification
```

Press any key to stop. Classified parts are written to:

| File | Condition |
|---|---|
| `pallet-1.csv` | Diameter within ±2% of reference (20 mm) — **accepted** |
| `pallet-2.csv` | Diameter within ±5% of reference — **rework** |
| `pallet-3.csv` | Diameter deviation > 5% — **rejected** |

---

## System Architecture

```
main()
 ├── gestionHilos()                  [management thread]
 │    ├── lecturaSensor("cinta1.txt") [belt 1 sensor thread]
 │    │    └── moverBrazo(part)       [arm thread per part]
 │    └── lecturaSensor("cinta2.txt") [belt 2 sensor thread]
 │         └── moverBrazo(part)       [arm thread per part]
 └── [waits for keypress → sets detenerCintas = true]
```

### Data flow

1. Belt sensor threads continuously read the first line of their respective `.txt` file and shift the remaining content (simulating a physical conveyor advancing).
2. Each part is passed to a new `moverBrazo` thread via a heap-allocated string.
3. `moverBrazo` parses the part data, acquires the semaphore (arm lock), calculates the percentage deviation from the 20 mm reference diameter, and appends the result to the appropriate CSV pallet.
4. The semaphore is released, allowing the next part to be processed.

### Classification thresholds

```
Reference diameter: 20.0 mm

  |deviation| ≤ 2%  →  pallet-1.csv  (high precision)
  |deviation| ≤ 5%  →  pallet-2.csv  (within tolerance)
  |deviation| > 5%  →  pallet-3.csv  (out of spec)
```

---

## Project Structure

```
partsClassification/
├── main.cpp        # Full application source
├── cinta1.txt      # Belt 1 input (user-provided)
├── cinta2.txt      # Belt 2 input (user-provided)
├── pallet-1.csv    # Output: accepted parts
├── pallet-2.csv    # Output: rework parts
└── pallet-3.csv    # Output: rejected parts
```

---

## Future Improvements

- **Cross-platform support** — replace Windows API calls with `std::thread` and `std::mutex` from the C++11 standard library to support Linux and macOS
- **Dynamic belt scaling** — accept a configurable number of conveyor belts at startup instead of two hardcoded instances
- **Live dashboard** — display a real-time terminal UI showing belt activity, pallet counts, and arm status
- **Network input** — replace file polling with a socket-based data stream to connect to real sensor hardware
- **Persistent logging** — add structured logging with timestamps for audit trails in production environments
- **Unit tests** — add test coverage for the classification logic and file parsing routines

---

## Author

Joel V — Embedded Systems & Industrial Automation
