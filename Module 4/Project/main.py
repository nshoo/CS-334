import struct
import pyaudio
import pvporcupine
import google_speech
import generate_dalle
import extract_object
import image_convert
import time
import os
from interface import SalvadorInterface
from custom import bg, custom_bounds

inter = SalvadorInterface()

porcupine = None
pa = None
audio_stream = None

porcupine = pvporcupine.create(
    access_key="API_KEY",
    keyword_paths=["wake_word/Hey-Salvador_en_raspberry-pi_v2_1_0.ppn"])

pa = pyaudio.PyAudio()

def open_stream():
    return pa.open(
            rate=porcupine.sample_rate,
            channels=1,
            format=pyaudio.paInt16,
            input=True,
            frames_per_buffer=porcupine.frame_length)

audio_stream = open_stream()

def get_audio_frame():
    pcm = audio_stream.read(porcupine.frame_length)
    pcm = struct.unpack_from("h" * porcupine.frame_length, pcm)
    return pcm

inter.error()

while True:
    keyword_index = porcupine.process(get_audio_frame())

    if keyword_index >= 0:
        print("Hotword Detected")
        audio_stream.stop_stream()
        prompt = google_speech.recognize_phrase(inter, audio_stream, pa, porcupine.frame_length, porcupine.sample_rate)

        obj = extract_object.extract(prompt)
        if obj:
            print("Object:", obj)
            inter.loading(True)
            generate_dalle.generate_image(inter, str(obj), "images/dalle.png")
            image_convert.convert("dalle")
            inter.loading(False)
            bg.plot_file("images/dalle.json", bounds=custom_bounds)
        else:
            inter.error()
            print("Invalid prompt!")

        audio_stream.start_stream()

