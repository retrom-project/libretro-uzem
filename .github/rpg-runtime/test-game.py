"""Opt-in operator ROM check; no game download or implicit private input."""
import ctypes as C
import hashlib
import sys
from pathlib import Path
lib = C.CDLL(str(Path(sys.argv[1]).resolve()))
rom = Path(sys.argv[2]).read_bytes()
class Game(C.Structure):
    _fields_ = [('path', C.c_char_p), ('data', C.c_void_p), ('size', C.c_size_t), ('meta', C.c_char_p)]
ENV = C.CFUNCTYPE(C.c_bool, C.c_uint, C.c_void_p)
VIDEO = C.CFUNCTYPE(None, C.c_void_p, C.c_uint, C.c_uint, C.c_size_t)
AUDIO = C.CFUNCTYPE(None, C.c_int16, C.c_int16)
POLL = C.CFUNCTYPE(None)
INPUT = C.CFUNCTYPE(C.c_int16, C.c_uint, C.c_uint, C.c_uint, C.c_uint)
frames = []
buttons = set()
audio_samples = 0
@ENV
def environment(cmd, data): return cmd in (10, 18)
@VIDEO
def video(data, width, height, pitch):
    frames.append(hashlib.sha256(C.string_at(data, height * pitch)).hexdigest())
@AUDIO
def audio(left, right):
    global audio_samples
    if left or right: audio_samples += 1
@POLL
def poll(): pass
@INPUT
def input_state(port, device, index, button): return int(port == 0 and button in buttons)
lib.retro_set_environment(environment)
lib.retro_set_video_refresh(video)
lib.retro_set_audio_sample(audio)
lib.retro_set_input_poll(poll)
lib.retro_set_input_state(input_state)
lib.retro_load_game.argtypes = [C.POINTER(Game)]; lib.retro_load_game.restype = C.c_bool
lib.retro_serialize_size.restype = C.c_size_t
lib.retro_serialize.argtypes = [C.c_void_p, C.c_size_t]; lib.retro_serialize.restype = C.c_bool
lib.retro_unserialize.argtypes = [C.c_void_p, C.c_size_t]; lib.retro_unserialize.restype = C.c_bool
buffer = C.create_string_buffer(rom)
game = Game(b'game.uze', C.cast(buffer,C.c_void_p),len(rom),None)
def start():
    lib.retro_init()
    assert lib.retro_load_game(C.byref(game))
def run(count):
    for _ in range(count): lib.retro_run()
def save():
    size = lib.retro_serialize_size(); data = C.create_string_buffer(size)
    assert lib.retro_serialize(data,size)
    return data
start(); run(180)
buttons.add(3); run(4); buttons.clear(); run(180)
buttons.add(7); run(20); buttons.clear()
saved = save()
run(60); expected = save().raw; sequence = frames[-60:]
assert lib.retro_unserialize(saved,len(saved)); run(60)
assert save().raw == expected and frames[-60:] == sequence, 'same-instance replay diverged'
lib.retro_unload_game(); lib.retro_deinit(); start(); run(1)
assert lib.retro_unserialize(saved,len(saved)); run(60)
assert save().raw == expected and frames[-60:] == sequence, 'fresh-instance replay diverged'
assert len(set(frames)) > 2 and audio_samples > 0
lib.retro_unload_game(); lib.retro_deinit()
print('PASS: frame/audio output, full-state deterministic replay and fresh-instance restore',len(saved),audio_samples)
