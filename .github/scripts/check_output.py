from pathlib import Path

# TODO: The main.cpp file should print progress markers with a given string in them and this should look only through each of those in order and match them to expected result.

if __name__ == '__main__':
    data = Path('serial.log').read_text()
    if "GNU GRUB" not in data:
        print("ERROR: Didn't load GRUB.")
        exit(1)
    if "Shell started" not in data:
        # TODO: this is highlighting an issue with logging. There should be a better way to handle this.
        # TODO: https://sematext.com/blog/logging-levels/
        print("ERROR: Failed to load shell")
        exit(1)
    if "Starting Process: doom.art" not in data:
        print("ERROR: Failed to load doom")
        exit(1)
    if "I_InitGraphics: Auto-scaling factor:" not in data:
        print("ERROR: Failed to load doom main menu")
        exit(1)
    if "Exiting doom.art PID:" not in data:
        print("ERROR: failed to exit doom using menu")
        exit(1)
    if "int_no, err_code:" in data:
        print("ERROR: FATAL os error. See logs for more detail.")
        exit(1)
    if data.split('\n')[-1] != "" or "doom.art exited with exit code 0" not in data.split('\n')[-2]:
        print("ERROR: failed to exit doom")
        exit(1)
    exit(0)
