# Transport Catalogue

A command-line C++17 application that ingests public transit data via JSON, manages an in-memory route catalogue, and renders geo-projected SVG maps of bus routes and stops — built entirely from scratch with no third-party libraries.

---

## Features

- **Route & stop management** — add bus routes (linear or circular/loop) and stops with GPS coordinates
- **Statistical queries** — retrieve route info (stop count, unique stops, total distance, curvature) and stop info (buses serving a stop)
- **SVG map rendering** — projects real-world lat/lon coordinates onto a 2D canvas using sphere projection, draws polylines for routes and labeled circles for stops with a configurable color palette
- **JSON I/O** — reads all input (base data + stat queries + render settings) from a single JSON document on `stdin`; writes query results to `stdout`

---

## Technical Highlights

| Area | Detail |
|---|---|
| Language | C++17 |
| JSON | Custom parser and builder — no external libraries (e.g. nlohmann) |
| SVG | Custom rendering engine for shapes, colors (RGB/RGBA), polylines, and text |
| Map projection | Sphere-to-2D projector that scales GPS coordinates to fit a configurable canvas with padding |
| Data structures | `unordered_map`-based O(1) lookups for buses, stops, distances, and cross-references |
| Architecture | Clean separation: domain model → catalogue → JSON I/O → renderer |

---

## Project Structure

```
TransortCatalogue/src/
├── main.cpp                  Entry point — reads JSON from stdin, runs queries
├── transport_catalogue.h/cpp Core data store (buses, stops, distances)
├── domain.h/cpp              Entity definitions (Bus, Stop, BusRoute, render info)
├── geo.h/cpp                 GPS coordinates and haversine distance calculation
├── json.h/cpp                JSON AST (Node, Document, Load)
├── json_builder.h/cpp        Fluent JSON output builder
├── json_reader.h/cpp         Parses input queries and drives the catalogue
├── map_renderer.h/cpp        SVG map orchestration and sphere projection
└── svg.h/cpp                 SVG primitives (colors, shapes, polylines, text)
```

---

## Build

Requires a C++17-compatible compiler (GCC 9+, Clang 10+, MSVC 2019+) and CMake 3.15+.

```bash
cmake -S . -B build
cmake --build build
```

The binary will be at `build/transport_catalogue` (Linux/macOS) or `build/Debug/transport_catalogue.exe` (Windows).

---

## Usage

The program reads a single JSON document from `stdin`. The document has three top-level keys:

| Key | Purpose |
|---|---|
| `base_requests` | Defines stops (with coordinates and distances) and bus routes |
| `stat_requests` | Queries to run — bus route stats or stop info |
| `render_settings` | Canvas size, colors, font sizes, label offsets, etc. |

**Run with a file:**
```bash
./build/transport_catalogue < input.json
```

**Example input structure:**
```json
{
  "base_requests": [
    { "type": "Stop", "name": "Tolstopaltsevo", "latitude": 55.611087, "longitude": 37.208290,
      "road_distances": { "Marushkino": 3900 } },
    { "type": "Bus", "name": "256", "stops": ["Tolstopaltsevo", "Marushkino"], "is_roundtrip": false }
  ],
  "stat_requests": [
    { "id": 1, "type": "Bus", "name": "256" },
    { "id": 2, "type": "Stop", "name": "Tolstopaltsevo" }
  ],
  "render_settings": {
    "width": 1200, "height": 1200, "padding": 50,
    "line_width": 14, "stop_radius": 5,
    "bus_label_font_size": 20, "bus_label_offset": [7, 15],
    "stop_label_font_size": 18, "stop_label_offset": [7, -3],
    "underlayer_color": [255, 255, 255, 0.85], "underlayer_width": 3,
    "color_palette": ["green", [255, 160, 0], "red"]
  }
}
```

**Example output:**
```json
[
  { "request_id": 1, "curvature": 1.36258, "route_length": 27600, "stop_count": 11, "unique_stop_count": 6 },
  { "request_id": 2, "buses": ["256", "828"] }
]
```

The SVG map is written to `stdout` alongside the JSON responses when render settings are provided.

---

## Why This Project

This project was built as a deep-dive into systems programming in C++. Every component — JSON parsing, SVG generation, coordinate projection — was implemented from the ground up to practice:

- Designing clean, composable class interfaces
- Managing memory efficiently with `string_view`, `deque`-backed stable pointers, and custom hash maps
- Implementing a recursive-descent parser for a non-trivial grammar (JSON)
- Applying geometric transformations (sphere projection) for real-world data visualization
