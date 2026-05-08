import ucryptolib
import os
import time as utime
from rsa.pkcs1 import encrypt, decrypt
import logger
import random
import enc_priv
import enc_pub


# ===== REMOVE BEFORE FINALIZING =====
# import omv


class EncNode:
    def __init__(self, my_addr):
        self.my_addr = my_addr
        self.rsa_pub = enc_pub.PubKeyRepo()
        self.pubkey = self.rsa_pub.get_pub_key(my_addr)
        if self.pubkey is None:
            logger.error(f"Public key not found for address {my_addr}")
            raise ValueError(f"Public key not found for address {my_addr}")
        self.rsa_priv = enc_priv.PrivKeyRepo()  # TODO REMOVE

    def get_pub_key(self):
        return self.pubkey

    def get_pub_key_for(self, address):
        return self.rsa_pub.get_pub_key(address)

    def get_prv_key_self(self):
        return self.rsa_priv.get_pvt_key(self.my_addr)
 
    def get_prv_key(self, othernode):
        return self.rsa_priv.get_pvt_key(othernode)

def setup_aes():
    aes_key = os.urandom(32)
    return aes_key

def setup_iv():
    iv = os.urandom(16)
    return iv

def load_rsa(encnode):
    pubkey = encnode.get_pub_key()
    privkey = encnode.get_prv_key_self()
    return pubkey, privkey

def pad(data):
    pad_len = 16 - (len(data) % 16)
    return data + bytes([pad_len] * pad_len)

def unpad(data):
    num_bytes_to_remove = 0 - int(data[-1])
    return data[:num_bytes_to_remove]

def encrypt_aes(msg, aes_key, iv):
    aes = ucryptolib.aes(aes_key, 2, iv)  # mode 2 = CBC
    padded_data = pad(msg)
    encrypted_data = aes.encrypt(padded_data)
    return encrypted_data

def decrypt_aes(encrypted_msg, iv, aes_key):
    aes = ucryptolib.aes(aes_key, 2, iv)  # mode 2 = CBC
    decrypted_msg = unpad(aes.decrypt(encrypted_msg))
    return decrypted_msg

def encrypt_rsa(msgstr, public_key): # Max 117 bytes
    return encrypt(msgstr, public_key)

def decrypt_rsa(msgstr, private_key):
    return decrypt(msgstr, private_key)

def encrypt_hybrid(msg, public_key):
    # AES key 32 bytes, IV 16bytes, Message encrypted.
    iv = os.urandom(16)
    aes_key = os.urandom(32)
    # keys RSA
    iv_rsa = encrypt_rsa(iv, public_key)
    aes_key_rsa = encrypt_rsa(aes_key, public_key)
    # msg, AES
    msg_aes = encrypt_aes(msg, aes_key, iv)
    return aes_key_rsa + iv_rsa + msg_aes

def decrypt_hybrid(msg, private_key):
    aes_key = decrypt_rsa(msg[:128], private_key)
    iv = decrypt_rsa(msg[128:256], private_key)
    msg_decrypt = decrypt_aes(msg[256:], iv, aes_key)
    return msg_decrypt

# Debugging only
def get_rand(n):
    rstr = ""
    for i in range(n):
        rstr += chr(65+random.randint(0,25))
    return rstr

# Debugging only
def test_encryption(encnode, nodeaddr, n2, enctype):
    clock_start = utime.ticks_ms()
    t0 = utime.ticks_diff(utime.ticks_ms(), clock_start)
    if enctype == "RSA" or enctype == "HYBRID":
        public_key, private_key = load_rsa(encnode)
    elif enctype == "AES":
        aes_key = setup_aes()
        iv = setup_iv()
    else:
        logger.error("WRONG INPUT")
        return
    t1 = utime.ticks_diff(utime.ticks_ms(), clock_start)
    logger.info(f"Setup key in time {t1-t0} mili seconds")
    lenstr = 1
    for i in range(1,n2):
        t2 = utime.ticks_diff(utime.ticks_ms(), clock_start)
        teststr = get_rand(lenstr)
        t3 = utime.ticks_diff(utime.ticks_ms(), clock_start)
        if enctype == "RSA":
            teststr_enc = encrypt_rsa(teststr.encode(), public_key)
            # cipher_bytes = teststr_enc.to_bytes((get_bit_length(teststr_enc) + 7) // 8, 'big')
        elif enctype == "AES":
            teststr_enc = encrypt_aes(teststr.encode(), aes_key, iv)
        elif enctype == "HYBRID":
            teststr_enc = encrypt_hybrid(teststr.encode(), public_key)
        else:
            return
        t4 = utime.ticks_diff(utime.ticks_ms(), clock_start)
        if enctype == "RSA":
            teststr_decrypt = decrypt_rsa(teststr_enc, private_key)
        elif enctype == "AES":
            teststr_decrypt = decrypt_aes(teststr_enc, iv, aes_key)
        elif enctype == "HYBRID":
            teststr_decrypt = decrypt_hybrid(teststr_enc, private_key)
        else:
            return
        t5 = utime.ticks_diff(utime.ticks_ms(), clock_start)
        logger.info(f"{enctype}@{nodeaddr} : Encrypting {len(teststr)} to {len(teststr_enc)}, "
            f"creation time = {t3-t2}, enc time = {t4-t3}, decrypt time = {t5-t4}"
        )
        if teststr.encode() != teststr_decrypt:
            logger.error(f"Strings DONT match {teststr} != {teststr_decrypt}")
        lenstr = lenstr*2

def main():
    encnode = EncNode(9)
    test_encryption(encnode, 9, 5, "RSA")
    test_encryption(encnode, 9, 5, "HYBRID")

    encnode = EncNode(221)
    test_encryption(encnode, 221, 5, "RSA")
    test_encryption(encnode, 221, 5, "HYBRID")

    encnode = EncNode(222)
    test_encryption(encnode, 222, 5, "RSA")
    test_encryption(encnode, 222, 5, "HYBRID")

    encnode = EncNode(223)
    test_encryption(encnode, 223, 5, "RSA")
    test_encryption(encnode, 223, 5, "HYBRID")

if __name__ == "__main__":
    main()
