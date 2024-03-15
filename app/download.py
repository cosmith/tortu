import subprocess
import sys


def download_and_convert(url):
    # Download video from YouTube
    download_command = [
        "yt-dlp",
        "-x",
        "--audio-format",
        "mp3",
        "--audio-quality",
        "0",
        url,
        "-o",
        "%(title)s.%(ext)s",
    ]
    subprocess.run(download_command)

    # Find downloaded MP3 files
    ls_command = ["ls", "*.mp3"]
    result = subprocess.run(ls_command, capture_output=True, text=True)
    files = result.stdout.splitlines()

    print(files)

    # Convert to 44kHz, 96kbps mono CBR MP3
    for file in files:
        output_file = f"{file.rsplit('.', 1)[0]}_converted.mp3"
        ffmpeg_command = [
            "ffmpeg",
            "-i",
            file,
            "-ac",
            "1",
            "-ar",
            "44000",
            "-ab",
            "96k",
            output_file,
        ]
        subprocess.run(ffmpeg_command)
        # Remove original file
        rm_command = ["rm", file]
        subprocess.run(rm_command)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <YouTube URL>")
        sys.exit(1)

    download_and_convert(sys.argv[1])
