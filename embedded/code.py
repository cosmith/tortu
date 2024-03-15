import os
import adafruit_ssd1306
import audiobusio
import audiomp3
import board
import busio
import sdcardio
import storage
import supervisor
import time
import digitalio

# -----------------------------------------------
# Setup buttons
play = digitalio.DigitalInOut(board.GP15)
play.direction = digitalio.Direction.INPUT
play.pull = digitalio.Pull.UP

next = digitalio.DigitalInOut(board.GP14)
next.direction = digitalio.Direction.INPUT
next.pull = digitalio.Pull.UP

# -----------------------------------------------
# Setup display
i2c = busio.I2C(
    board.GP17,  # SCL
    board.GP16,  # SDA
)
DISPLAY_WIDTH = 128
DISPLAY_HEIGHT = 32
display = adafruit_ssd1306.SSD1306_I2C(DISPLAY_WIDTH, DISPLAY_HEIGHT, i2c)

# -----------------------------------------------
# Setup audio
audio = audiobusio.I2SOut(
    bit_clock=board.GP0,  # Bit clock
    word_select=board.GP1,  # Word select / left right clock
    data=board.GP2,  # data
)


def show_text(text):
    print(text)
    display.fill(0)
    display.text(text, 0, 0, 1)
    display.show()


def setup_sdcard():
    print("here")
    spi = busio.SPI(
        board.GP10,  # clock
        board.GP11,  # MOSI
        board.GP12,  # MISO
    )
    print("spi", spi)
    sd = sdcardio.SDCard(spi, board.GP13, 4000000)
    print("sd", sd)
    vfs = storage.VfsFat(sd)  # type: ignore
    storage.mount(vfs, "/sd")


def play_mp3s():
    mp3s = sorted(
        [f for f in os.listdir("/sd") if f.endswith(".mp3") and not f.startswith(".")]
    )
    show_text("Found " + str(len(mp3s)) + " mp3s")
    time.sleep(0.5)

    for mp3_filepath in mp3s:
        show_text("Playing " + mp3_filepath)
        filename = "/sd/" + mp3_filepath

        mp3 = audiomp3.MP3Decoder(filename)
        audio.play(mp3)

        last_pause = None

        while audio.playing:
            time.sleep(0.01)
            now = time.monotonic()

            if (
                not play.value
                and audio.paused
                and last_pause
                and now - last_pause > 0.1
            ):
                print(now - last_pause)
                audio.resume()
                show_text("Resumed")
                last_pause = None

            if not play.value and audio.playing:
                audio.pause()
                show_text("Paused")
                last_pause = now
                print(last_pause)

            if not next.value and audio.playing:
                audio.stop()
                show_text("Stopped")
                break

    show_text("Done playing!")


def show_image(img_path):
    with open(img_path, "rb") as f:
        # BMP files begin with a 54-byte header
        header = f.read(54)

        # Get the width and height of the image
        width = int.from_bytes(header[18:22], "little")
        height = int.from_bytes(header[22:26], "little")

        if width > DISPLAY_WIDTH or height > DISPLAY_HEIGHT:
            raise ValueError("Image too large for display")

        # Offset where the pixel data starts
        data_offset = int.from_bytes(header[10:14], "little")

        # Go to start of pixel data
        f.seek(data_offset)

        # Read the pixel data
        pixel_data = f.read()

        # Each byte in the pixel_data corresponds to 8 pixels
        # 1 bit for each pixel, 0 for black, 1 for white
        pixels = []
        for byte in pixel_data:
            for i in range(7, -1, -1):
                pixels.append((byte >> i) & 1)

        # The pixels are read in rows, where each row has width pixels
        # Also note that rows are stored bottom-up
        rows = [pixels[i : i + width] for i in range(0, len(pixels), width)]

        # If you want the rows to be top-down, you can reverse the rows list:
        rows = list(reversed(rows))

    display.fill(0)

    for y, row in enumerate(rows):
        for x, color in enumerate(row):
            display.pixel(x, y, color)
    display.invert(False)
    display.show()


show_text("Hello!")
try:
    setup_sdcard()
    #    show_image("arbre.bmp")
    play_mp3s()
except Exception as e:
    show_text(str(e))
    time.sleep(1)
    show_text("reloading in 3..")
    time.sleep(1)
    show_text("reloading in 2..")
    time.sleep(1)
    show_text("reloading in 1..")
    time.sleep(1)
    supervisor.reload()
