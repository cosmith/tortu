FROM python:3.9-slim

# Set the working directory in the container
WORKDIR /usr/src/app

# Install ffmpeg and curl
RUN apt-get update && \
    apt-get install -y ffmpeg curl && \
    rm -rf /var/lib/apt/lists/*

# Install yt-dlp
RUN pip install -U yt-dlp[default] openai

# Copy the Python script into the container
COPY download.py .

# Run the script when the container launches
# Note: YouTube URL must be passed via command line arguments when starting the container
ENTRYPOINT ["python", "./download.py"]
