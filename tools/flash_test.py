#!/usr/bin/env python3
"""
DFU flash test utility.
Tests flash read/write operations by comparing checksums of random data,
or verifies flash contents against a binary file.
"""

import argparse
import subprocess
import time
import hashlib
import os
import sys


class Colors:
    """ANSI color codes for terminal output."""
    RESET = '\033[0m'
    BOLD = '\033[1m'
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    MAGENTA = '\033[95m'
    CYAN = '\033[96m'


def print_color(text, color='', bold=False):
    """Print colored text."""
    prefix = Colors.BOLD if bold else ''
    print(f"{prefix}{color}{text}{Colors.RESET}")


def print_header(text):
    """Print a section header."""
    print_color(f"\n{text}", Colors.CYAN, bold=True)


def print_success(text):
    """Print success message."""
    print_color(f"✓ {text}", Colors.GREEN, bold=True)


def print_error(text):
    """Print error message."""
    print_color(f"✗ {text}", Colors.RED, bold=True)


def print_warning(text):
    """Print warning message."""
    print_color(f"⚠ {text}", Colors.YELLOW)


def run_command(cmd, description=None, show_output=True):
    """Run a command and return the result."""
    if description:
        print(f"{description}...")
    if show_output:
        # Stream output directly to terminal
        result = subprocess.run(cmd, shell=True)
    else:
        # Capture output
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
        print_error(f"Command failed")
        sys.exit(1)
    return result


