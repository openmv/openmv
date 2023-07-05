# Post files with HTTP/Post urequests module example
import network
import usocket
import ussl
import urequests

# AP info
SSID='' # Network SSID
KEY=''  # Network key

# Init wlan module and connect to network
print("Trying to connect... (may take a while)...")

wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())

url = 'http://website.com/upload.php/'
# Or <ip>/<host>:port
#url = 'http://website.com:80/upload.php/'
# SSL is supported.
#url = 'https://192.168.1.102:443/upload.php/'

headers = {
    'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10.12; rv:55.0) Gecko/20100101 Firefox/55.0',
    # Add more headers if needed
}

# Send some files
files = {
    'image1': ('example1.jpg', open('example1.jpg', 'rb')),
    'image2': ('example2.jpg', open('example2.jpg', 'rb')),
}

# Post a request
if (True):
    # Send some files
    r = urequests.post(url, files=files, headers=headers) #Can add auth=('username', 'password') if needed
else:
    # Or send some JSON data
    r = urequests.post(url, json={'some': 'data'}, headers=headers) #Can add auth=('username', 'password') if needed

print(r.status_code, r.reason)
print(r.headers, r.content)
