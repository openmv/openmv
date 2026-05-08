import os
import sys
import logger


def create_dir_if_not_exists(dir_path):
    try:
        parts = [p for p in dir_path.split('/') if p]
        if len(parts) < 2:
            logger.warning(f"[FS] Invalid directory path (no parent): {dir_path}")
            return
        parent = '/' + '/'.join(parts[:-1])
        dir_name = parts[-1]
        if dir_name not in os.listdir(parent):
            os.mkdir(dir_path)
            logger.info(f"[FS] Created {dir_path}")
        else:
            try:
                os.listdir(dir_path)  # for valid diretory
                logger.info(f"[FS] {dir_path} directory already exists")
            except OSError:
                logger.warning(
                    f"dir:{dir_path} exists but not a directory, trying deleting and recreating..."
                )
                try:
                    os.remove(dir_path)
                    os.mkdir(dir_path)
                    logger.info(f"info - Removed file {dir_path} and created directory")
                except OSError as e:
                    logger.error(
                        f"Failed to remove file {dir_path} and create directory: {e}, exiting..."
                    )
                    sys.exit()
            except Exception as e:
                logger.warning(f"[FS] Unexpected error accessing {dir_path}: {e}")
                try:
                    os.remove(dir_path)
                    os.mkdir(dir_path)
                    logger.info(f"info - Removed file {dir_path} and created directory")
                except OSError as e:
                    logger.error(
                        f"Failed to remove file {dir_path} and create directory: {e}, exiting..."
                    )
                    sys.exit()

    except OSError as e:
        logger.error(f"[FS] Failed to create/access {dir_path}: {e}")
