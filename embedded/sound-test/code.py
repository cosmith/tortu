import board
import audiomp3
import audiobusio

# Setup audio
audio = audiobusio.I2SOut(
    bit_clock=board.GP0,  # Bit clock
    word_select=board.GP1,  # Word select / left right clock
    data=board.GP2,  # data
)


print("playing")

mp3 = audiomp3.MP3Decoder("lion44-56mono.mp3")

audio.play(mp3)
while audio.playing:
    pass
print("stopped")
