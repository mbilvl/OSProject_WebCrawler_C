# Use Ubuntu as the base image
FROM ubuntu:22.04

# Set working directory
WORKDIR /app

# Install required dependencies
RUN apt-get update && apt-get install -y \
    gcc \
    make \
    libcurl4-openssl-dev \
    libtidy-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy all source files to the container
COPY *.c *.h Makefile ./

# Build the crawler
RUN make

# Create a directory for output files
RUN mkdir -p /app/output

# Set the default command to run the crawler
CMD ["./crawler"]