#!/usr/bin/env python3
import os, shutil, sys, zipfile

if len(sys.argv) != 2:
    raise SystemExit("usage: tools/prepare_apk.py /path/to/Spider-man-HD-1-0-8.apk")
apk = os.path.abspath(sys.argv[1])
out = os.path.abspath('.apk_libs')
os.makedirs(out, exist_ok=True)
with zipfile.ZipFile(apk) as z:
    member = 'lib/armeabi/libspiderman.so'
    if member not in z.namelist():
        raise SystemExit(f"{member} not found in {apk}")
    with z.open(member) as src, open(os.path.join(out, 'libspiderman.so'), 'wb') as dst:
        shutil.copyfileobj(src, dst)
print(os.path.join(out, 'libspiderman.so'))
