import serial
import time
import sys

SERIAL_PORT = "/dev/tty.usbserial-0001"      # replace with your port (e.g., /dev/ttyUSB0 on Linux/macOS)
BAUD_RATE = 115200
OUTPUT_FILE = "probes.csv"
TIMEOUT = 5          

# Setup serial with the right port
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
except Exception as e:
    print(f"Failed to open {SERIAL_PORT}: {e}")
    sys.exit(1)

time.sleep(1)  #give the esp32 a second to boot just to be safe


ser.write(b"GET_CSV\n") #send the command to get the csv
ser.flush()

lines = []
start_time = time.time()
print("Retrieving CSV...")

while True:
    line = ser.readline()
    if not line:
        # check for timeout
        if time.time() - start_time > TIMEOUT:
            print("Timeout reached")
            break
        continue
    decoded = line.decode(errors="ignore").strip()
    if decoded == "---END---": #check for the EOF
        break
    lines.append(decoded)


if lines: #if we actually found something then append it to our current csv
    with open(OUTPUT_FILE, "a", encoding="utf-8") as f:
        for l in lines:
            f.write(l + "\n")
    print(f"Saved {len(lines)} lines to {OUTPUT_FILE}")
else:
    print("No data received.")

ser.close()
