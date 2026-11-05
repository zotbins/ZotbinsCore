"""Saves the photos the ESP32 sends over the USB serial cable

the esp32 prints each photo as hex text between a "JPEG_START <num> <bytes>" line and a "JPEG_END" line
this script listens on the serial port, turns that text back into bytes and saves photo_001.jpg,
photo_002.jpg and so on in the folder i run it from
if a photo comes in broken it still gets saved but as photo_00N_BAD.jpg so i can tell them apart

how i run it (close the vs code serial monitor first, only one program can use the port at a time):
    python capture_photos.py COM3                 live from the board, stop with ctrl+c
    python capture_photos.py --log monitor.txt    from a saved monitor log instead (idea from my teammate's script)
"""
import argparse
import os
import sys

import serial  # pyserial, it already comes with the esp-idf python environment

HEX_CHARS = set("0123456789abcdefABCDEF")
JPEG_HEADER = b"\xff\xd8"  # every jpeg file starts with these 2 bytes


def save_photo(num, expected_len, hex_lines, dropped_lines):
    """turns the hex lines back into bytes, checks them and saves the file
    returns (file name, the bytes) so i can test it without a board"""
    # every line i kept has an even number of hex chars so this cant fail, but i catch it just in case
    try:
        data = bytes.fromhex("".join(hex_lines))
    except ValueError as e:
        print(f"WARNING: photo {num} has bad hex in it, not saving it ({e})")
        return None

    # checks so a broken photo doesnt get mistaken for a good one
    problems = []
    if dropped_lines:
        problems.append(f"{dropped_lines} broken line(s) got thrown out")
    if len(data) != expected_len:
        problems.append(f"expected {expected_len} bytes, got {len(data)}")
    if data[:2] != JPEG_HEADER:
        problems.append("does not start like a jpeg")

    name = f"photo_{num:03d}.jpg" if not problems else f"photo_{num:03d}_BAD.jpg"

    # write to a temp file first and rename it when its done, so if something goes wrong halfway
    # i never end up with a half written or empty jpg sitting there
    tmp_name = name + ".part"
    with open(tmp_name, "wb") as f:
        f.write(data)
    os.replace(tmp_name, name)

    note = "" if not problems else "  WARNING: " + "; ".join(problems)
    print(f"Saved {name} ({len(data)} bytes){note}")
    return name, data


def start_photo(line, state):
    """reads the JPEG_START line, if its cut off or garbled i skip that photo instead of crashing"""
    parts = line.split()
    try:
        num, expected = int(parts[1]), int(parts[2])
    except (IndexError, ValueError):
        print(f"WARNING: couldnt read the start line {line!r}, skipping this photo")
        state.clear()
        return
    if state.get("collecting"):
        # a new photo started before the last one finished (like the board reset), so the old one is lost
        print(f"WARNING: photo {state['num']} never finished, starting photo {num} instead")
    state.update(collecting=True, num=num, expected=expected, lines=[], dropped=0)
    print(f"Receiving photo {num} ({expected} bytes)...")


def handle_line(line, state):
    """handles one line of text from the esp32
    state remembers what photo im in the middle of, returns (file name, bytes) when a photo gets saved"""
    if line.startswith("JPEG_START"):
        start_photo(line, state)
        return None

    if line == "JPEG_END":
        if state.get("collecting"):
            state["collecting"] = False
            return save_photo(state["num"], state["expected"], state["lines"], state["dropped"])
        return None  # an end with no start, nothing to save

    if line and set(line) <= HEX_CHARS:
        if not state.get("collecting"):
            return None  # photo data from a photo i skipped, no point printing hundreds of hex lines
        if len(line) % 2 == 0:
            state["lines"].append(line)
        else:
            # odd number of hex chars means part of the line got lost on the way, i throw it out
            # and the size check will flag the photo as bad
            state["dropped"] += 1
        return None

    if line:
        print(line)  # normal esp32 log lines (or anything that isnt photo data) just get printed
    return None


def lines_from_serial(port):
    """yields one text line at a time from the serial port, forever"""
    # serial_for_url works with normal port names like COM3 too
    ser = serial.serial_for_url(port, do_not_open=True)
    ser.baudrate = 115200   # has to match the esp32 console speed
    ser.timeout = 1         # readline gives up after 1 s so ctrl+c still works
    ser.dtr = False         # keep the board's reset/boot lines idle so the esp32 keeps running
    ser.rts = False
    try:
        ser.open()
    except serial.SerialException as e:
        print(f"Couldnt open {port}: {e}")
        print("Check the port number, and close the vs code serial monitor if its open")
        sys.exit(1)
    print(f"Listening on {port}. Photos are saved in this folder. Press Ctrl+C to stop.")
    try:
        while True:
            raw = ser.readline()
            if raw:
                yield raw.decode("ascii", errors="ignore").strip()
    finally:
        ser.close()


def lines_from_log(path):
    """yields one line at a time from a saved monitor log file"""
    with open(path, encoding="utf-8", errors="ignore") as f:
        for line in f:
            yield line.strip()


def main():
    parser = argparse.ArgumentParser(description="save photos the esp32 sends over serial")
    parser.add_argument("port", nargs="?", default="COM3", help="serial port, like COM3")
    parser.add_argument("--log", help="read a saved monitor log instead of the serial port")
    args = parser.parse_args()

    lines = lines_from_log(args.log) if args.log else lines_from_serial(args.port)
    state = {}
    saved = 0
    try:
        for line in lines:
            if handle_line(line, state):
                saved += 1
    except KeyboardInterrupt:
        print("Stopped.")
    if state.get("collecting"):
        print(f"WARNING: photo {state['num']} was still coming in when it stopped, it wasnt saved")
    print(f"{saved} photo(s) saved")


if __name__ == "__main__":
    main()