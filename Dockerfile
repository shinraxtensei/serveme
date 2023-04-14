# Base image
FROM debian:buster

# Install Siege
RUN apt-get update && \
    apt-get install -y siege

# Set the entrypoint to Siege
ENTRYPOINT ["siege"]

# Set the default command to test your web server (replace with your own URL)
CMD ["-c", "3", "-r", "1", "10.11.9.7:6969"]