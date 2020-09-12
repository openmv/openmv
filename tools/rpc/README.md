# OpenMV Remote Python Call Library

The `rpc` module on the OpenMV Cam allows you to connect your OpenMV Cam to another microcontroller or computer and execute remote python (or procedure) calls on your OpenMV Cam. The `rpc` module also allows for the reverse too if you want your OpenMV Cam to be able to execute remote procedure (or python) calls on another microcontroller or computer.

For computer control the [rpc](rpc.py) python module in this directory implements the OpenMV `rpc` protocol for control of an OpenMV Cam over USB VCP (i.e. a USB serial port) or over Ethernet/WiFi (i.e. over sockets).

# Library Dependencies

The OpenMV Cam `rpc` library on the computer only depends on [pyserial](https://pythonhosted.org/pyserial/). All other modules used by it come installed with python. To get `pyserial` just do:

    pip install pyserial

However, the examples depend on [pygame](https://www.pygame.org/news) so you need to install pygame too:

    pip install pygame

Because the interface library is implemented in pure python with no external dependencies it works on Windows, Mac, and Linux.

## UART Support

pySerial provides support for pure USB virtual COM ports, USB to RS232/RS422/RS485/TTL COM ports, and standard RS232/RS422/RS485/TTL COM ports. Please use the `rpc_usb_vcp_master` and `rpc_usb_vcp_slave` for pure USB virtual COM port communication and `rpc_uart_master` and `rpc_uart_slave` for USB to RS232/RS422/RS485/TTL COM ports and standard RS232/RS422/RS485/TTL COM ports.

### USB to Serial Converter Issues

Pure hardware RS232/RS422/RS485/TTL COM ports should work using the `rpc_uart_master` and `rpc_uart_slave` interfaces without issues. However, FTDI like USB to serial converter chips may add unexpected latency to communication. In particular, FTDI chips have a latency timer which is used to buffer bytes for transmission over USB to improve bandwidth... but, that also increases the worse case latency of a single byte being sent over USB to **16 ms** by default. You need to reduce the latency timer to **1 ms** as detailed below or the interface library will fail to work:

* https://stackoverflow.com/questions/54230836/inconsistent-delay-in-reading-serial-data-in-python-3-7-2-using-pyserial
* https://projectgus.com/2011/10/notes-on-ftdi-latency-with-arduino/

## CAN Support

You may use the RPC Interface Library over CAN using [Kvaser](https://www.kvaser.com/) hardware on Windows and Linux (Kvaser does not support Mac). You need to install the following:

* For Windows
  * [Kvaser Drivers for Windows](https://www.kvaser.com/download/?utm_source=software&utm_ean=7330130980013&utm_status=latest)
  * [Kvaser CANlib SDK](https://www.kvaser.com/download/?utm_source=software&utm_ean=7330130980150&utm_status=latest)
  * [Python module](https://www.kvaser.com/download/?utm_source=software&utm_ean=7330130981911&utm_status=latest)
* For Linux
  * [Kvaser Linux Drivers and SDK](https://www.kvaser.com/download/?utm_source=software&utm_ean=7330130980754&utm_status=latest)
  * [Python module](https://www.kvaser.com/download/?utm_source=software&utm_ean=7330130981911&utm_status=latest)

# How to use the Library

Please checkout the following scripts for how to control your OpenMV Cam from the comptuer:

* [Slow but Synchronus JPG Image Transfer](rpc_image_transfer_jpg_as_the_controller_device.py)
* [Fast JPG Image Streaming](rpc_image_transfer_jpg_streaming_as_the_controller_device.py)
* [Face Detection, April Tag Detection, Color Tracking, and more](rpc_popular_features_as_the_controller_device_example.py)

You will need to edit the example code above to choose which interface you want to use (USB versus Ethernet/WiFi) and to play with the settings the scripts use. When you run the scripts make sure to save them first after editing them and then run them with `python -u <script_name>` to make sure that script output to stdio is not buffered.

In general, for the controller device to use the `rpc` library you will create an interface object using the `rpc` library. For example:

    interface = rpc.rpc_usb_vcp_master("COM3")

This create a USB VCP interface to talk to your OpenMV Cam over COM3. For Mac and Linux you would pass some type of `/dev/...` device instead.

Once the interface is created you just need to do:

    memory_view_object_result = interface.call("remote_function_or_method_name", bytes_object_argument)

And the `rpc` library will try to execute that `"remote_function_or_method_name"` on your OpenMV Cam. The remote function or method will receive the `bytes_object_argument` which can be up to 2^32-1 bytes in size. Once the remote method finishes executing it will return a `memory_view_object_result` which can also be up to 2^32-1 bytes in size. Because the argument and response are both generic byte containers you can pass anything through the `rpc` library and receive any type of response. A simple way to pass arguments is to use `struct.pack()` to create the argument and `struct.unpack()` to receieve the argument on the OpenMV Cam side. For the response, the OpenMV Cam may send a string object or json string as the result which the computer can then interpret. Most objects or lists returned from method calls on the OpenMV Cam generate valid json strings when you call `str()` on them.

As for errors, if you try to execute a non-existant function or method name on the OpenMV Cam the `call` method will return an empty `bytes()` object. If the `rpc` library failed to communicate with the OpenMV Cam the `rpc` library will return `None`.

To keep things simple the `rpc` library doesn't maintain a connection between the master and slave devices. The `call` method encapsulates trying to connect to the OpenMV Cam, starting execution of the remote function or method, and getting the result.

Now, on the OpenMV Cam side of things you have to create an `rpc` interface to communicate with the computer. This looks like:

    interface = rpc.rpc_usb_vcp_slave()

This will create the interface layer on the OpenMV Cam (this needs to be done on a script running on the OpenMV Cam - see the [Remote Control](../../scripts/examples/34-Remote-Control) example scripts).

Once you create the slave interface you then need to register call backs that the master can call with the interface object.

    def remote_function_or_method_name(memoryview_object_argument):
        <lots of code>
        return bytes_object_result

    interface.register_callback(remote_function_or_method_name)

You may register as many callbacks as you like on the OpenMV Cam that the computer can call. Finally, once you are done registering callbacks you just need to execute:

    interface.loop()

On the OpenMV Cam to start the `rpc` library up and begin listening for the computer. Note that the `loop()` method does not return. Also, to make your OpenMV Cam more robust against errors you may want to wrap the `loop()` with `try:` and `except:` for whatever exceptions might be thrown by your callback methods. The `rpc` library will not generate any exceptions itself. Note: passing large data structures around (like jpeg images) can potentially exhaust the heap on the OpenMV Cam and generate `MemoryError` exceptions.

And that is it! The `rpc` library is designed to be simple to use. It was designed to allow remote control of the OpenMV Cam by a computer or microcontroller so there are also interfaces for control of your OpenMV Cam over CAN, I2C, SPI, and UART.

# API

Please see the example scripts above for starting code on how to use the `rpc` library. The below API documents the public interface of the library.

## class rpc():

The `rpc` base class is reimplemented by the `rpc_master` and `rpc_slave` classes to create the master and slave interfaces. It is a pure virtual class and not meant to be used directly.

#### get_bytes(buff, timeout_ms):

This method is meant to be reimplemented by specific interface classes of `rpc_master` and `rpc_slave`. It should fill the `buff` argument which is either a `bytearray` or `memoryview` object of bytes from the interface equal to the length of the `buff` object in `timeout_ms` milliseconds. On timeout this method should return `None`. Note that for master and slave synchronization this method should try to always complete in at least `timeout_ms` milliseconds and not faster as the `rpc_master` and `rpc_slave` objects will automatically increase the `timeout_ms` to synchronize.

#### put_bytes(data, timeout_ms):

This method is meant to be reimplemented by specific interface classes of `rpc_master` and `rpc_slave`. It should send `data` bytes on the interface within `timeout_ms` milliseconds. If it completes faster than the timeout that is okay. No return value is expected.

#### stream_reader(call_back, queue_depth=1, read_timeout_ms=5000):

This method is meant to be called directly. After synchronization of the master and slave on return of a callback `stream_reader` may be called to receive data as fast as possible from the master or slave device. `call_back` will be called repeatedly with a `bytes_or_memory_view` argument that was sent by the `stream_writer`. `call_back` is not expected to return anything. `queue_depth` defines how many frames of data the `stream_writer` may generate before slowing down and waiting on the `stream_reader`. Higher `queue_depth` values lead to higher performance (up to a point) but require the `stream_reader` to be able to handle outstanding packets in its interface layer. Note that computers typically do not buffer much more than 4KB of data in device driver buffers. If you make the `queue_depth` larger than 1 then `call_back` should return very quickly and not block. Otherwise, you should implement a multi-thread architecture to process the received data so that `stream_reader` is always executing and moving data out of device driver buffers into larger user memory buffers. Finally, `read_timeout_ms` defines how many milliseconds to wait to receive the `bytes_or_memory_view` payload per `call_back`.

On any errors `stream_reader` will return. The master and slave devices can try to setup the stream again afterwards to continue as demonstrated in [Fast JPG Image Streaming](rpc_image_transfer_jpg_streaming_as_the_controller_device.py).

If you need to cancel the `stream_reader` just raise an exception in the `call_back` and catch it. The remote side will automatically timeout.

#### stream_writer(call_back, write_timeout_ms=5000):

This method is meant to be called directly. After synchronization of the master and slave on return of a callback `stream_writer` may be called to send data as fast as possible from the master or slave device. `call_back` will be called repeatedly and should return a `bytes_or_memory_view` object that will be sent to the `stream_reader`. `call_back` should not take any arguments. Finally, `write_timeout_ms` defines how many milliseconds to wait to send the `bytes_or_memory_view` object returned by `call_back`.

On any errors `stream_writer` will return. The master and slave devices can try to setup the stream again afterwards to continue as demonstrated in [Fast JPG Image Streaming](rpc_image_transfer_jpg_streaming_as_the_controller_device.py).

If you need to cancel the `stream_writer` just raise an exception in the `call_back` and catch it. The remote side will automatically timeout.

## class rpc_master():

The `rpc_master` is a pure virtual class and not meant to be used directly. Specific interface classes should reimplement `rpc_master`.

#### call(name, data=bytes(), send_timeout=1000, recv_timeout=1000):

Executes a remote call on the slave device. `name` is a string name of the remote function or method to execute. `data` is the `bytes` like object that will be sent as the argument of the remote function or method to exeucte. `send_timeout` defines how many milliseconds to wait while trying to connect to the slave and get it to execute the remote function or method. Once the master starts sending the argument to the slave deivce `send_timeout` does not apply. The library will allow the argument to take up to 5 seconds to be sent. `recv_timeout` defines how many milliseconds to wait after the slave started executing the remote method to receive the repsonse. Note that once the master starts receiving the repsonse `recv_timeout` does not apply. The library will allow the response to take up to 5 seconds to be received.

Note that a new packet that includes a copy of `data` will be created internally inside the `rpc` library. You may encounter memory issues on the OpenMV Cam if you try to pass very large data arguments.

## class rpc_slave():

The `rpc_slave` is a pure virtual class and not meant to be used directly. Specific interface classes should reimplement `rpc_slave`.

#### register_callback(cb):

Registers a call back that can be executed by the master device. The call back should take one argument which will be a `memoryview` object and it should return a `bytes()` like object as the result. The call back should return in less than 1 second if possible.

#### schedule_callback(cb):

After you execute `loop()` it is not possible to execute long running operations outside of the `rpc` library. `schedule_callback` allows you to break out of the `rpc` library temporarily after completion of an call back. You should execute `schedule_callback` during the execution of an `rpc` call back method to register a new non-rpc call back that will be executed immediately after the successful completion of that call back you executed `schedule_callback` in. The function or method should not take any arguments. After the the call back that was registered returns it must be registered again in the next parent call back. On any error of the parent call back the registered call back will not be called and must be registered again. Here's how to use this:

    def some_function_or_method_that_takes_a_long_time_to_execute():
        <do stuff>

    def normal_rpc_call_back(data):
        <process data>
        interface.schedule_callback(some_function_or_method_that_takes_a_long_time_to_execute)
        return bytes(response)

    interface.register_callback(normal_rpc_call_back)

    interface.loop()

`schedule_callback` in particular allows you to use the `get_bytes` and `put_bytes` methods for cut-through data transfer between one device and another without the cost of packetization which limits the size of the data moved inside the `rpc` library without running out of memory on the OpenMV Cam.

#### setup_loop_callback(cb):

The loop call back is called every loop iteration of `loop()`. Unlike the `schedule_callback()` call back this call back stays registered after being registered once. You can use the loop call back to blink an activity LED or something like that. You should not use the loop call back to execute any blocking code as this will get in the way of polling for communication from the master. Additionally, the loop call back will be called at a variable rate depending on when and what call backs the master is trying to execute. Given this, the loop call back is not suitable for any method that needs to be executed at a fixed frequency.

On the OpenMV Cam, if you need to execute something at a fixed frequency, you should setup a timer before executing `loop()` and use a timer interrupt based callback to execute some function or method at a fixed frequency. Please see how to [Write Interrupt Handlers](http://docs.openmv.io/reference/isr_rules.html) for more information. Note: The `mutex` library is installed on your OpenMV Cam along with the `rpc` library.

#### loop(recv_timeout=1000, send_timeout=1000):

Starts execution of the `rpc` library on the slave to receive data. This method does not return (except via an exception from a call back). You should register all call backs first before executing this method. However, it is possible to register new call backs inside of a call back previously being registered that is executing.

`recv_timeout` defines how long to wait to receive a command from the master device before trying again. `send_timeout` defines how long the slave will wait for the master to receive the call back response before going back to trying to receive. The loop call back will be executed before trying to receive again.

## rpc_uart_master(port, baudrate=9600)

Creates a master implementation of the `rpc` library to communicate over a hardware RS232/RS422/RS485 or TTL COM port. `port` is the string name of the serial port. `baudrate` is the bits per second to run at.

Please use the `rpc_usb_vcp_master` to talk to the OpenMV Cam over its USB interface. This class is for talking to the OpenMV Cam over its hardware UART.

If you are using this class with USB to serial converters you will likely experience intermittent synchronization issues. The RPC library uart interface on the OpenMV Cam is designed to work with real-time UARTs like hardware RS232/RS422/RS485  COM ports on PCs and TTL UARTS on single board computers like the RaspberryPi and Beaglebone. That said, the interface library can handle this and will continue to work.  

## rpc_uart_slave(port, baudrate=9600)

Creates a slave implementation of the `rpc` library to communicate over a hardware RS232/RS422/RS485  or TTL COM port. `port` is the string name of the serial port. `baudrate` is the bits per second to run at.

Please use the `rpc_usb_vcp_slave` to talk to the OpenMV Cam over its USB interface. This class is for talking to the OpenMV Cam over its hardware UART.

If you are using this class with USB to serial converters you will likely experience intermittent synchronization issues. The RPC library uart interface on the OpenMV Cam is designed to work with real-time UARTs like hardware RS232/RS422/RS485  COM ports on PCs and TTL UARTS on single board computers like the RaspberryPi and Beaglebone. That said, the interface library can handle this and will continue to work. 

## rpc_usb_vcp_master(port):

Creates a master implementation of the `rpc` library to communicate over a USB VCP (virtual COM port). `port` is the string name of the serial port.

Communication over USB is the most reliable and high speed way to connect an OpenMV Cam to a computer. However, as the OpenMV Cam has only one VCP port we recommend that you fully debug your script using the `rpc_usb_vcp_slave` on your OpenMV using another `rpc` interface if possible as you will not be able to get error messages off the OpenMV Cam easily. For example, connecting the `rpc_usb_vcp_master` to the `rpc_uart_slave` on your OpenMV Cam at 115200 BPS over a USB-to-Serial adapter when debugging will make your life far easier. Alternatively, use the Ethernet or WiFi interface when debugging.

## rpc_usb_vcp_slave(port):

Creates a slave implementation of the `rpc` library to communicate over a USB VCP (virtual COM port). `port` is the string name of the serial port.

Communication over USB is the most reliable and high speed way to connect an OpenMV Cam to a computer. However, as the OpenMV Cam has only one VCP port we recommend that you fully debug your script using the `rpc_usb_vcp_master` on your OpenMV using another `rpc` interface if possible as you will not be able to get error messages off the OpenMV Cam easily. For example, connecting the `rpc_usb_vcp_slave` to the `rpc_uart_master` on your OpenMV Cam at 115200 BPS over a USB-to-Serial adapter when debugging will make your life far easier. Alternatively, use the Ethernet or WiFi interface when debugging.

## rpc_wifi_or_ethernet_master(slave_ip, my_ip="", port=0x1DBA):

Creates a master implementation of the `rpc` library to communicate over WiFi or Ethernet. `slave_ip` is the IPV4 address of the `rpc` slave device. `my_ip` can be `""` which binds the master to any interface adapter to communicate to the slave. If `my_ip` is not `""` then it should be an IP address on the same subnet as `slave_ip`. `port` is a free port to use for UDP and TCP traffic.

## rpc_wifi_or_ethernet_slave(my_ip="", port=0x1DBA):

Creates a slave implementation of the `rpc` library to communicate over WiFi or Ethernet. `my_ip` can be `""` which binds the slave to any interface adapter to communicate to the master. If `my_ip` is not `""` then it should be an IP address on the same subnet as the master. `port` is a free port to use for UDP and TCP traffic.

## rpc_kvarser_can_master(channel, message_id=0x7FF, bit_rate=250000, sampling_point=75)

Creates a master implementation of the `rpc` library to communicate over CAN using Kvarser hardware. `channel` is the Kvarser channel to use. `message_id` is the 11-bit CAN ID to use for data transport. `bit_rate` is the CAN bus rate. `sampling_point` is the percentage sample point on the CAN bus.

## rpc_kvarser_can_slave(channel, message_id=0x7FF, bit_rate=250000, sampling_point=75)

Creates a slave implementation of the `rpc` library to communicate over CAN using Kvarser hardware. `channel` is the Kvarser channel to use. `message_id` is the 11-bit CAN ID to use for data transport. `bit_rate` is the CAN bus rate. `sampling_point` is the percentage sample point on the CAN bus.
