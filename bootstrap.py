import urllib.request
import shutil
import platform
import os
import sys
import subprocess

os_type = platform.system()

if os_type == "Windows":
  dest_file = "llvm.7z"
  download_url = "https://github.com/ldc-developers/llvm-project/releases/download/ldc-v11.0.1/llvm-11.0.1-windows-x86.7z"
  print(">> Downloading " + download_url + " as llvm.tar.xz")
  with urllib.request.urlopen(download_url) as response, open(dest_file, 'wb') as out_file:
    shutil.copyfileobj(response, out_file)

  subprocess.call(["C:\\Program Files\\7-Zip\\7z.exe", "x", "llvm.7z", "-ollvm"])

  os.makedirs("bin", exist_ok=True)
  os.chdir("bin")
  print(">> Configuring project with CMake")
  subprocess.call(["cmake", "-DLLVM_DIR=..\\llvm\\lib\\cmake\\llvm", ".."])

else:
  if os.path.exists("/usr/local/lib/cmake/llvm"):
    # LLVM is already installed correctly
    print(">> LLVM is already set up correctly, nothing to do")
    sys.exit()
  import tarfile
  dest_file = "llvm.tar.xz"
  if os_type == "Darwin":
    download_url = "https://github.com/ldc-developers/llvm-project/releases/download/ldc-v11.0.1/llvm-11.0.1-osx-x86_64.tar.xz"
  else:
    arch = platform.uname().machine
    if arch == "armv7l":
      download_url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-11.0.1/clang+llvm-11.0.1-armv7a-linux-gnueabihf.tar.xz"

  print(">> Downloading " + download_url + " as llvm.tar.xz")
  with urllib.request.urlopen(download_url) as response, open(dest_file, 'wb') as out_file:
    shutil.copyfileobj(response, out_file)

  print(">> Extracting files from llvm.tar.xz")
  with tarfile.open(dest_file, "r:xz") as tar:
    os.makedirs("llvm", exist_ok=True)
    basename = os.path.basename(download_url).replace(".tar.xz", "")
    subdir_and_files = [
        tarinfo for tarinfo in tar.getmembers()
        if tarinfo.name.startswith(basename + "/lib/")
    ]
    tar.extractall("llvm", members=subdir_and_files)
    tar.close()

  os.makedirs("bin", exist_ok=True)
  os.chdir("bin")
  print(">> Configuring project with CMake")
  subprocess.call(["cmake", "-DLLVM_DIR=../" + basename + "/lib/cmake/llvm", ".."])

print(">> Cleaning up redundant files")
os.chdir("..")
os.remove(dest_file)