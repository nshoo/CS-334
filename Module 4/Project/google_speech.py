import speech_recognition as sr
from google.cloud import speech_v1 as speech
import pyaudio
import wave

def get_transcript(response):
    transcript = ""
    for result in response.results:
        best_alternative = result.alternatives[0]
        transcript += best_alternative.transcript
    return transcript

def speech_to_text(config, audio):
    client = speech.SpeechClient()
    response = client.recognize(config=config, audio=audio)
    return get_transcript(response)

def recognize_phrase(inter, stream, pa, chunk, fs):
    # Record audio
    r = sr.Recognizer()

    inter.status_light(True)
    record_wav(inter, stream, pa, chunk, fs)

    with sr.AudioFile("output.wav") as source:
        audio = r.record(source)
    inter.status_light(False)

    # Recognize audio
    config = dict(language_code="en-US")
    audio_dict = {"content": audio.get_wav_data()}
    return speech_to_text(config, audio_dict)

def record_wav(inter, stream, pa, chunk, fs):
    sample_format = pyaudio.paInt16  # 16 bits per sampl
    channels = 1

    stream.start_stream()

    filename = "output.wav"

    frames = []  # Initialize array to store frames

    while not inter.start_btn.is_pressed:
        data = stream.read(chunk)
        frames.append(data)

    stream.stop_stream()

    wf = wave.open(filename, 'wb')
    wf.setnchannels(channels)
    wf.setsampwidth(pa.get_sample_size(sample_format))
    wf.setframerate(fs)
    wf.writeframes(b''.join(frames))
    wf.close()

