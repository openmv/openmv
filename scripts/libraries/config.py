import binascii
import machine
from machine import LED
import uasyncio as asyncio
import json
from config_store import ConfigStore

# Encryption policy
ENCRYPTION_ENABLED = True

# Map board UID (hex bytes) to node address.
UID_TO_ADDR = {
    # b'e606fe64d709051c': 216,
    b'e076465dd7193d2a': 217,
    b"e076465dd709102e": 218,
    b"04bd545dd7593b40": 219, # 04bd545dd7593b40
    b"04bd545dd759392c": 220,
    b"e076465dd719431e": 222,
    b"e076465dd7091843": 223,
    b"e076465dd719421e": 224,
    b"e076465dd7194025": 225,
    b"e076465dd7194318": 226,
    b"e076465dd7193a09": 227,
    b"e076465dd7090e41": 228,
    b"e606fe64d7090425": 229,
    b"e606fe64d7110c31": 231,
    b"e606fe64d7110b25": 232,
    b"e606fe64d7091126": 233,
    b"e076465dd7090d1c": 234,
    b"e076465dd7090f42": 235,
    b"04bd545dd759452f": 236,
    b"04bd545dd759362e": 237, 
    b"04bd545dd7594a36": 238,
    b"e076465dd719401d": 239,
    b"04bd545dd7594d2a": 240,
    b'e076465dd7194211': 241,
    b'e606fe64d709093b': 242,
    b'e606fe64d7090616': 243,
    b'98844061d7492821': 244,
    b'04bd545dd7594117': 245,  
    b'04bd545dd7593733': 246,
    b'04bd545dd7594b19': 247, 
    b'04bd545dd759380e': 248,

}

# XX.XX.X
# 02.00.4
major = 2  # (0-63)
minor = 0  # (0-99)
patch = 4  # (0-9)
# version as integer value, max_val = 64_999 < 65_535 (2 bytes)
VERSION = major * 1_000 + minor * 10 + patch

def get_version_str(version):
    """
    Decode packed version integer to XX.XX.X string (inverse of major*1000 + minor*10 + patch).

    Args:
        version (int): Packed version, e.g. 2001 for 2.0.1.

    Returns:
        str: Version string, e.g. "02.00.1".
    """
    patch = version % 10
    minor = (version // 10) % 100
    major = version // 1000
    return "{:02d}.{:02d}.{}".format(major, minor, patch)


uid = binascii.hexlify(machine.unique_id())
my_addr = UID_TO_ADDR.get(uid)

def get_my_addr(default=None):
    """Return node address for current board UID."""
    if my_addr is None:
        print("Error: my_addr not defined for this device.")
        return None
    return my_addr

def uses_rsa_encryption(msg_type):
    if not ENCRYPTION_ENABLED:
        return False
    if msg_type in ["*"]:  # None of the messages are rsa_encrypted
        return True
    return False


def uses_hybrid_encryption(msg_type):
    if not ENCRYPTION_ENABLED:
        return False
    if msg_type == "P":
        return True
    return False


async def led_restart_blinker():
    led = LED("LED_GREEN")
    blink_count = 5
    blink_duration = 0.1
    for i in range(blink_count):
        led.on()
        await asyncio.sleep(blink_duration)
        led.off()
        await asyncio.sleep(blink_duration)

# flash memory config store
store = ConfigStore('/flash', 'nodestate')
def get_key_value(key: str):
    saved_state = store.load()
    if saved_state:
        json_data = json.loads(saved_state)
        result = json_data.get(key, None)
    else:
        result = None
    return result
    
def set_key_value(key: str, value: any):
    saved_state = store.load()
    if saved_state:
        json_data = json.loads(saved_state)
        json_data[key] = value
        store.save(json.dumps(json_data).encode())
    else:
        json_data = {key: value}
        store.save(json.dumps(json_data).encode())

# machines states cruds
def get_deployment_id():
    return get_key_value('deployment_id')

def set_deployment_id(id):
    set_key_value('deployment_id', id)
    
def save_log_entry(log_entry: str):
    log_entries = get_key_value('log_entries')
    if log_entries:
        log_entries.append(log_entry)
        set_key_value('log_entries', log_entries)
    else:
        log_entries = [log_entry]
    set_key_value('log_entries', log_entries)

def get_log_entries():
    return get_key_value('log_entries')

    
def main():
    print("\n======>  MACHINE CONFIGURATION <======")
    print("Version: ", get_version_str(VERSION))
    print("UID: ", uid)
    print("My address: ", get_my_addr())
    print("Encryption enabled: ", ENCRYPTION_ENABLED)
    print("Hybrid encryption enabled: ", uses_hybrid_encryption("P"))
    print("RSA encryption enabled: ", uses_rsa_encryption("*"))
    print("======================================\n\n")
    
    
    from config_store import ConfigStore
    store = ConfigStore('/flash', 'nodestate')
    d = store.load()
    print(json.loads(d) if d else 'no state yet')
    
    store.save(json.dumps({'key1': 10000, 'key2': "10000"}).encode())
    d = store.load()
    print(json.loads(d) if d else 'no state yet')

if __name__ == "__main__":
    main()