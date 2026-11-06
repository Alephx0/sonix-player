"""Build the pinned WebP decoder only, inside a private build directory."""
import argparse
import fcntl
import hashlib
import os
import shutil
from pathlib import Path
import subprocess
import tarfile
import urllib.request

VERSION = '1.6.0'
SHA256 = 'e4ab7009bf0629fd11982d4c2aa83964cf244cffba7347ecd39019a9e38c4564'

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('root', type=Path)
    p.add_argument('arch', choices=['host', 'target'])
    p.add_argument('--cc', default='cc')
    a = p.parse_args()
    root = a.root.resolve()
    root.mkdir(parents=True, exist_ok=True)
    source = root / ('libwebp-' + VERSION)
    with (root / 'source.lock').open('w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        if not source.exists():
            archive = root / ('libwebp-' + VERSION + '.tar.gz')
            if not archive.exists():
                url = 'https://storage.googleapis.com/downloads.webmproject.org/releases/webp/' + archive.name
                with urllib.request.urlopen(url, timeout=45) as response:
                    data = response.read(10 * 1024 * 1024)
                if hashlib.sha256(data).hexdigest() != SHA256:
                    raise SystemExit('WebP archive checksum mismatch')
                archive.write_bytes(data)
            if hashlib.sha256(archive.read_bytes()).hexdigest() != SHA256:
                raise SystemExit('WebP archive checksum mismatch')
            with tarfile.open(archive) as tar:
                # Reject links and paths outside the single pinned source root.
                for member in tar.getmembers():
                    path = Path(member.name)
                    if path.is_absolute() or '..' in path.parts or path.parts[0] != source.name or not (member.isdir() or member.isfile()):
                        raise SystemExit('Unexpected WebP archive entry')
                tar.extractall(root)
    build = root / a.arch
    build.mkdir(parents=True, exist_ok=True)
    options = ['--disable-shared', '--enable-static', '--enable-libwebpdecoder',
               '--disable-libwebpmux', '--disable-libwebpdemux', '--disable-threading',
               '--disable-sse2', '--disable-sse4.1', '--disable-avx2', '--disable-neon',
               '--disable-gl', '--disable-sdl', '--disable-png', '--disable-jpeg',
               '--disable-tiff', '--disable-gif']
    env = dict(os.environ, CFLAGS='-Os -ffunction-sections -fdata-sections')
    if a.arch == 'target':
        cc = Path(a.cc).resolve()
        env.update(CC=str(cc), PATH=str(cc.parent) + os.pathsep + env['PATH'])
        env['CFLAGS'] += ' -march=mips32r2'
        options.append('--host=mipsel-rockbox-linux-gnu')
    subprocess.run([str(source / 'configure'), *options], cwd=build, env=env, check=True)
    for directory, target in [('dec', 'libwebpdecode.la'), ('dsp', 'libwebpdspdecode.la'),
                              ('utils', 'libwebputilsdecode.la'), ('', 'libwebpdecoder.la')]:
        subprocess.run(['make', '-C', str(build / 'src' / directory), '-j4', target], env=env, check=True)
    shutil.copyfile(build / 'src/.libs/libwebpdecoder.a', build / 'libwebpdecoder.a')

if __name__ == '__main__':
    main()
