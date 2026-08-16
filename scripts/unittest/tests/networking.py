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

    # HTTP GET via requests: httpbin echoes the request back as JSON.
    r = requests.get("http://httpbin.org/get")
    if r.status_code != 200:
        return False
    if r.json()["url"] != "http://httpbin.org/get":
        return False

    # HTTP POST via requests: httpbin echoes the posted JSON back.
    payload = {"board": "openmv", "value": 42}
    r = requests.post("http://httpbin.org/post", json=payload)
    if r.status_code != 200:
        return False
    if r.json()["json"] != payload:
        return False

    # MQTT round-trip: publish to a topic we are subscribed to and wait for the
    # broker to deliver it back.  Public brokers are often overloaded, so try a
    # few in turn with a bounded timeout.
    servers = (
        "test.mosquitto.org",
        "broker.hivemq.com",
        "broker.emqx.io",
        "mqtt.eclipseprojects.io",
        "public.mqtthq.com",
    )
    topic = b"openmv/unittest"
    client_id = b"openmv-%d" % (time.ticks_ms() & 0xFFFF)
    message = b"openmv %d" % (time.ticks_ms() & 0xFFFF)

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
                client.wait_msg()  # our own publish, echoed back
            finally:
                client.disconnect()
        except OSError:
            continue
        break
    else:
        # No broker was reachable; treat it as an environment issue, not a
        # regression, so a broker outage does not fail the build.
        return "skip"

    if not received or received[0] != message:
        return False

    return True
