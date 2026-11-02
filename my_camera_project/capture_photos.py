"""Save photos sent by the ESP32-CAM over the USB serial cable.

The ESP32 prints each photo as hex text between a "JPEG_START <num> <bytes>" line and a
"JPEG_END" line. This script listens on the serial port, turns that text back into bytes,
and saves photo_001.jpg, photo_002.jpg, ... in the folder you run it from.

Usage (close the VS Code serial monitor first, only one program can use the port):
    python capture_photos.py COM3
Stop with Ctrl+C.
"""
import sys

import serial  # pyserial, already installed with ESP-IDF

HEX_CHARS = set("0123456789abcdefABCDEF")


def save_photo(num, expected_len, hex_lines):
    data = bytes.fromhex("".join(hex_lines))
    name = f"photo_{num:03d}.jpg"
    with open(name, "wb") as f:
        f.write(data)
    problems = []
    if len(data) != expected_len:
        problems.append(f"expected {expected_len} bytes, got {len(data)}")
    if data[:2] != b"\xff\xd8":
        problems.append("does not start like a JPEG")
    note = "" if not problems else "  WARNING: " + "; ".join(problems)
    print(f"Saved {name} ({len(data)} bytes){note}")
    return name, data


def handle_line(line, state):
    """Process one text line. state holds what we are in the middle of receiving."""
    if line.startswith("JPEG_START"):
        parts = line.split()
        state.update(collecting=True, num=int(parts[1]), expected=int(parts[2]), lines=[])
        print(f"Receiving photo {state['num']} ({state['expected']} bytes)...")
        return None
    if line == "JPEG_END" and state.get("collecting"):
        state["collecting"] = False
        return save_photo(state["num"], state["expected"], state["lines"])
    if state.get("collecting") and line and set(line) <= HEX_CHARS:
        state["lines"].append(line)
        return None
    if line:
        print(line)  # normal ESP32 log lines
    return None


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else "COM3"
    ser = serial.Serial()
    ser.port = port
    ser.baudrate = 115200
    ser.timeout = 1
    ser.dtr = False  # keep the MB's reset/boot lines idle so the ESP32 keeps running
    ser.rts = False
    ser.open()
    print(f"Listening on {port}. Photos are saved in this folder. Press Ctrl+C to stop.")
    state = {}
    try:
        while True:
            raw = ser.readline()
            if raw:
                handle_line(raw.decode("ascii", errors="ignore").strip(), state)
    except KeyboardInterrupt:
        print("Stopped.")
    finally:
        ser.close()


if __name__ == "__main__":
    main()