def unittest(data_path, temp_path):
    import omv

    # Emulated networking is only available on the qemu MPS2_AN500 board.
    if "MPS2" not in omv.arch():
        return "skip"

    import time
    import network
    import requests
    from mqtt import MQTTClient

    lan = network.LAN()
    lan.active(True)  # DHCP; active() returns once the lease is bound

    # Public echo services are occasionally slow or return 5xx, so retry each
    # request a few times before giving up.  Only transport errors and non-200
    # responses are retried; a 200 with the wrong content is a real failure.
    def http_request(method, url, **kw):
        for _ in range(3):
            try:
                r = requests.request(method, url, timeout=10, **kw)
                if r.status_code == 200:
                    return r
            except Exception:
                pass
            time.sleep_ms(1000)
        return None

    # HTTP GET via requests: httpbin echoes the request back as JSON.
    r = http_request("GET", "http://httpbin.org/get")
    if r is None:
        return False
    if r.json()["url"] != "http://httpbin.org/get":
        return False

    # HTTP POST via requests: httpbin echoes the posted JSON back.
    payload = {"board": "openmv", "value": 42}
    r = http_request("POST", "http://httpbin.org/post", json=payload)
    if r is None:
        return False
    if r.json()["json"] != payload:
        return False

    # MQTT round-trip: publish to a topic we are subscribed to and wait for the
    # broker to deliver it back.  Public brokers are often overloaded, so try a
    # few in turn with a bounded timeout.  The topic is shared with anyone else
    # on the broker (other CI runs, retained messages), so ignore messages that
    # are not ours rather than expecting ours to arrive first.
    servers = (
        "test.mosquitto.org",
        "broker.hivemq.com",
        "broker.emqx.io",
        "mqtt.eclipseprojects.io",
        "public.mqtthq.com",
    )
    nonce = time.ticks_us()  # Varies with network latency up to this point.
    topic = b"openmv/unittest/%08x" % (nonce & 0xFFFFFFFF)
    client_id = b"openmv-%08x" % (nonce & 0xFFFFFFFF)
    message = b"openmv %08x" % (nonce & 0xFFFFFFFF)

    received = []

    def on_message(topic, msg):
        received.append(msg)

    for server in servers:
        del received[:]
        client = MQTTClient(client_id, server, port=1883, keepalive=30)
        client.set_callback(on_message)
        try:
            client.connect(timeout=5)
            try:
                client.subscribe(topic)
                client.publish(topic, message)
                deadline = time.ticks_add(time.ticks_ms(), 10000)
                while message not in received:
                    if time.ticks_diff(deadline, time.ticks_ms()) <= 0:
                        raise OSError("timed out waiting for echo")
                    client.wait_msg()  # Raises OSError on socket timeout.
            finally:
                client.disconnect()
        except Exception:
            # Any broker failure (refused, reset, closed mid-packet) moves on
            # to the next broker.
            if client.sock is not None:
                client.sock.close()
            continue
        break
    else:
        # No broker was reachable; treat it as an environment issue, not a
        # regression, so a broker outage does not fail the build.
        return "skip"

    return True
