def unittest(data_path, temp_path):
    import os

    test_file = temp_path + "/test_file.txt"
    test_binary = temp_path + "/test_binary.bin"

    # Test 1: Basic write and read
    f = open(test_file, "w")
    f.write("Hello, World!")
    f.close()

    f = open(test_file, "r")
    content = f.read()
    f.close()

    if content != "Hello, World!":
        return False

    # Test 2: Write multiple lines and read them back
    f = open(test_file, "w")
    f.write("Line 1\n")
    f.write("Line 2\n")
    f.write("Line 3\n")
    f.close()

    f = open(test_file, "r")
    lines = f.readlines()
    f.close()

    if len(lines) != 3:
        return False
    if lines[0] != "Line 1\n" or lines[1] != "Line 2\n" or lines[2] != "Line 3\n":
        return False

    # Test 3: Seek operations
    f = open(test_file, "w")
    f.write("0123456789")
    f.close()

    f = open(test_file, "r")

    # Seek to position 5
    f.seek(5)
    char = f.read(1)
    if char != "5":
        f.close()
        return False

    # Seek to beginning
    f.seek(0)
    char = f.read(1)
    if char != "0":
        f.close()
        return False

    # Get current position
    pos = f.seek(0, 1)
    if pos != 1:
        f.close()
        return False

    f.close()

    # Test 4: Append mode
    f = open(test_file, "w")
    f.write("Initial")
    f.close()

    f = open(test_file, "a")
    f.write(" Appended")
    f.close()

    f = open(test_file, "r")
    content = f.read()
    f.close()

    if content != "Initial Appended":
        return False

    # Test 5: Binary mode operations
    data = bytearray([0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD])

    f = open(test_binary, "wb")
    f.write(data)
    f.close()

    f = open(test_binary, "rb")
    read_data = f.read()
    f.close()

    if len(read_data) != len(data):
        return False
    for i in range(len(data)):
        if read_data[i] != data[i]:
            return False

    # Test 6: Read with size limit
    f = open(test_file, "w")
    f.write("ABCDEFGHIJ")
    f.close()

    f = open(test_file, "r")
    chunk1 = f.read(3)
    chunk2 = f.read(3)
    remaining = f.read()
    f.close()

    if chunk1 != "ABC" or chunk2 != "DEF" or remaining != "GHIJ":
        return False

    # Test 7: File existence and deletion
    # Create a file
    f = open(test_file, "w")
    f.write("temp")
    f.close()

    # Check if file exists
    f = open(test_file, "r")
    f.close()

    # Delete the file
    os.remove(test_file)

    # Test 8: readline() operation
    f = open(test_file, "w")
    f.write("First\nSecond\nThird")
    f.close()

    f = open(test_file, "r")
    line1 = f.readline()
    line2 = f.readline()
    line3 = f.readline()
    f.close()

    if line1 != "First\n" or line2 != "Second\n" or line3 != "Third":
        return False

    # Test 9: Write and read with different positions
    f = open(test_binary, "wb")
    f.write(bytearray([0xAA, 0xBB, 0xCC, 0xDD]))
    f.close()

    f = open(test_binary, "rb")
    f.seek(2)
    byte1 = f.read(1)
    f.seek(0)
    byte2 = f.read(1)
    f.close()

    if byte1[0] != 0xCC or byte2[0] != 0xAA:
        return False

    return True
