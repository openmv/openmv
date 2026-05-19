metadata(version="2.6.2")

# microdot.session uses PyJWT.
require("pyjwt")

# Freeze the curated set of microdot modules from the submodule source.
package(
    "microdot",
    base_path="../microdot/src",
    files=[
        "__init__.py",
        "microdot.py",
        "helpers.py",
        "auth.py",
        "cors.py",
        "csrf.py",
        "login.py",
        "multipart.py",
        "session.py",
        "sse.py",
        "websocket.py",
    ],
)
