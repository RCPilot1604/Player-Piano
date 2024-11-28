import mido
import time

# Define the MIDI note range (37 to 108 for piano notes)
start_note = 21
end_note = 100

# Create a MIDI file
mid = mido.MidiFile()

# Add a new track to the MIDI file
track = mido.MidiTrack()
mid.tracks.append(track)

# Initialize current_time to keep track of the cumulative time for each event
current_time = 0

for note_number in range(start_note, end_note + 1):
    # Note on message (channel 0, note number, velocity, time since last event)
    msg_on = mido.Message('note_on', note=note_number, velocity=64, time=0)
    track.append(msg_on)
    #current_time = current_time + 480  # Reset current_time as the time is relative to the last event
    
    # Note off message after 1 second (Mido's time is in ticks, not milliseconds or seconds. Assuming default ticks per beat is 480, for 1 second delay, calculate ticks based on tempo)
    # Assuming a default tempo of 500000 microseconds per beat (120 BPM), 1 second = 480 ticks
    msg_off = mido.Message('note_off', note=note_number, velocity=0, time=480)
    track.append(msg_off)
    #current_time = current_time + 480  # Reset current_time for the next note

# Save the MIDI file
mid.save('test.mid')