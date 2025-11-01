def unittest(data_path, temp_path):
    try:
        import crc
    except ImportError:
        raise Exception("crc module unavailable")

    # Test data
    test_data1 = b"Hello, World!"
    test_data2 = b"OpenMV"
    test_data3 = b"1234567890"

    # Test 1: Basic CRC16 calculation
    crc16_result1 = crc.crc16(test_data1)
    if not isinstance(crc16_result1, int):
        return False

    # CRC16 should be 16-bit value (0-65535)
    if crc16_result1 < 0 or crc16_result1 > 0xFFFF:
        return False

    # Test 2: Basic CRC32 calculation
    crc32_result1 = crc.crc32(test_data1)
    if not isinstance(crc32_result1, int):
        return False

    # CRC32 should be 32-bit value
    if crc32_result1 < 0 or crc32_result1 > 0xFFFFFFFF:
        return False

    # Test 3: Consistency - same input should give same output
    crc16_result1_repeat = crc.crc16(test_data1)
    if crc16_result1 != crc16_result1_repeat:
        return False

    crc32_result1_repeat = crc.crc32(test_data1)
    if crc32_result1 != crc32_result1_repeat:
        return False

    # Test 4: Different inputs should give different outputs
    crc16_result2 = crc.crc16(test_data2)
    if crc16_result1 == crc16_result2:
        return False

    crc32_result2 = crc.crc32(test_data2)
    if crc32_result1 == crc32_result2:
        return False

    # Test 5: Incremental CRC16 calculation
    # Calculate CRC of concatenated data in one go
    combined_data = test_data1 + test_data2
    crc16_combined = crc.crc16(combined_data)

    # Calculate CRC incrementally
    crc16_part1 = crc.crc16(test_data1)
    crc16_incremental = crc.crc16(test_data2, value=crc16_part1)

    # Both methods should give same result
    if crc16_combined != crc16_incremental:
        return False

    # Test 6: Incremental CRC32 calculation
    crc32_combined = crc.crc32(combined_data)

    crc32_part1 = crc.crc32(test_data1)
    crc32_incremental = crc.crc32(test_data2, value=crc32_part1)

    if crc32_combined != crc32_incremental:
        return False

    # Test 7: Multi-step incremental CRC16
    crc16_step1 = crc.crc16(test_data1)
    crc16_step2 = crc.crc16(test_data2, value=crc16_step1)
    crc16_step3 = crc.crc16(test_data3, value=crc16_step2)

    # Should match single calculation
    all_data = test_data1 + test_data2 + test_data3
    crc16_all = crc.crc16(all_data)

    if crc16_step3 != crc16_all:
        return False

    # Test 8: Multi-step incremental CRC32
    crc32_step1 = crc.crc32(test_data1)
    crc32_step2 = crc.crc32(test_data2, value=crc32_step1)
    crc32_step3 = crc.crc32(test_data3, value=crc32_step2)

    crc32_all = crc.crc32(all_data)

    if crc32_step3 != crc32_all:
        return False

    # Test 9: Empty data handling
    empty_data = b""
    crc16_empty = crc.crc16(empty_data)
    crc32_empty = crc.crc32(empty_data)

    # Empty data should return valid CRC values
    if not isinstance(crc16_empty, int) or not isinstance(crc32_empty, int):
        return False

    # Test 10: Single byte data
    single_byte = b"A"
    crc16_single = crc.crc16(single_byte)
    crc32_single = crc.crc32(single_byte)

    if not isinstance(crc16_single, int) or not isinstance(crc32_single, int):
        return False

    return True
