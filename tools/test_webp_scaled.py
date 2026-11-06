"""Exercise native WebP decoding with generated, non-network fixtures."""
from pathlib import Path
import subprocess
import sys
import tempfile
from PIL import Image

root = Path(sys.argv[1]).resolve() / 'sonix-player'
with tempfile.TemporaryDirectory(prefix='sonix-webp-') as directory:
    d = Path(directory)
    Image.new('RGB', (640, 320), 'red').save(d/'lossless.webp', lossless=True)
    Image.new('RGB', (256, 256), 'blue').save(d/'lossy.webp', quality=80)
    Image.new('RGB', (1025, 1025), 'green').save(d/'large.webp', lossless=True)
    Image.new('RGB', (64, 64), 'red').save(d/'animated.webp', save_all=True,
        append_images=[Image.new('RGB', (64, 64), 'blue')], duration=100, loop=0)
    subprocess.run(['gcc', '-Wall', '-Wextra', '-Werror', '-fsanitize=address,undefined',
        '-I.', '-I.deps/webp/libwebp-1.6.0/src', 'tests/test_webp_scaled.c',
        'src/system/image/webp_scaled.c', '.deps/webp/host/libwebpdecoder.a',
        '-lm', '-o', str(d/'test')], cwd=root, check=True)
    subprocess.run([str(d/'test'), *[str(d/name) for name in
        ('lossless.webp','lossy.webp','large.webp','animated.webp')]], check=True)
