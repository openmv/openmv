import asyncio
import gc
import hashlib
import os
import ubinascii
import utime

import logger


class StoreUtils:

    # SD card write lock
    filesave_lock = asyncio.Lock()

    def __init__(self) -> None:
        """
        Base initialisation for filesystem helpers.

        Holds the per-instance FS stats counters that mirror the globals
        used in nv2/main.register_fs_succ.
        """
        self.file_system_err = 0
        self.file_system_success = 0
        self.file_system_consecutive_err = 0

    # ------------------------------------------------------------------
    # FS success/error tracking (equivalent of nv2/main.register_fs_succ)
    # ------------------------------------------------------------------
    def register_fs_succ(self, succ: bool) -> None:
        """
        Track filesystem success/failure counts.

        Mirrors logic from nv2/main.register_fs_succ:
          - increments success counter and resets consecutive_err on success
          - increments err and consecutive_err on failure
        """
        if succ:
            self.file_system_success += 1
            self.file_system_consecutive_err = 0
        else:
            self.file_system_err += 1
            self.file_system_consecutive_err += 1

    async def save_file_once(self, data, filepath):
        async with self.filesave_lock:
            try:
                with open(filepath, "wb") as f:
                    f.write(data)
                    os.sync()
            except Exception as e:
                logger.error(f"Could not save encrypted file {filepath} : {e}")
                return False
            logger.info(f"[FS] Saved datafile: {filepath}, datasize: {len(data)} bytes")
            return True

    async def read_file_once(self, filepath):
        async with self.filesave_lock:
            try:
                with open(filepath, "rb") as f:
                    data = f.read()
                    logger.debug(
                        f"[FS] ****************** Readable datafile, datasize: {len(data)} bytes"
                    )
                    return True, data
            except Exception as e:
                logger.error(f"[FS] -----  Failed to read file : {filepath}, e: {e}")
        return False, None

    async def save_file(self, data, filepath):
        logger.info(
            f"[FS] Saving datafile to {filepath}, datasize: {len(data)} bytes..."
        )
        try:

            for i in range(3):
                succ = await self.save_file_once(data, filepath)
                if succ:
                    await asyncio.sleep(2)
                    logger.info(f"Writtend : {filepath}")
                    readable, _ = await self.read_file_once(filepath)
                    if readable:
                        logger.info(f"Writtend and readable : {filepath}")
                        # notify DbStore-style tracker if present
                        if hasattr(self, "register_fs_succ"):
                            try:
                                self.register_fs_succ(True)
                            except Exception as e:
                                logger.error(f"[FS] register_fs_succ(True) failed: {e}")
                        return True
                    else:
                        logger.error(f"Writtend and NOT readable : {filepath}")
                else:
                    logger.error(f"Not able to write {filepath}")

            logger.error(f"Writing failed after retries for {filepath}")
            if hasattr(self, "register_fs_succ"):
                try:
                    self.register_fs_succ(False)
                except Exception as e:
                    logger.error(f"[FS] register_fs_succ(False) failed: {e}")
            return False
        except Exception as e:
            logger.error(f"Some unknown Error saving file {filepath}: {e}")
            if hasattr(self, "register_fs_succ"):
                try:
                    self.register_fs_succ(False)
                except Exception as e2:
                    logger.error(
                        f"[FS] register_fs_succ(False) failed in exception path: {e2}"
                    )
            return False


