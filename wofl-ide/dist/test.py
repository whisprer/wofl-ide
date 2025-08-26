import time
import sys

print("=== WOFL IDE Python Test ===")
print(f"Python version: {sys.version}")
print("Starting countdown...")

for i in range(5, 0, -1):
    print(f"Count: {i}")
    time.sleep(1)

print("Done! Your IDE is working perfectly.")
print("Output colors should be visible now.")
