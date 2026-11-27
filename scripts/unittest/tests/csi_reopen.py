def unittest(data_path, temp_path):
    import csi
    import gc

    count = [0]

    def on_frame(c):
        count[0] += 1

    # Each sensor has one object.
    if csi.CSI() is not csi.CSI():
        return False

    # Dropping every reference to the object must leave its callback working,
    # since the callback is called with it.
    csi0 = csi.CSI()
    csi0.reset()
    csi0.pixformat(csi.GRAYSCALE)
    csi0.framesize(csi.QQVGA)
    csi0.frame_callback(on_frame)
    csi0 = None
    gc.collect()
    for i in range(5):
        csi.CSI().snapshot()
    csi.CSI().frame_callback(None)

    # Ports without real sensors, like QEMU, never call the frame callback.
    if 0 < count[0] < 5:
        return False

    # Find an auxiliary sensor, which has its own framebuffer.
    cid = None
    for dev in csi.devices():
        if "Auxiliary           : true" in str(csi.CSI(cid=dev)):
            cid = dev
            break

    if cid is None:
        return True

    # Dropping a reference must leave the framebuffer to the others.
    csi1 = csi.CSI(cid=cid)
    csi1.reset(hard=False)
    # Prints the framebuffer, which reads as garbage once it's freed.
    state = str(csi1)
    csi2 = csi.CSI(cid=cid)
    del csi1
    gc.collect()
    return str(csi2) == state