class DbStore(StoreUtils):
    """
    Storage helper for this project, matching how `nv2/main.py` uses:
    - image directories on SD card
    - in-transit image/file buffer
    - in-memory image buffers
    """

    # Recompile buffer (same size as in nv2/main.py)
    DATA_BUFFER_SIZE = 120 * 1024  # 120KB

    # Image list buffer defaults (aligned with nv2/main.py, extended header)
    IMG_LIST_CAPACITY = 20  # Number of image slots
    IMG_MAX_SIZE = 100  # 100KB per image
    IMG_LIST_SLOT_SIZE = IMG_MAX_SIZE * 1024

    # Header layout inside each image slot:
    # [0]         : 1 byte  -> occupied flag (0 = empty, 1 = used)
    # [1:7]       : 6 bytes -> epoch_ms (48-bit big-endian, enough for 13-digit ms)
    # [7]         : 1 byte  -> creator_id (0-255)
    # [8]         : 1 byte  -> retry count (0-10)
    # [9:12]      : 3 bytes -> image size (big-endian)
    IMG_FILLED_FLAG_OFFSET = 0
    IMG_LIST_EPOCH_OFFSET = 1
    IMG_LIST_EPOCH_BYTES = 6
    IMG_LIST_CREATOR_OFFSET = 7
    IMG_LIST_CREATOR_BYTES = 1
    IMG_LIST_RETRY_OFFSET = 8
    IMG_LIST_RETRY_BYTES = 1
    IMG_LIST_SIZE_OFFSET = 9
    IMG_LIST_HEADER_SIZE = 12
    IMG_LIST_DATA_CAPACITY = IMG_LIST_SLOT_SIZE - IMG_LIST_HEADER_SIZE

    def __init__(self, process_id_str, my_addr) -> None:
        """
        Complete all 5 tasks described in the comments:
        1. Optional image directory on SD card
        2. In-transit recompile buffer
        3. Image ring buffer
        """
        # Initialise base StoreUtils (including FS counters)
        super().__init__()

        if not process_id_str:
            raise ValueError("process_id_str must be a non-empty string")

        self.process_id_str: str = process_id_str
        self.my_addr = my_addr

        self.fs_root = "/sdcard"
        self.sdcard_available = (
            self._is_sdcard_readable() and self._is_sdcard_writable()
        )

        self.process_dir = None
        self.image_dir = None
        self.logs_dir = None

        if self.sdcard_available:
            logger.info(f"[DB] ⛃⛃⛃⛁⛁⛁ SD CARD AVAILABLE & USABLE, using fs_root={self.fs_root}")
            self.process_dir = f"{self.fs_root}/{self.process_id_str}"
            self.image_dir = f"{self.process_dir}/all_images"
            self.logs_dir = f"{self.process_dir}/logs"
            self._create_dir_if_not_exists(self.process_dir)
            self._create_dir_if_not_exists(self.image_dir)
            self._create_dir_if_not_exists(self.logs_dir)
        else:
            logger.warning("[DB] ⛃⛃⛃⛁⛁⛁ SD CARD NOT AVAILABLE/USABLE, operating in memory-only mode")

        # ---- TASK 2: in-transit recompile buffer ----
        # self.image_recompile_buffer = None
        # self._init_file_recompile_buffer()

        # ---- TASK 3: image ring buffer ----
        self.image_list_buffer = None
        self._last_sent_img_creator = None
        self._init_image_list_buffer()

        # counters
        self.image_queued_count = 0
        self.img_sent_count = 0
        self.img_dropped_count = 0
        self.img_failed_count = 0

        self.LIMIT_PER_DEVICE = 6
        self.img_retry_count_to_fail = 20 # TODO need to increased

    # ------------------------------------------------------------------
    # Internal helpers (filesystem)
    # ------------------------------------------------------------------
    def _is_sdcard_readable(self) -> bool:
        """Best-effort check that SD card root is listable."""
        for attempt in range(2):
            try:
                utime.sleep_ms(300 * (attempt + 1))
                os.listdir(self.fs_root)
                logger.debug(f"[DB] SD card readable (attempt {attempt + 1})")
                return True
            except OSError:
                logger.warning(f"[DB] SD card not ready (attempt {attempt + 1}/5)")
        return False

    def _is_sdcard_writable(self) -> bool:
        """Best-effort check that we can create a tiny file."""
        test_file = f"{self.fs_root}/.dbstore_test"
        try:
            with open(test_file, "wb") as f:
                f.write(b"ok")
            os.remove(test_file)

            test_file = "/sdcard/processid"
            with open(test_file, "wb") as f:
                self.process_id_str.encode()
            return True
        except OSError:
            logger.warning("[DB] SD card not writable")
            return False

    def _create_dir_if_not_exists(self, dir_path: str) -> None:
        """Minimal variant of `create_dir_if_not_exists` in nv2/main.py."""
        try:
            parts = [p for p in dir_path.split("/") if p]
            if len(parts) < 2:
                logger.warning(f"[DB] Invalid directory path (no parent): {dir_path}")
                return
            parent = "/" + "/".join(parts[:-1])
            dir_name = parts[-1]

            if dir_name not in os.listdir(parent):
                os.mkdir(dir_path)
                logger.info(f"[DB] Created {dir_path}")
            else:
                try:
                    os.listdir(dir_path)
                except OSError:
                    logger.warning(
                        f"[DB] {dir_path} exists but is not a directory; recreating"
                    )
                    try:
                        os.remove(dir_path)
                        os.mkdir(dir_path)
                        logger.info(f"[DB] Recreated directory {dir_path}")
                    except OSError as e:
                        logger.error(
                            f"[DB] Failed to recreate directory {dir_path}: {e}"
                        )
        except Exception as e:
            logger.error(f"[DB] Error ensuring directory {dir_path}: {e}")

    # ------------------------------------------------------------------
    # Internal helpers (buffers)
    # ------------------------------------------------------------------
    # def _init_file_recompile_buffer(self) -> None:
    #     """Allocate the in-transit image/file recompilation buffer."""
    #     gc.collect()
    #     try:
    #         self.image_recompile_buffer = bytearray(self.DATA_BUFFER_SIZE)
    #         logger.info(
    #             f"[DB] Pre-allocated recompile buffer: "
    #             f"{len(self.image_recompile_buffer) // 1024}KB"
    #         )
    #     except MemoryError as e:
    #         logger.error(f"[DB] Failed to allocate recompile buffer: {e}")
    #         self.image_recompile_buffer = None
    #     except Exception as e:
    #         logger.error(f"[DB] Error allocating recompile buffer: {e}")
    #         self.image_recompile_buffer = None

    def _init_image_list_buffer(self) -> None:
        """Allocate the in-memory circular ring buffer for images."""
        gc.collect()
        try:
            self.image_list_buffer = [
                bytearray(self.IMG_LIST_SLOT_SIZE)
                for _ in range(self.IMG_LIST_CAPACITY)
            ]
            self.image_queued_count = 0
            logger.info(
                f"[DB] Pre-allocated image ring: "
                f"{self.IMG_LIST_CAPACITY} x {self.IMG_LIST_SLOT_SIZE // 1024}KB"
            )
        except MemoryError as e:
            logger.error(f"[DB] Failed to allocate image ring buffer: {e}")
            self.image_list_buffer = None
            self.image_queued_count = 0
        except Exception as e:
            logger.error(f"[DB] Error allocating image ring buffer: {e}")
            self.image_list_buffer = None
            self.image_queued_count = 0

    # ------------------------------------------------------------------
    # Public API: setters / getters / round-robin access
    # ------------------------------------------------------------------
    # Image ring operations ---------------------------------------------

    def storage_available(self, creator):
        # For self.my_addr, reserve LIMIT_PER_DEVICE image slots (e.g. if 2 stored,
        # assume more reserved for self.my_addr only).
        # For creator == self.my_addr, always True (old images drop, new ones store).
        # For others: True only if stored count < LIMIT_PER_DEVICE and queue has
        # space beyond the reservation for self.my_addr.
        try:
            if creator == self.my_addr:
                return True

            if self.image_list_buffer is None:
                return False

            creator_img_count = 0
            my_img_count = 0
            total_filled = 0

            for idx in range(self.IMG_LIST_CAPACITY):
                slot = self.image_list_buffer[idx]
                if slot[self.IMG_FILLED_FLAG_OFFSET] == 0:
                    continue
                total_filled += 1
                c_id = slot[self.IMG_LIST_CREATOR_OFFSET]
                if c_id == creator:
                    creator_img_count += 1
                if c_id == self.my_addr:
                    my_img_count += 1

            # Reserve free slots for self.my_addr up to LIMIT_PER_DEVICE.
            reserved_for_me = max(0, self.LIMIT_PER_DEVICE - my_img_count)
            max_usable_for_others = self.IMG_LIST_CAPACITY - total_filled - reserved_for_me
            if creator_img_count >= self.LIMIT_PER_DEVICE:
                logger.info(
                    f"[DB] creator={creator} has {creator_img_count} images, "
                    f"exceeding limit of {self.LIMIT_PER_DEVICE}, returning False"
                )
                return False
            if max_usable_for_others <= 0:
                logger.info(
                    "[DB] no space left in queue for other devices, returning False"
                )
                return False
            return True
        except Exception as e:
            logger.error(f"[DB] storage_available failed for creator={creator}: {e}")
            return False

    def store_image_raw(self, epoch_ms: int, creator_id: int, img_snapshot) -> bool:
        """
        Store raw image bytes into the image list buffer.
        """
        try:
            if self.sdcard_available:
                raw_path = f"{self.image_dir}/{self.process_id_str}_{creator_id}_{epoch_ms}_raw.jpg"
                logger.info(
                    f"Saving raw image to {raw_path} : imbytesize = {len(img_snapshot.bytearray())}"
                )
                img_snapshot.save(raw_path)
                logger.info(
                    f"Saved raw image: {raw_path}: raw size = {len(img_snapshot.bytearray())} bytes"
                )
            else:
                logger.warning("[DB] SD card not ready, skipping raw image save...")
        except Exception as e:
            logger.error(f"Failed to save raw image: {e}")

    def store_image(
        self,
        epoch_ms: int,
        creator_id: int,
        retry: int,
        img_bytes: bytes,
        save_file: bool = True,
    ):
        """
        Store raw image bytes into the image list buffer.
        Header layout per slot:
          - 1 byte  : occupied flag (0 = empty, 1 = used)
          - 6 bytes : epoch_ms (48-bit big-endian)
          - 1 byte  : creator_id (0-255)
          - 3 bytes : image size (big-endian)

        Return:
            (store_succ: bool, error: str)
        """
        if retry >= self.img_retry_count_to_fail:
            self.update_img_failed_count(1)
            return False, "retry_limit_reached"

        if self.image_list_buffer is None:
            logger.error("[DB] image_list_buffer not INITIALIZED")
            return False, "image_buffer_not_initialized"

        if creator_id < 0 or creator_id > 255:
            logger.error(f"[DB] creator_id out of range (0-255): {creator_id}")
            return False, "creator_out_of_range"

        size = len(img_bytes)
        if size > self.IMG_LIST_DATA_CAPACITY:
            logger.error(
                f"[DB] Image size {size} exceeds image data capacity "
                f"{self.IMG_LIST_DATA_CAPACITY}, returning..."
            )
            return False, "image_too_large"

        # ------------------------------------------------------------------
        # Slot selection strategy
        # ------------------------------------------------------------------
        # 1) If there is free space (image_list_count < capacity), find first empty slot.
        # 2) Else (buffer full):
        #    - If image is from this device (creator_id == self.my_addr),
        #      replace the oldest image of this creator (min epoch_ms).
        #    - If image is from other device, reject (return False).

        slot_idx = None
        creator_img_count = 0
        my_img_count = 0
        total_filled = 0

        for idx in range(self.IMG_LIST_CAPACITY):
            slot = self.image_list_buffer[idx]
            if slot[self.IMG_FILLED_FLAG_OFFSET] == 0:
                continue
            total_filled += 1
            c_id = slot[self.IMG_LIST_CREATOR_OFFSET]
            if c_id == creator_id:
                creator_img_count += 1
            if c_id == self.my_addr:
                my_img_count += 1

        if (
            creator_img_count < self.LIMIT_PER_DEVICE
            and self.image_queued_count < self.IMG_LIST_CAPACITY
        ):
            # Find first empty slot (flag == 0)
            for idx in range(self.IMG_LIST_CAPACITY):
                slot = self.image_list_buffer[idx]
                if slot[self.IMG_FILLED_FLAG_OFFSET] == 0:
                    slot_idx = idx
                    break
            if slot_idx is None:
                logger.error(
                    "[DB] Error in the code, couldn't find empty space saving image"
                )
                return False, "empty_slot_not_found:unknown_error"
        else:
            if creator_id != self.my_addr:
                logger.warning(
                    f"[DB] Buffer full and creator_id {creator_id} != my_addr {self.my_addr}; "
                    "rejecting new image"
                )
                return False, "buffer_full_non_creator"
            else:  # find the index to replace the old image
                oldest_epoch = None
                oldest_idx = None
                for idx in range(self.IMG_LIST_CAPACITY):
                    slot = self.image_list_buffer[idx]
                    if slot[self.IMG_FILLED_FLAG_OFFSET] == 0:
                        continue

                    c_id = slot[self.IMG_LIST_CREATOR_OFFSET]
                    if c_id != self.my_addr:
                        continue

                    epoch_bytes = slot[
                        self.IMG_LIST_EPOCH_OFFSET : self.IMG_LIST_EPOCH_OFFSET
                        + self.IMG_LIST_EPOCH_BYTES
                    ]
                    epoch_val = int.from_bytes(epoch_bytes, "big")

                    if oldest_epoch is None or epoch_val < oldest_epoch:
                        oldest_epoch = epoch_val
                        oldest_idx = idx

                if oldest_idx is None:
                    logger.warning(
                        "[DB] Buffer full but no existing image for this creator; rejecting new image"
                    )
                    return False, "buffer_full_no_replace_candidate"

                slot_idx = oldest_idx
                self.update_img_queued_count(-1)
                self.update_img_dropped_count(1)

        # ------------------------------------------------------------------
        # Write header + data into chosen slot
        # ------------------------------------------------------------------
        slot = self.image_list_buffer[slot_idx]

        # Occupied flag
        slot[self.IMG_FILLED_FLAG_OFFSET] = 1

        # Epoch (48-bit, big-endian)
        epoch_clamped = epoch_ms & ((1 << (8 * self.IMG_LIST_EPOCH_BYTES)) - 1)
        epoch_bytes = epoch_clamped.to_bytes(self.IMG_LIST_EPOCH_BYTES, "big")
        start = self.IMG_LIST_EPOCH_OFFSET
        slot[start : start + self.IMG_LIST_EPOCH_BYTES] = epoch_bytes

        # Creator id (1 byte)
        slot[self.IMG_LIST_CREATOR_OFFSET] = creator_id & 0xFF

        # Retry count (1 byte, clamped 0-10)
        retry_clamped = max(0, min(10, retry))
        slot[self.IMG_LIST_RETRY_OFFSET] = retry_clamped & 0xFF

        # Size (3 bytes, big-endian)
        size_bytes = size.to_bytes(3, "big")
        start = self.IMG_LIST_SIZE_OFFSET
        slot[start : start + 3] = size_bytes

        # Image data
        data_start = self.IMG_LIST_HEADER_SIZE
        slot[data_start : data_start + size] = img_bytes[:size]

        # Housekeeping counters
        self.image_queued_count = min(self.image_queued_count + 1, self.IMG_LIST_CAPACITY)
        logger.info(f"Image added for db_queue, new length: {self.image_queued_count}, {self.process_id_str}_{creator_id}_{epoch_ms}.enc")

        # Optionally persist the encrypted image to filesystem (fire-and-forget).
        if self.sdcard_available and save_file:
            enc_filepath = (
                f"{self.image_dir}/{self.process_id_str}_{creator_id}_{epoch_ms}.enc"
            )
            logger.info(f"[DB] Scheduling save of encrypted image to {enc_filepath}")
            try:
                # Do not block caller; schedule async write in background.
                asyncio.create_task(self.save_file(img_bytes, enc_filepath))
            except Exception as e:
                logger.error(
                    f"[DB] Failed to schedule encrypted image save to {enc_filepath}: {e}"
                )

        return True, ""

    def get_next_image_to_send(self):
        """
        Select the next image to send in round-robin fashion across creators.

        Returns:
            tuple (img_bytes, epoch_ms, creator_id) or None if no image is available.

        Side effects:
            - Removes the chosen image from the buffer (marks slot as empty).
            - Updates internal round-robin state.
        """
        if self.image_list_buffer is None or self.image_queued_count == 0:
            return None, None, None, None, None

        # Collect distinct creators from occupied slots
        creators = []
        for idx in range(self.IMG_LIST_CAPACITY):
            slot = self.image_list_buffer[idx]
            if slot[self.IMG_FILLED_FLAG_OFFSET] == 0:
                continue
            c_id = slot[self.IMG_LIST_CREATOR_OFFSET]
            if c_id not in creators:
                creators.append(c_id)

        if not creators:
            return None, None, None, None, None

        creators.sort()

        # Round-robin across creators
        if (
            self._last_sent_img_creator is not None
            and self._last_sent_img_creator in creators
        ):
            last_idx = creators.index(self._last_sent_img_creator)
            next_idx = (last_idx + 1) % len(creators)
            chosen_creator = creators[next_idx]
        else:
            chosen_creator = creators[0]

        # Among slots for chosen_creator, pick oldest (min epoch_ms)
        chosen_slot_idx = None
        chosen_epoch = None

        for idx in range(self.IMG_LIST_CAPACITY):
            slot = self.image_list_buffer[idx]
            if slot[self.IMG_FILLED_FLAG_OFFSET] == 0:
                continue

            c_id = slot[self.IMG_LIST_CREATOR_OFFSET]
            if c_id != chosen_creator:
                continue

            epoch_bytes = slot[
                self.IMG_LIST_EPOCH_OFFSET : self.IMG_LIST_EPOCH_OFFSET
                + self.IMG_LIST_EPOCH_BYTES
            ]
            epoch_val = int.from_bytes(epoch_bytes, "big")

            if chosen_slot_idx is None or epoch_val < chosen_epoch:
                chosen_slot_idx = idx
                chosen_epoch = epoch_val

        if chosen_slot_idx is None:
            # Fallback: pick any oldest image regardless of creator
            for idx in range(self.IMG_LIST_CAPACITY):
                slot = self.image_list_buffer[idx]
                if slot[self.IMG_FILLED_FLAG_OFFSET] == 0:
                    continue
                epoch_bytes = slot[
                    self.IMG_LIST_EPOCH_OFFSET : self.IMG_LIST_EPOCH_OFFSET
                    + self.IMG_LIST_EPOCH_BYTES
                ]
                epoch_val = int.from_bytes(epoch_bytes, "big")
                if chosen_slot_idx is None or epoch_val < chosen_epoch:
                    chosen_slot_idx = idx
                    chosen_epoch = epoch_val

        if chosen_slot_idx is None:
            return None, None, None, None, None

        slot = self.image_list_buffer[chosen_slot_idx]

        # Decode header
        epoch_bytes = slot[
            self.IMG_LIST_EPOCH_OFFSET : self.IMG_LIST_EPOCH_OFFSET
            + self.IMG_LIST_EPOCH_BYTES
        ]
        epoch_ms = int.from_bytes(epoch_bytes, "big")

        creator_id = slot[self.IMG_LIST_CREATOR_OFFSET]
        retry = slot[self.IMG_LIST_RETRY_OFFSET]

        size_bytes = slot[self.IMG_LIST_SIZE_OFFSET : self.IMG_LIST_SIZE_OFFSET + 3]
        size = int.from_bytes(size_bytes, "big")

        if size <= 0 or size > self.IMG_LIST_DATA_CAPACITY:
            # Corrupt entry; clear it and skip
            logger.warning(f"[DB] Corrupt image slot at {chosen_slot_idx}, clearing")
            slot[self.IMG_FILLED_FLAG_OFFSET] = 0
            self.image_queued_count = max(0, self.image_queued_count - 1)
            return None, None, None, None, None

        data_start = self.IMG_LIST_HEADER_SIZE
        img_bytes = bytes(slot[data_start : data_start + size])

        # Mark slot as empty and update counters
        slot[self.IMG_FILLED_FLAG_OFFSET] = 0
        self.image_queued_count = max(0, self.image_queued_count - 1)

        # Update round-robin state
        self._last_sent_img_creator = creator_id
        img_md5 = ubinascii.hexlify(hashlib.md5(img_bytes).digest()).decode()
        return epoch_ms, creator_id, retry, img_bytes, img_md5

    # Listing Getters and Setters for images

    def update_img_queued_count(self, count):
        self.image_queued_count = self.image_queued_count + count

    def get_img_queued_count(self):
        return self.image_queued_count

    def get_img_queued_list(self):  # TODO akash to implement it
        return []

    def get_img_sent_count(self):
        return self.img_sent_count

    def update_img_sent_count(self, count):
        self.img_sent_count = self.img_sent_count + count

    def get_img_dropped_count(self):
        return self.img_dropped_count

    def update_img_dropped_count(self, count):
        self.img_dropped_count = self.img_dropped_count + count

    def get_img_failed_count(self):
        return self.img_failed_count

    def update_img_failed_count(self, count):
        self.img_failed_count = self.img_failed_count + count

    def get_fs_succ_count(self):
        return self.file_system_success

    def get_fs_err_count(self):
        return self.file_system_err

    def get_fs_consecutive_err_count(self):  # NOT IN USE
        return self.file_system_consecutive_err

    def db_store(self):
        # TODO akash, return th list of dict in form of
        return []

    def clear_image_list(self):
        # TODO akash, make it
        return True
