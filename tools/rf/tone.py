"""Finite, explicitly invoked B210 tone; gain is uncalibrated, not output dBm."""
import argparse
import hashlib
import json
import signal
import time
import numpy as np
import uhd

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--fpga', required=True)
p.add_argument('--serial', required=True)
p.add_argument('--frequency', type=float, required=True)
p.add_argument('--seconds', type=float, required=True)
p.add_argument('--gain', type=float, default=20)
a = p.parse_args()
if not 0 < a.seconds <= 240 or not 0 <= a.gain <= 50:
    p.error('Duration must be <=240 s and gain 0..50')
with open(a.fpga, 'rb') as f:
    if hashlib.sha256(f.read()).hexdigest() != '7f0e329b4724e0d919a06b9924aaa3c77305b6f25038deebd6171657ac7c8a99':
        raise ValueError('Unexpected FPGA image')
running = True
def stop(*_):
    global running
    running = False
signal.signal(signal.SIGTERM, stop)
signal.signal(signal.SIGINT, stop)
radio = uhd.usrp.MultiUSRP(f'serial={a.serial},fpga={a.fpga}')
radio.set_tx_subdev_spec(uhd.usrp.SubdevSpec('A:A'))
radio.set_tx_rate(1e6)
radio.set_tx_gain(a.gain)
radio.set_tx_antenna('TX/RX')
radio.set_tx_bandwidth(1e6)
radio.set_tx_freq(uhd.types.TuneRequest(a.frequency - 100000))
rate = radio.get_tx_rate()
if abs(rate - 1e6) > 1 or abs(radio.get_tx_freq() - (a.frequency - 100000)) > 1:
    raise RuntimeError('Unexpected tuning/rate readback')
if not radio.get_tx_sensor('lo_locked').to_bool():
    raise RuntimeError('TX LO not locked')
stream = radio.get_tx_stream(uhd.usrp.StreamArgs('fc32', 'sc16'))
n = 10000
samples = (0.1 * np.exp(2j * np.pi * 100000 * np.arange(n) / rate)).astype(np.complex64)
md = uhd.types.TXMetadata()
md.start_of_burst = True
deadline = time.monotonic() + a.seconds
print(json.dumps(dict(event='RF_ON', frequency_hz=a.frequency, gain=a.gain,
                      amplitude=0.1, duration_s=a.seconds, subdevice='A:A')), flush=True)
try:
    while running and time.monotonic() < deadline:
        sent = 0
        while sent < n and running and time.monotonic() < deadline:
            count = stream.send(samples[sent:], md, 0.5)
            if count <= 0:
                raise RuntimeError('TX send timeout')
            sent += count
            md.start_of_burst = False
finally:
    md.start_of_burst = False
    md.end_of_burst = True
    stream.send(np.zeros(1, dtype=np.complex64), md, 0.5)
    radio.set_tx_gain(0)
    print(json.dumps(dict(event='RF_OFF')), flush=True)