def calculate_md5(filename):
    """Calculate MD5 hash of a file."""
    hash_md5 = hashlib.md5()
    with open(filename, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()


def get_file_size(filename):
    """Get file size in bytes."""
    return os.path.getsize(filename)


def format_size(size_bytes):
    """Format size in human-readable format."""
    for unit in ['B', 'KB', 'MB', 'GB']:
        if size_bytes < 1024.0:
            return f"{size_bytes:.2f} {unit}"
        size_bytes /= 1024.0
    return f"{size_bytes:.2f} TB"


def verify_mode(device_id, partition, binary_file):
    """Verify flash contents against a binary file."""
    print_header("DFU Verification Mode")

    if not os.path.exists(binary_file):
        print_error(f"Binary file '{binary_file}' does not exist")
        sys.exit(1)

    file_size = get_file_size(binary_file)

    print_color(f"Binary file: {binary_file}", Colors.BLUE)
    print_color(f"File size: {format_size(file_size)} ({file_size} bytes)", Colors.BLUE)
    print_color(f"Partition: {partition}", Colors.BLUE)
    print_color(f"Device ID: {device_id}", Colors.BLUE)

    # Create temporary dump file
    dump_file = "/tmp/dfu_dump.bin"
    if os.path.exists(dump_file):
        os.remove(dump_file)
    print_color(f"Temp dump file: {dump_file}", Colors.BLUE)

    # Read from flash
    print_header("Reading from flash")
    start_time = time.time()
    run_command(f"dfu-util -d {device_id} -a {partition} -U {dump_file}")
    read_time = time.time() - start_time

    if not os.path.exists(dump_file):
        print_error("Failed to read from flash")
        sys.exit(1)

    print_color(f"Read completed in {read_time:.3f}s", Colors.GREEN)

    # Truncate dump file to match binary file size
    run_command(
        f"truncate --size={file_size} {dump_file}",
        "Truncating dump file",
        show_output=False
    )

    # Calculate and compare MD5 checksums
    print_header("Verifying checksums")
    binary_md5 = calculate_md5(binary_file)
    dump_md5 = calculate_md5(dump_file)

    print(f"  {os.path.basename(binary_file)}: {binary_md5}")
    print(f"  {os.path.basename(dump_file)}: {dump_md5}")

    if binary_md5 == dump_md5:
        print_success("Checksums match! Flash contents verified.")
        print_color(f"\nPerformance:", Colors.MAGENTA, bold=True)
        print(f"  Read speed: {format_size(file_size / read_time)}/s")
        # Clean up temp file on success
        os.remove(dump_file)
        return 0
    else:
        print_error("Checksums do not match! Flash verification failed.")
        print_warning(f"Dump file saved for inspection: {dump_file}")
        return 1


def test_mode(device_id, partition, size):
    """Test flash by writing random data and verifying."""
    print_header("DFU Flash Test Mode")

    print_color(f"Partition: {partition}", Colors.BLUE)
    print_color(f"Size: {size}", Colors.BLUE)
    print_color(f"Device ID: {device_id}", Colors.BLUE)

    # Create temporary files
    random_file = "/tmp/dfu_random.bin"
    dump_file = "/tmp/dfu_dump.bin"

    # Clean up any existing files
    for f in [random_file, dump_file]:
        if os.path.exists(f):
            os.remove(f)

    print_color(f"Temp random file: {random_file}", Colors.BLUE)
    print_color(f"Temp dump file: {dump_file}", Colors.BLUE)

    # Generate random data file
    print_header("Generating random data")
    run_command(
        f"dd if=/dev/urandom of={random_file} bs={size} count=1 2>&1",
        f"Creating {size} random data file"
    )

    file_size = get_file_size(random_file)
    print_color(f"Generated {format_size(file_size)} ({file_size} bytes)", Colors.GREEN)

    # Write to flash
    print_header("Writing to flash")
    start_time = time.time()
    run_command(f"dfu-util -d {device_id} -a {partition} -D {random_file}")
    write_time = time.time() - start_time
    print_color(f"Write completed in {write_time:.3f}s", Colors.GREEN)

    # Read from flash
    print_header("Reading from flash")
    start_time = time.time()
    run_command(f"dfu-util -d {device_id} -a {partition} -U {dump_file}")
    read_time = time.time() - start_time
    print_color(f"Read completed in {read_time:.3f}s", Colors.GREEN)

    # Truncate dump file to match original size
    run_command(
        f"truncate --size={size} {dump_file}",
        "Truncating dump file",
        show_output=False
    )

    # Calculate and compare MD5 checksums
    print_header("Verifying data integrity")
    random_md5 = calculate_md5(random_file)
    dump_md5 = calculate_md5(dump_file)

    print(f"  {os.path.basename(random_file)}: {random_md5}")
    print(f"  {os.path.basename(dump_file)}: {dump_md5}")

    if random_md5 == dump_md5:
        print_success("Data integrity verified! Flash test passed.")
        print_color(f"\nPerformance:", Colors.MAGENTA, bold=True)
        print(f"  Write speed: {format_size(file_size / write_time)}/s")
        print(f"  Read speed: {format_size(file_size / read_time)}/s")
        # Clean up temp files on success
        os.remove(random_file)
        os.remove(dump_file)
        return 0
    else:
        print_error("Checksums do not match! Flash test failed.")
        print_warning(f"Files saved for inspection:")
        print_warning(f"  Random: {random_file}")
        print_warning(f"  Dump: {dump_file}")
        return 1


def main():
    parser = argparse.ArgumentParser(
        description="Test DFU flash operations or verify flash contents against a binary",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Test mode: Write random data and verify
  %(prog)s -p 3 -s 16M -v 37c5
  %(prog)s -p 5 -s 8M -v 37c5 -i 9206

  # Verify mode: Compare flash contents to a binary file
  %(prog)s -p 3 -v 37c5 -b firmware.bin
  %(prog)s -p 5 -v 37c5 -i 9206 -b bootloader.bin
"""
    )
    parser.add_argument(
        "-p", "--partition",
        type=int,
        required=True,
        help="Partition number"
    )
    parser.add_argument(
        "-s", "--size",
        help="Partition size for test mode (e.g., 16M, 8M, 1024K)"
    )
    parser.add_argument(
        "-v", "--vendor-id",
        required=True,
        help="USB Vendor ID (hex without 0x prefix, e.g., 37c5, 37c5)"
    )
    parser.add_argument(
        "-i", "--product-id",
        default=None,
        help="USB Product ID (hex without 0x prefix, e.g., 9206)"
    )
    parser.add_argument(
        "-b", "--binary",
        help="Binary file to verify against (enables verify mode)"
    )

    # Show help if no arguments provided
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(0)

    args = parser.parse_args()

    # Construct device ID string
    if args.product_id:
        device_id = f"{args.vendor_id}:{args.product_id}"
    else:
        device_id = args.vendor_id

    # Determine mode
    if args.binary:
        # Verify mode
        return verify_mode(device_id, args.partition, args.binary)
    else:
        # Test mode
        if not args.size:
            print_error("Size (-s) is required for test mode")
            parser.print_help()
            sys.exit(1)
        return test_mode(device_id, args.partition, args.size)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print_warning("\n\nOperation cancelled by user")
        sys.exit(130)
